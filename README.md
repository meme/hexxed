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

### Debian based Linux

    apt install g++ cmake pkg-config libglib2.0-dev libncurses-dev
    cmake .
    make

## Usage

    ./hexxed [FILE]

FILE should be writeable.

### Commands

* F3 - Edit mode
* F5 - Goto (hex input only)
* F10 - Quit
