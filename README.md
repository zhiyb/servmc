# Depend

## MacOS:

```
brew install json-c
brew install libwebsockets
brew install readline
brew install libmagic
```

# Build

## Linux Build:

```
make
```

## MacOS Build:

```
make CFLAGS+=-I/usr/local/opt/readline/include CFLAGS+=-I/usr/local/opt/openssl/include LIBS+=-L/usr/local/opt/readline/lib
```

# Run

```
make run
```