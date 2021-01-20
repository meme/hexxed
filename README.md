# hexxed

A portable, public-domain hex editor released under the Unlicense, or the public domain otherwise.

![Hexxed with an open calculator dialog](https://i.imgur.com/66jwWmv.png)

## Features

* Multi-pane: Text and Hex pane (Code planned)
* Integrated 64-bit calculator
* Goto/Search
* Comments
* Bookmarks
* Highlighting and selection

## Building

### Debian-based Linux

```
apt install g++ cmake pkg-config libglib2.0-dev libncurses-dev
mkdir build && cd build
cmake .. && make
```
### Fedora

```
dnf install gcc-c++ cmake pkgconf-pkg-config glib2-devel ncurses-devel
mkdir build && cd build
cmake .. && make
```

## Usage

```
./hexxed [path]
```

See *hexxed*(1) and *hexxed-tutorial*(7) for documentation and tutorials.
