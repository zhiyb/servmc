SRCS	= main.c exec.c cmd.c net.c update.c monitor.c backup.c restart.c web.c
TRG	= servmc

OBJS	= $(SRCS:.c=.o)
CFLAGS	:= -Wall -Werror -O2
CFLAGS	+= $(shell curl-config --cflags)
CFLAGS	+= $(shell pkg-config --cflags json-c)
CFLAGS	+= $(shell pkg-config --cflags libwebsockets)
LDFLAGS	= -Wall -Werror -O2
LIBS	:= $(shell curl-config --libs)
LIBS	+= $(shell pkg-config --libs json-c)
LIBS	+= $(shell pkg-config --libs libwebsockets)
LIBS	+= -lreadline -lmagic -lpthread

.PHONY: all
all: $(TRG)

$(TRG): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

.c.o:
	$(CC) $(CFLAGS) -o $@ -c $< -MMD

-include $(OBJS:%.o=%.d)

.PHONY: run
run: $(TRG)
	./$(TRG)

.PHONY: clean
clean:
	rm -f $(OBJS) $(OBJS:%.o=%.d)
