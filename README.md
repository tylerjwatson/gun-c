# gun-c
A [gun.js](https://github.com/amark/gun) port to C using libwebsockets, ideal for running on small and embedded devices.

## Building

Requirements:
* `gcc`
* `cmake`
* `libwebsockets` or `libwebsockets-dev` on Ubuntu
* `gdb` for debugging and `valgrind` for leak checking

```bash
$ git clone git@github.com:tylerjwatson/gun-c.git
$ cd gun-c
$ mkdir build
$ ./build.sh
```

## Running
```bash
build/gun
```
