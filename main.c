#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <signal.h>

#define MAX_LINE_LENGTH 1024

void execute_command(char *args[], int background); //odpalanie w tle
void execute_piped_commands(char *commands[], int num_commands);
void append_command_to_history(const char *command);
void sigquit_handler(int sig); //historia
int check_cd_command(char *args[]);
int check_touch_command(char *args[]);

int main(int argc, char *argv[]) {
    char *line = NULL;
    size_t line_buf_size = 0;
    ssize_t line_size;
    char *args[MAX_LINE_LENGTH / 2 + 1]; //przechowuje podzieloną na segmenty linię komend do dalszego przetwarzania
    int background;
    int first_line = 1; //flaga uzywana do pominiecia linii shebang w skryptach

    signal(SIGQUIT, sigquit_handler); //obsluga sygnalu

    FILE *input_stream = stdin; // wskaźnik input_stream na standardowe wejście

    // Obsługa uruchamiania skryptu jako argumentu programu
    if (argc > 1) {
        input_stream = fopen(argv[1], "r");
        if (!input_stream) {
            fprintf(stderr, "Error opening script file: %s\n", argv[1]);
            exit(EXIT_FAILURE);
        }
    }

    while (1) {
        if (input_stream == stdin) {
            printf("> ");
        }
        line_size = getline(&line, &line_buf_size, input_stream); //zbieramy nasza linie kodu

        if (line_size <= 0) {
            break; // Koniec pliku (EOF) lub błąd
        }

        if (line[line_size - 1] == '\n') {
            line[line_size - 1] = '\0'; // Usuwa znak nowej linii
        }

        // pomijanie pierwszej linii skryptu jesli jest to shebang
        if (first_line && line[0] == '#' && line[1] == '!') {
            first_line = 0;
            continue;
        }
        first_line = 0;

        append_command_to_history(line); // Zapis do historii

        char *commands[MAX_LINE_LENGTH / 2 + 1]; // Podzial na polecenia i argumenty
        int num_commands = 0;
        char *token = strtok(line, "|");

        while (token != NULL) {
            commands[num_commands++] = token;
            token = strtok(NULL, "|");
        }
        //wykonanie polecenia z potokami lub bez
        if (num_commands > 1) {
            execute_piped_commands(commands, num_commands);
        } else {
            int arg_count = 0;
            token = strtok(line, " "); //podzial linii na słowa (tokeny)
            while (token != NULL) {
                args[arg_count++] = token; //zczytywanie argumetow
                token = strtok(NULL, " ");
            };
            args[arg_count] = NULL;

            if (arg_count == 0) {
                continue; // Pusta linia
            }

            background = 0;
            if (args[arg_count - 1] && strcmp(args[arg_count - 1], "&") == 0) { //sprawdzamy czy ma byc w tle
                background = 1;
                args[--arg_count] = NULL;
            }

            if (!check_touch_command(args) && !check_cd_command(args)) { //sprawdzamy czy cd i touch
                execute_command(args, background); //jak nie to odpalamy w tle
            }
        }
    }

    if (input_stream != stdin) {
        fclose(input_stream);  // Zamknij plik skryptu jeśli był używany
    }
    free(line);
    return 0;
}


int check_cd_command(char *args[]) {
    if (strcmp(args[0], "cd") == 0) {
        if (args[1] == NULL) {
            fprintf(stderr, "cd: argument expected\n");
        } else {
            if (chdir(args[1]) != 0) { //zmienamy katalog
                perror("cd");
            }
        }
        return 1;
    }
    return 0;
}

void execute_command(char *args[], int background) { //uruchomienie polecen
    int redirect_output = 0; // flaga z przekierowaniem
    char *output_file = NULL;

    // szukamy znak przekierowania i oddzielamy nazwe pliku
    for (int i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], ">") == 0) {
            args[i] = NULL; // Usuwamy '>' z argumentów
            output_file = args[i + 1]; // Nazwa pliku jest następnym argumentem
            redirect_output = 1;
            break;
        }
    }

    pid_t pid = fork(); // Rozdziela biezacy proces na proces rodzica i proces potomny
    if (pid == 0) { // Proces dziecka
        // Przekierowanie wyjscia
        if (redirect_output) {
            int fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644); // o_wronly - zapis o_trunc - plik do długości 0, jeżeli już istnieje
            //0644 to prawa dostępu do pliku
            if (fd < 0) {
                perror("open");
                exit(EXIT_FAILURE);
            }
            dup2(fd, STDOUT_FILENO); //kopiuje deskryptor pliku fd do deskryptora pliku wyjscia standardowego
            close(fd); // oryginalny deskryptor pliku
        }

        // Wykonanie polecenia
        if (execvp(args[0], args) < 0) {
            perror("execvp");
            exit(EXIT_FAILURE);
        }
    } else if (pid > 0) { // Proces rodzica
        int status;
        if (!background) {
            waitpid(pid, &status, 0); // Czekaj na zakończenie procesu dziecka jesli nie jest w tle

        }
    } else {
        perror("fork"); // Nie udalo się utworzyć procesu
    }
}


void execute_piped_commands(char *commands[], int num_commands) { // potoki polecen
    int num_pipes = num_commands - 1; //Oblicza liczbe potrzebnych potoków  ktora jest zawsze o jeden mniejsza niż liczba poleceń ponieważ potok łączy dwa procesy
    int pipe_fd[2 * num_pipes]; // tworzy tablice do przechowywania deskryptorów plików dla potoków

    // Tworzenie wszystkich potokow
    for (int i = 0; i < num_pipes; i++) {
        if (pipe(pipe_fd + 2 * i) < 0) { // 2 * i zapewnia że każda para deskryptorów jest unikalna dla danego potoku.
            perror("pipe");
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < num_commands; i++) {
        pid_t pid = fork(); // Tworzy nowy proces
        if (pid == 0) { // Proces dziecka
            if (i > 0) { // Nie pierwszy proces - podlacz do potoku z poprzednim procesem
                dup2(pipe_fd[2 * (i - 1)], STDIN_FILENO); // sluzy do przekierowania standardowego wejścia procesu na wcześniej utworzony potok
            }
            if (i < num_pipes) { // Nie ostatni proces - podlacz do potoku z następnym procesem
                dup2(pipe_fd[2 * i + 1], STDOUT_FILENO); // sluzy do przekierowania standardowego wejścia procesu na wcześniej utworzony potok
            }

            // Zamknij wszystkie deskryptory w potoku
            for (int j = 0; j < 2 * num_pipes; j++) {
                close(pipe_fd[j]);
            }

            // dzielimy polecenie na argumenty
            char *args[MAX_LINE_LENGTH / 2 + 1]; // tablica na argumenty
            int arg_count = 0;
            char *token = strtok(commands[i], " "); //dzielimy argumenty
            while (token != NULL) {
                args[arg_count++] = token;
                token = strtok(NULL, " "); // pobiera kolejny token z tego samego stringu
            }
            args[arg_count] = NULL; //dajemy ostatnie miejsce na null

            if (execvp(args[0], args) < 0) { // wywolanie funkcji execvp z pierwszym argumentem jako nazwa programu do wykonania
                perror("execvp");
                exit(EXIT_FAILURE);
            }
        }
    }

    // Zamknij wszystkie deskryptory w procesie rodzica
    for (int i = 0; i < 2 * num_pipes; i++) {
        close(pipe_fd[i]);
    }

    // Czekaj na zakończenie wszystkich procesów
    for (int i = 0; i < num_commands; i++) {
        wait(NULL);
    }
}

void append_command_to_history(const char *command) { // zapisywanie kazdego wykonanego polecenia do pliku historii
    const char *home_directory = getenv("HOME"); // pobierz sciezke do katalogu domowego
    if (home_directory == NULL) {
        perror("getenv");
        exit(EXIT_FAILURE);
    }

    char history_file_path[256];
    snprintf(history_file_path, sizeof(history_file_path), "%s/.my_shell_history", home_directory); // tworzy pelna sciezke do pliku historii

    FILE *file = fopen(history_file_path, "a"); // Otwórz plik historii do dopisywania
    if (file == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    fprintf(file, "%s\n", command); // dopisz polecenie do pliku historii
    fclose(file);
}

void sigquit_handler(int sig) { //obsługa sygnalu SIGQUIT
    const char *home_directory = getenv("HOME");
    if (home_directory == NULL) {
        perror("getenv");
        exit(EXIT_FAILURE);
    }

    char history_file_path[256];
    snprintf(history_file_path, sizeof(history_file_path), "%s/.my_shell_history", home_directory); // sciezka do pliku polecen

    printf("Historia poleceń:\n");
    FILE *file = fopen(history_file_path, "r");
    if (file == NULL) {
        perror("fopen");
        return;
    }

    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    while ((read = getline(&line, &len, file)) != -1) { // czyta kazda linie z pliku aż do konca pliku
        printf("%s", line);
    }
    printf("> ");

    free(line);
    fclose(file);
}

int check_touch_command(char *args[]) { //obsluga touch
    if (strcmp(args[0], "touch") == 0) { // sprawdzamy czy argumenten jest touch
        if (args[1] == NULL) { // do touch jest potrzebny argument
            fprintf(stderr, "touch: argument expected\n");
        } else {
            for (int i = 1; args[i] != NULL; i++) {
                int fd = open(args[i], O_WRONLY | O_CREAT, 0644); // O_WRONLY - otwarcie pliku tylko do zapisu, O_CREAT - jak nie ma pliczku to go tworzymy
                if (fd < 0) {
                    perror("touch");
                } else {
                    close(fd); // deskryptor pliku zamykamy
                }
            }
        }
        return 1; // potwierdzenie że polecenie zostalo obsluzone
    }
    return 0;
}

