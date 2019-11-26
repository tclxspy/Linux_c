src = $(wildcard *.c)
target = $(patsubst %.c, %, $(src))

argv_1 = wrap.c
argv_2 = -Wall -g -lpthread

ALL:$(target)

%:%.c
	gcc $(argv_1) $< -o $@ $(argv_2)

clean:
	-rm -rf $(target)

.PHONY: clean ALL
