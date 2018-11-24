/*
 * Copyright 2015 Your Name <your@email.address>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef COMMON_H
#define COMMON_H

#include <BeBuild.h>
#include <ILexer.h>

#define EXT_LEXER_DECL

typedef Scintilla::ILexer4 *(*LexerFactoryFunction)();

#endif // _H
