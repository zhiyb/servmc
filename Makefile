SRC	= main.c exec.c cmd.c net.c update.c
TRG	= servmc

OBJ	= $(SRC:.c=.o)
CFLAGS	:= -Wall -Werror -O2
CFLAGS	+= $(shell curl-config --cflags)
CFLAGS	+= $(shell pkg-config --cflags json-c)
LDFLAGS	= -Wall -Werror -O2
LIBS	:= $(shell curl-config --libs)
LIBS	+= $(shell pkg-config --libs json-c)
LIBS	+= -lreadline -lpthread

.PHONY: all
all: $(TRG)

$(TRG): $(OBJ)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

.c.o:
	$(CC) $(CFLAGS) -o $@ -c $<

.PHONY: run
run: $(TRG)
	./$(TRG)

.PHONY: clean
clean:
	rm -f $(OBJ)
