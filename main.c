#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <signal.h>

#define MAX_LINE_LENGTH 1024

void execute_command(char *args[], int background);
void execute_with_pipe(char *first_cmd[], char *second_cmd[]);
void append_command_to_history(const char *command);
void sigquit_handler(int sig);
int check_cd_command(char *args[]);

int main() {
    char *line = NULL;
    size_t line_buf_size = 0;
    ssize_t line_size;
    char *args[MAX_LINE_LENGTH / 2 + 1]; // Maksymalna liczba argumentów
    int background;

    signal(SIGQUIT, sigquit_handler);

    while (1) {
        printf("> ");
        line_size = getline(&line, &line_buf_size, stdin);

        if (line_size <= 0) {
            break; // Koniec pliku (EOF)
        }

        // Usuwamy znak nowej linii na końcu
        if (line[line_size - 1] == '\n') {
            line[line_size - 1] = '\0';
        }

        append_command_to_history(line); // Zapisz polecenie do historii

        int arg_count = 0;
        char *token = strtok(line, " ");
        while (token != NULL) {
            args[arg_count++] = token;
            token = strtok(NULL, " ");
        }
        args[arg_count] = NULL; // Ostatni element musi być NULL dla execvp

        if (arg_count == 0) {
            continue; // Pusta linia
        }

        // Sprawdzenie, czy ostatni argument to '&'
        background = 0;
        if (strcmp(args[arg_count - 1], "&") == 0) {
            background = 1;
            args[--arg_count] = NULL;
        }

        if (!check_cd_command(args)) {
            execute_command(args, background);
        }
    }

    free(line); // Zwolnienie pamięci zaalokowanej przez getline
    return 0;
}

int check_cd_command(char *args[]) {
    if (strcmp(args[0], "cd") == 0) {
        if (args[1] == NULL) {
            fprintf(stderr, "cd: argument expected\n");
        } else {
            if (chdir(args[1]) != 0) {
                perror("cd");
            }
        }
        return 1;
    }
    return 0;
}

void execute_command(char *args[], int background) {
    int redirect_output = 0;
    char *output_file = NULL;

    // Znajdź znak przekierowania i oddziel nazwę pliku
    for (int i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], ">") == 0) {
            args[i] = NULL; // Usuń '>' z argumentów
            output_file = args[i + 1]; // Nazwa pliku jest następnym argumentem
            redirect_output = 1;
            break;
        }
    }

    pid_t pid = fork();
    if (pid == 0) { // Proces dziecka
        // Przekierowanie wyjścia, jeśli wymagane
        if (redirect_output) {
            int fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd < 0) {
                perror("open");
                exit(EXIT_FAILURE);
            }
            dup2(fd, STDOUT_FILENO);
            close(fd);
        }

        // Wykonanie polecenia
        if (execvp(args[0], args) < 0) {
            perror("execvp");
            exit(EXIT_FAILURE);
        }
    } else if (pid > 0) { // Proces rodzica
        if (!background) {
            waitpid(pid, NULL, 0); // Czekaj na zakończenie procesu dziecka, jeśli nie jest w tle
        }
    } else {
        perror("fork"); // Nie udało się utworzyć procesu
    }
}

void execute_with_pipe(char *first_cmd[], char *second_cmd[]) {
    int pipe_fd[2]; // Deskryptory plików dla potoku: [0] do odczytu, [1] do zapisu
    if (pipe(pipe_fd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    pid_t pid1 = fork();
    if (pid1 == 0) { // Proces dziecka nr 1
        close(pipe_fd[0]); // Zamknięcie niepotrzebnego końca odczytu
        dup2(pipe_fd[1], STDOUT_FILENO); // Przekierowanie STDOUT do końca zapisu potoku
        close(pipe_fd[1]); // Zamknięcie oryginalnego deskryptora zapisu potoku

        execvp(first_cmd[0], first_cmd);
        perror("execvp first command");
        exit(EXIT_FAILURE);
    }

    pid_t pid2 = fork();
    if (pid2 == 0) { // Proces dziecka nr 2
        close(pipe_fd[1]); // Zamknięcie niepotrzebnego końca zapisu
        dup2(pipe_fd[0], STDIN_FILENO); // Przekierowanie STDIN do końca odczytu potoku
        close(pipe_fd[0]); // Zamknięcie oryginalnego deskryptora odczytu potoku

        execvp(second_cmd[0], second_cmd);
        perror("execvp second command");
        exit(EXIT_FAILURE);
    }

    // Zamknięcie obu końców potoku w procesie rodzica
    close(pipe_fd[0]);
    close(pipe_fd[1]);

    // Czekanie na zakończenie obu procesów dziecka
    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);
}

void append_command_to_history(const char *command) {
    const char *home_directory = getenv("HOME"); // Pobierz ścieżkę do katalogu domowego
    if (home_directory == NULL) {
        perror("getenv");
        exit(EXIT_FAILURE);
    }

    char history_file_path[256];
    snprintf(history_file_path, sizeof(history_file_path), "%s/.my_shell_history", home_directory);

    FILE *file = fopen(history_file_path, "a"); // Otwórz plik historii do dopisywania
    if (file == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    fprintf(file, "%s\n", command); // Dopisz polecenie do pliku historii
    fclose(file);
}

void sigquit_handler(int sig) {
    const char *home_directory = getenv("HOME");
    if (home_directory == NULL) {
        perror("getenv");
        exit(EXIT_FAILURE);
    }

    char history_file_path[256];
    snprintf(history_file_path, sizeof(history_file_path), "%s/.my_shell_history", home_directory);

    printf("Historia poleceń:\n");
    FILE *file = fopen(history_file_path, "r");
    if (file == NULL) {
        perror("fopen");
        return;
    }

    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    while ((read = getline(&line, &len, file)) != -1) {
        printf("%s", line);
    }

    free(line);
    fclose(file);
}

// zrobic touch!!
// wszystko co jest w /bin