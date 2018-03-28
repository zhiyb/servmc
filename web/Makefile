.PHONY: all
all: node_modules build

.DELETE_ON_ERROR:

node_modules: package.json
	npm install
	touch node_modules

build: node_modules $(wildcard src/* src/pages/*)
	npm run build

.PHONY: clean
clean:
	rm -rf build

.PHONY: distclean
distclean: clean
	rm -rf node_modules
