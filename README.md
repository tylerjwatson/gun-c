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

## Credits
This software has integrated code from the following libraries:
* [Libwebsockets](https://libwebsockets.org/) (MIT Licensed and others)
* [Yuarel](https://github.com/jacketizer/libyuarel) (MIT Licensed)
* [mjson](https://github.com/cesanta/mjson) (MIT Licensed)
* [log.c](https://github.com/rxi/log.c) (MIT Licensed)
* [ht](https://github.com/benhoyt/ht) (MIT Licensed)

Many thanks to the authors and contributors of this code.
