# Definicja kompilatora
CC=gcc

# Opcje kompilacji, włączające wszystkie ostrzeżenia i standard C99
CFLAGS=-Wall -Wextra -std=c99

# Nazwa pliku wykonywalnego
TARGET=my_shell

# Pliki źródłowe
SOURCES=main.c

# Cel domyślny
all: $(TARGET)

# Reguła linkowania
$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCES)

# Reguła czyszczenia projektu
clean:
	rm -f $(TARGET)

# Dodaj tutaj dodatkowe reguły, jeśli potrzebujesz
