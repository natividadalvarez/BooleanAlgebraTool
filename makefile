build: bav.c parsing.c parsing.h
	cc -Wall -Wextra bav.c parsing.c -o bav -O3

test: test.c parsing.c
	cc test.c parsing.c -O3 \
		`pkg-config --libs criterion` -o test

