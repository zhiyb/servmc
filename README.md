# ServMC: Minecraft Server Management Console

This is a helper program. By encapsulating the server application for the Java version of Minecraft, it provides useful server management functionalities. It also features an improved console interface and an intuitive web interface.

## Features

+ Improved console user interface, using Readline and the GNU History Library
+ Intuitive web interface, using React and Matrial Design
+ WebSocket supports real-time web-based console
+ Automatic backup scheduler
+ Automatic server version update
+ Server restart management

# Build dependencies

## Linux

### Debian-based distributions

```
curl -sL https://deb.nodesource.com/setup_8.x | sudo -E bash -
sudo apt-get install -y nodejs
sudo apt-get install -y libcurl4-openssl-dev libjson-c-dev libwebsockets-dev libreadline-dev libmagic-dev zlib1g-dev
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

## Linux

```
make
```

## MacOS

### Homebrew

```
make CFLAGS+=-I/usr/local/opt/readline/include CFLAGS+=-I/usr/local/opt/openssl/include LIBS+=-L/usr/local/opt/readline/lib
```

# Run

```
make run
```
