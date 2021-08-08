SRC=$(wildcard src/*.c)

main: $(SRC)
	gcc -o $@ -Wall -Wextra -pedantic $^

fmt:
	clang-format -i *.c *.h
