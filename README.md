# Depend

## Linux

### Debian and Ubuntu based

```
curl -sL https://deb.nodesource.com/setup_8.x | sudo -E bash -
sudo apt-get install -y nodejs
```

## MacOS

### Homebrew

```
sudo brew install json-c
sudo brew install libwebsockets
sudo brew install readline
sudo brew install libmagic
sudo brew install node
```

# Build

## Linux Build:

```
make
```

## MacOS Build:

### Homebrew

```
make CFLAGS+=-I/usr/local/opt/readline/include CFLAGS+=-I/usr/local/opt/openssl/include LIBS+=-L/usr/local/opt/readline/lib
```

# Run

```
make run
```