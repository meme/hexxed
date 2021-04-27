hexxed-tutorial(7)

# NAME

hexxed - Hexxed tutorial

# CONTENTS

. Starting Hexxed
. Navigating
. Selecting Blocks
. Changing Representations
. Editing
. Calculator
. Quit
. Further Reading

# Step 1: Starting Hexxed

Launch Hexxed by typing *hexxed* in the terminal and passing a file that is
writeable. Hexxed will open the Hex pane:

```
[example]                           {UNK+.00000000`00000000}
00000000: |7f|45 4c 46-02 01 01 00-00 00 00 00  .ELF........
0000000c:  02 00 3e 00-01 00 00 00-a0 12 40 00  ..>.......@.
00000018:  40 00 00 00-00 00 00 00-80 ad 01 00  @...........
00000024:  00 00 00 00-40 00 38 00-0d 00 40 00  ....@.8...@.
00000030:  06 00 00 00-04 00 00 00-40 00 00 00  ........@...
0000003c:  40 00 40 00-00 00 00 00-40 00 40 00  @.@.....@.@.
00000048:  d8 02 00 00-00 00 00 00-d8 02 00 00  ............
00000054:  08 00 00 00-00 00 00 00-03 00 00 00  ............
00000060:  18 03 00 00-00 00 00 00-18 03 40 00  ..........@.
0000006c:  18 03 40 00-00 00 00 00-1c 00 00 00  ..@.........
00000078:  1c 00 00 00-00 00 00 00-01 00 00 00  ............
00000084:  01 00 00 00-04 00 00 00-00 00 00 00  ............
00000090:  00 00 40 00-00 00 00 00-00 00 40 00  ..@.......@.
0000009c:  e0 0d 00 00-00 00 00 00-e0 0d 00 00  ............
000000a8:  00 10 00 00-00 00 00 00-01 00 00 00  ............
000000b4:  00 10 00 00-00 00 00 00-00 10 40 00  ..........@.
000000c0:  00 10 40 00-00 00 00 00-45 0b 01 00  ..@.....E...
000000cc:  45 0b 01 00-00 00 00 00-00 10 00 00  E...........
000000d8:  01 00 00 00-04 00 00 00-00 20 01 00  ......... ..
000000e4:  00 20 41 00-00 00 00 00-00 20 41 00  . A...... A.
000000f0:  b8 3e 00 00-00 00 00 00-b8 3e 00 00  .>.......>..
000000fc:  00 10 00 00-00 00 00 00-01 00 00 00  ............
```

- The *[]* enclosed characters are the *buffer* path name
- The *{}* enclosed characters are the *file type* and *virtual address* of the current buffer
- The *||* enclosed characters are the selected *hex pair*

The ascending numbers on the right are the offset into the file, which may be
virtual addresses depending on the file type.

The data on the far right is the ASCII representation of the buffer data, see
*isprint*(3) for more information.

# Step 2: Navigating

Navigating in Hexxed is straightforward, use the arrow keys to move around the
buffer and *Page up* and *Page down* to advance by the current terminal height.
The *Home* and *End* keys go to the beginning and end of the buffer,
respectively. Hexxed uses an overscroll of 3 rows. Additionally, *+* and *-*
can be used to push and pop bookmarks on a stack, respectively. A maximum of 8
bookmarks is supported by default.

# Step 3: Selecting Blocks

To select a block of *data*, use the *v* key. Hit *Home* or scroll to the start
of the buffer, and hit *v* then move to the end of the line. Then, hit *v*
again and move to the right. The cursor will be placed on the next line.

```
example                               UNK+.00000000`00000000
00000000: [7f 45 4c 46-02 01 01 00-00 00 00 00] .ELF........
0000000c: {02}00 3e 00-01 00 00 00-a0 12 40 00  ..>.......@.
00000018:  40 00 00 00-00 00 00 00-80 ad 01 00  @...........
00000024:  00 00 00 00-40 00 38 00-0d 00 40 00  ....@.8...@.
00000030:  06 00 00 00-04 00 00 00-40 00 00 00  ........@...
0000003c:  40 00 40 00-00 00 00 00-40 00 40 00  @.@.....@.@.
00000048:  d8 02 00 00-00 00 00 00-d8 02 00 00  ............
00000054:  08 00 00 00-00 00 00 00-03 00 00 00  ............
00000060:  18 03 00 00-00 00 00 00-18 03 40 00  ..........@.
0000006c:  18 03 40 00-00 00 00 00-1c 00 00 00  ..@.........
00000078:  1c 00 00 00-00 00 00 00-01 00 00 00  ............
00000084:  01 00 00 00-04 00 00 00-00 00 00 00  ............
00000090:  00 00 40 00-00 00 00 00-00 00 40 00  ..@.......@.
0000009c:  e0 0d 00 00-00 00 00 00-e0 0d 00 00  ............
000000a8:  00 10 00 00-00 00 00 00-01 00 00 00  ............
000000b4:  00 10 00 00-00 00 00 00-00 10 40 00  ..........@.
000000c0:  00 10 40 00-00 00 00 00-45 0b 01 00  ..@.....E...
000000cc:  45 0b 01 00-00 00 00 00-00 10 00 00  E...........
000000d8:  01 00 00 00-04 00 00 00-00 20 01 00  ......... ..
000000e4:  00 20 41 00-00 00 00 00-00 20 41 00  . A...... A.
000000f0:  b8 3e 00 00-00 00 00 00-b8 3e 00 00  .>.......>..
000000fc:  00 10 00 00-00 00 00 00-01 00 00 00  ............
```

- The *[]* enclosed characters are the selected *block*
- The *{}* enclosed characters denote the cursor location

# Step 4: Changing Representations

In addition to the Hex representation, Hexxed also offers a *Raw* view. Since
the panes share a buffer, they will also share underyling *comments* and blocks.
Hit *Enter* to cycle panes into the *Raw* pane. Here, only printable characters
are displayed on the screen. See *isprint(3)* for more information. Hit *Enter*
to cycle back to the *Hex* view

# Step 5: Editing

Hexxed is fundamentally a hex editor, so it is straightforward to perform
edit operations. Hit *F3* to enter edit mode, and the cursor will change to select
a single *nibble* instead of a *pair*. To overwrite the data under the cursor,
enter a character in the range *0-9* or *a-f*. The change will be reflected in the
buffer. Hit *Escape* to exit.

# Step 6: Calculator

Hexxed can also be used as a calculator. Hit *=* to open the calculator in any
pane.

```
+----------------------------- Calculator -----------------------------+
|                                                                      |
| Sig:0                                                                |
| Uns:0                                                                |
| Bin:0000000000000000000000000000000000000000000000000000000000000000 |
| Hex:00000000`00000000                                                |
+----------------------------------------------------------------------+
```

Enter an expression and hit *Enter*. The following representations are listed:

- *Sig* for a signed decimal representation
- *Uns* for an unsigned decimal representation
- *Bin* for a binary representation
- *Hex* for a hexadecimal representation

Hit *Escape* to exit the calculator.

See *hexxed*(1) for a list of all operators and prefixes.

# Step 7: Quit

Congratulations, this has concluded the Hexxed tutorial. Hit *F10* to exit
the editor and persist changes to disk.

# SEE ALSO

*hexxed*(1)

# AUTHORS

For more information about Hexxed development, see
https://github.com/meme/hexxed.
