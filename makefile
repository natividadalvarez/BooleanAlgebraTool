build: bav.c parsing.c algebra.c parsing.h algebra.h
	cc -Wall -Wextra bav.c parsing.c algebra.c -o bav -O3

debug: bav.c parsing.c algebra.c parsing.h algebra.h
	cc -Wall -Wextra -Wno-unused-parameter bav.c parsing.c algebra.c -o bav -Og -g

test: test.c parsing.c algebra.c parsing.h algebra.h
	cc test.c parsing.c algebra.c -O3 \
		`pkg-config --libs criterion` -o test

