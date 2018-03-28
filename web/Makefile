
.PHONY: all
all: install build

install: node_modules

node_modules: package.json
	npm install
	touch node_modules

build: $(wildcard src/**/*)  
	npm run build

clean:
	rm -rf build

cleanall:
	rm -rf node_modules
	rm -rf build