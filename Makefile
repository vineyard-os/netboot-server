SERVER		:= bin/server

SRC			:= $(shell find ./ -name '*.c')
OBJ			:= $(addprefix bin/,$(addsuffix .o,$(SRC)))

CFLAGS		:= -g -Wall -Wextra -Iinclude $(shell pkg-config readline --cflags)
LDFLAGS		:= -g $(shell pkg-config readline --libs)

server: $(SERVER)

$(SERVER): $(OBJ)
	gcc -o $@ $^ $(LDFLAGS)

bin/%.c.o: %.c
	mkdir -p $(dir $@)
	gcc -c -o $@ $^ $(CFLAGS)

clean:
	$(RM) $(SERVER) $(OBJ)

.PHONY: server clean

.SUFFIXES:
.SILENT:
