./bin/main: *.c
	gcc -o ./bin/main -Wall -Wextra -pedantic \
		main.c \
		common.c \
		editor.c
