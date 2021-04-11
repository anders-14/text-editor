
./bin/main: *.c
	gcc -o ./bin/main -Wall -Wextra -pedantic \
		common.c \
		cursor.c \
		draw.c \
		edit.c \
		file.c \
		keys.c \
		main.c

fmt:
	clang-format -i *.c *.h
