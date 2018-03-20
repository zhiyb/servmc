SRC	= main.c exec.c cmd.c
TRG	= servmc

OBJ	= $(SRC:.c=.o)
LIBS	= -lreadline -lpthread

CFLAGS	= -Wall -Werror -O2
LDFLAGS	= -Wall -Werror -O2

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
