CFLAGS = -Wall -Werror -Wextra -pedantic -std=c99
SRC = src/main.c src/page.c src/arena.c
DEBUG = 1

ifeq ($(DEBUG),1)
	CFLAGS += -ggdb -O0
endif

.PHONY: build
build:
	$(CC) $(CFLAGS) $(SRC) -o build/colla

.PHONY: lib
lib:
	$(CC) $(CFLAGS) $(SRC) -o build/colla.so -fPIC -shared
