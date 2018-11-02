# Scintilla Haiku lexers

Scintilla lexers for Haiku specific file types.

## Compiling

Before this project can be compiled, it needs [lexlib](https://sourceforge.net/p/scintilla/code/ci/default/tree/lexlib/) directory from Scintilla.
Then run `make all` (`make` will not work).

It also requires makefile-engine (installed by default in Haiku).

## Installation

Lexers should be added to any data directory in scintilla/lexers (for example /system/data/scintilla/lexers).

Applications should look for these lexers there.
