SRC=$(wildcard src/*.c)

main: $(SRC)
	gcc -o $@ -Wall -Wextra -pedantic $^

fmt:
	clang-format -i src/*.c src/*.h
