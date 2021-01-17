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
	Opens the specified file as read-write, the file must have writable permissions
	or it will fail to open.

# GLOBAL COMMANDS

*F3*
	Enter edit mode. The cursor shape will change to a single cell, and navigation
	will move to the nearest odd or even hex value under the cursor. Escape exits
	edit mode.

*F5*
	Open the Goto dialog. This dialog supports full expression evaluation like
	the *Calculator*. Hit enter after entering an expression, or Escape to exit.

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

# SEE ALSO

*hexxed-tutorial*(7)

# AUTHORS

Maintained by Keegan Saunders <keegan@undefinedbehaviour.org>, who is assisted
by other open source contributors. For more information about Hexxed development,
see https://github.com/meme/hexxed.
