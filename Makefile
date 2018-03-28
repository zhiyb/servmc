OBJS	:= main.o exec.o cmd.o net.o update.o monitor.o backup.o restart.o
OBJS	+= web.o query.o
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
all: all-$(TRG) all-web

.PHONY: clean
clean: clean-$(TRG) clean-web

.PHONY: distclean
distclean: clean-$(TRG) distclean-web

.PHONY: run
run: all
	./$(TRG)

# $(TRG) related rules

.PHONY: all-$(TRG)
all-$(TRG): $(TRG)

$(TRG): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

.c.o:
	$(CC) $(CFLAGS) -o $@ -c $< -MMD

-include $(OBJS:%.o=%.d)

.PHONY: clean-$(TRG)
clean-$(TRG):
	rm -f $(OBJS) $(OBJS:%.o=%.d)

# Web page related rules

.PHONY: all-web
all-web: | www
	$(MAKE) -C web

www:
	ln -sf web/build www

.PHONY: clean-web
clean-web:
	$(MAKE) -C web clean
	rm -f www

.PHONY: distclean-web
distclean-web:
	$(MAKE) -C web distclean
	rm -f www
