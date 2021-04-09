./bin/main: *.c
	gcc -o ./bin/main -Wall -Wextra -pedantic \
		main.c \
		common.c \
		cursor.c \
		editor.c

fmt:
	clang-format -i *.c *.h
