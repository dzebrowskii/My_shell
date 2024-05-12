# Dokumentacja Projektu My Shell
## Opis projektu
Projekt My Shell to prosty emulator powłoki systemowej (shell), który czyta polecenia ze standardowego wejścia (lub z pliku skryptu) i wykonuje je, korzystając z systemowego środowiska uruchomieniowego. Shell wykonuje polecenia przez wywołania systemowe z rodziny exec (np. execl, execlp, execvp), obsługuje przekierowania, potoki oraz inne podstawowe funkcjonalności powłoki.
## Funkcjonalności

Wykonywanie poleceń: Shell wykonuje polecenia wprowadzone przez użytkownika lub zawarte w skrypcie, rozdzielając je na komendy i argumenty.

Przekierowanie wyjścia (>): Możliwość przekierowania standardowego wyjścia polecenia do pliku.

Obsługa potoków (|): Tworzenie potoków dowolnej długości, umożliwiające przekazywanie wyjścia jednego polecenia jako wejścia innego.

Uruchamianie w tle (&): Polecenia mogą być uruchamiane w tle, co umożliwia kontynuowanie pracy bez czekania na ich zakończenie.

Zmiana katalogu roboczego (cd): Funkcja zmiany bieżącego katalogu pracy.

Tworzenie plików (touch): Możliwość tworzenia nowych plików.

Wyświetlanie historii poleceń: Historia wykonanych poleceń jest zapisywana i może być wyświetlana.

Obsługa sygnałów (SIGQUIT): Obsługa sygnałów, np. dla wyświetlenia historii poleceń.

## Uruchamianie projektu

Projekt można skompilować i uruchomić przy użyciu standardowych narzędzi w środowisku Unix/Linux. Poniżej znajduje się przykład komend do kompilacji i uruchomienia:

make

./my_shell

## Testowanie projektu
### Skrypty do testowania
#### Skrypt nr 1: Testowanie podstawowych poleceń (test_basic_commands.sh)

#!/home/student/moj_shell

ls

ls -a

echo "Wyświetlam aktualny katalog:"

pwd

echo "Lista użytkowników:"

who


#### Skrypt nr 2: Testowanie działania funkcji cd (test_cd_command.sh)

#!/home/student/moj_shell

echo "Aktualny katalog:"

pwd

cd ..

echo "Katalog po zmianie:"

pwd

cd /tmp

echo "Katalog po zmianie na /tmp:"

pwd

#### Skrypt nr 3: Działanie skryptu w tle (test_background.sh)

#!/home/student/moj_shell

echo "Uruchamianie sleep na 10 sekund w tle"

sleep 10 &

echo "Uruchamiam ls:"

ls

#### Skrypt nr 4: Przekierowanie wyjścia (test_redirection.sh)

#!/home/student/moj_shell

echo "Zapisuje listę plików do pliku files.txt"

ls > files.txt

echo "Zawartość pliku files.txt:"

cat files.txt

#### Skrypt nr 5: Tworzenie potoków dowolnej długości (test_pipes.sh)

#!/home/student/moj_shell

echo "Połączenie kilku poleceń w potok:"

ls -l | grep ".txt" | sort

#### Skrypt nr 6: Polecenie touch, nano, cat (test_file_operations.sh)

#!/home/student/moj_shell

echo "Tworzę nowy plik tekstowy"

touch testfile.txt

echo "Edytuję plik w nano (zamknij, aby kontynuować)"

nano testfile.txt

echo "Wyświetlam zawartość pliku testfile.txt"

cat testfile.txt

#### Skrypt nr 7: Wyświetlenie historii poleceń (test_history.sh)

#!/home/student/moj_shell

echo "Wysyłam sygnał SIGQUIT do shella, aby wyświetlić historię"

kill -SIGQUIT $$

### Sposób uruchamiania skryptów

Aby uruchomić którykolwiek ze skryptów, trzeba nadać mu prawa do wykonania poleceniem chmod +x nazwa_skryptu.sh, a następnie uruchomić poleceniem:
./nazwa_skryptu.sh

