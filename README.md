# Scintilla Haiku lexers

Scintilla lexers for Haiku specific file types.

## Compiling

Before this project can be compiled, it needs [lexlib](https://github.com/ScintillaOrg/lexilla/tree/master/lexlib) directory from Lexilla.
Then run `make all` (`make` will not work).

It also requires makefile-engine (installed by default in Haiku).

## Installation

Lexers should be added to any lib directory in lexilla (for example /system/lib/lexilla).

Applications should look for these lexers there.
