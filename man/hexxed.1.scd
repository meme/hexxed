hexxed(1)

# NAME

hexxed - A portable, public-domain hex editor

# SYNOPSIS

_hexxed_ [path]

For a guided tutorial, use *man hexxed-tutorial* from your terminal.

# DESCRIPTION

Hexxed is a mode editor, like vi. It uses panes and dialogs to convey information.

Panes are views over of buffers, which are views into files on disk. Each
pane has its own set of commands, with additional commands that are global to
every pane.

Dialogs are simple popups that prompt the user or display a message. Dialogs
are _not_ stacking.

# OPTIONS

*path*
	Opens the specified file as read-only. Edit mode requires the file to have
	writable permissions.

# GLOBAL COMMANDS

*F3*
	Enter edit mode. The cursor shape will change to a single cell, and navigation
	will move to the nearest odd or even hex value under the cursor. Escape exits
	edit mode.

*F5*
	Open the Goto dialog. This dialog supports full expression evaluation like
	the *Calculator*. Hit enter after entering an expression, or Escape to exit.

*F9*
	List all comments. Select a comment and hit *Enter* to go to it.

*F10*
	Save changes and exit.

*Enter*
	Cycle through current modes. There are two modes in Hexxed, Raw and Hex.

# LOCAL COMMANDS

*v*
	Enters block-selection mode. Move your cursor to the end of the block and
	hit *v* again to select. The block will remain selected as you move around
	the pane. Blocks are persisted across buffers.

*[0-9a-f]*
	Inserts hex over existing values, if in edit mode.

*;*
	Inserts a comment at the current position.

*=*
	Opens the calculator. See *Calculator*.

*+*
	Push the cursor position to the bookmark stack.

*-*
	Pop a bookmark from the stack and move to that location.

# CALCULATOR

Hexxed can also be used as a 64-bit calculator. The following operators are
supported, and follow the C operator precendence order.

- && ||
- & ^ |
- == !=
- > >= < <=
- << >>
- + \-
- \* / %
- !
- ~

The calculator is _hex by default_, but operands base can be changed using
a prefix. The following prefixes are supported:

- 0n for base 10
- 0o for base 8
- 0x for base 16 (the default if no prefix is specified)
- 0b for base 2

The buffer can also be accessed from the calculator using the following data
specifiers and a prefix:

- @ for unsigned data
- # for signed data

- b for a byte
- s for a 16-bit integer
- i for a 32-bit integer
- l for a 64-bit integer

Lower-case specifiers read little-endian data, while upper-case specifiers read
big-endian data. For example, *@S* will read an unsigned big-endian short while a
*#l* will read a little-endian signed long.

# SEE ALSO

*hexxed-tutorial*(7)

# AUTHORS

For more information about Hexxed development, see
https://github.com/meme/hexxed.
