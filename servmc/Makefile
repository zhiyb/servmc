OBJS	:= main.o exec.o cmd.o net.o update.o monitor.o restart.o
OBJS	+= web.o action.o
OBJS	+= $(subst .c,.o,$(wildcard mon/*.c))
TRG		= servmc

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
run:
	$(MAKE) -C .. run

.PHONY: clean
clean:
	rm -f $(TRG) $(OBJS) $(OBJS:%.o=%.d)

.PHONY: distclean
distclean: clean
