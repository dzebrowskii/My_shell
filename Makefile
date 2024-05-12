# Definicja kompilatora
CC=gcc

# Opcje kompilacji wlaczajace wszystkie ostrzezenia i standard C99
CFLAGS=-Wall -Wextra -std=c99

# Nazwa pliku wykonywalnego
TARGET=my_shell

# pliki zrodlowe
SOURCES=main.c

# Cel domyslny
all: $(TARGET)

# Regula linkowania
$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCES)

# Regula czyszczenia projektu
clean:
	rm -f $(TARGET)

