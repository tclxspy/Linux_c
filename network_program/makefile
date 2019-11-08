src = $(wildcard *.c)
target = $(patsubst %.c, %, $(src))

myArgs = -Wall -g -lpthread

ALL:$(target)

%:%.c
	gcc $< -o $@ $(myArgs)

clean:
	-rm -rf $(target)

.PHONY: clean ALL
