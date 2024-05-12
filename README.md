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

### Sposób uruchamiania skryptów

Aby uruchomić którykolwiek ze skryptów, zapisz go jako plik .sh, nadaj mu prawa do wykonania poleceniem chmod +x nazwa_skryptu.sh, a następnie uruchom poleceniem:
./nazwa_skryptu.sh

