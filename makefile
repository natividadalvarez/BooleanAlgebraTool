build: main.c parsing.c algebra.c parsing.h algebra.h
	cc -std=c99 -Wall -Wextra main.c parsing.c algebra.c -o main -O3

debug: main.c parsing.c algebra.c parsing.h algebra.h
	cc -std=c99 -Wall -Wextra -Wno-unused-parameter main.c parsing.c algebra.c -o main -Og -g

test: test.c parsing.c algebra.c parsing.h algebra.h
	cc test.c parsing.c algebra.c -O3 \
		`pkg-config --libs criterion` -o test

