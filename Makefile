.PHONY: all
all: all-servmc all-web

.PHONY: clean
clean: clean-servmc clean-web

.PHONY: distclean
distclean: distclean-servmc distclean-web

.PHONY: run
run: all
	./servmc/servmc

# servmc related rules

.PHONY: all-servmc clean-servmc distclean-servmc
all-servmc clean-servmc distclean-servmc: %-servmc:
	$(MAKE) -C servmc $*

# Web page related rules

.PHONY: all-web
all-web: | www
	$(MAKE) -C web

www:
	ln -sf web/build www

.PHONY: clean-web distclean-web
clean-web distclean-web: %-web:
	$(MAKE) -C web $*
	rm -f www
