SRCS	= main.c exec.c cmd.c net.c update.c monitor.c backup.c restart.c web.c
TRG	= servmc

OBJS	= $(SRCS:.c=.o)

override	CFLAGS	+= -Wall -Werror -O2
override	CFLAGS	+= $(shell curl-config --cflags)
override	CFLAGS	+= $(shell pkg-config --cflags json-c)
override	CFLAGS	+= $(shell pkg-config --cflags libwebsockets)
override	LDFLAGS	+= -Wall -Werror -O2
override	LIBS	+= $(shell curl-config --libs)
override	LIBS	+= $(shell pkg-config --libs json-c)
override	LIBS	+= $(shell pkg-config --libs libwebsockets)
override	LIBS	+= -lreadline -lmagic -lpthread

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
