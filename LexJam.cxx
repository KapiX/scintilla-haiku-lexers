/*
 * Copyright 2015 Your Name <your@email.address>
 * All rights reserved. Distributed under the terms of the MIT license.
 */


#include <string.h>
#include <assert.h>
#include <ctype.h>

#include <map>
#include <string>
#include <stdexcept>

#include <ILexer.h>
#include <Scintilla.h>

#include "StringCopy.h"
#include "WordList.h"
#include "LexAccessor.h"
#include "Accessor.h"
#include "StyleContext.h"
#include "CharacterSet.h"
#include "OptionSet.h"
#include "DefaultLexer.h"

#include "common.h"

using namespace Scintilla;

static const char styleSubable[] = { 0 };
static const char LexerName[] = "Jam";

enum kwType { kwOther, kwLocal, kwFor };

enum {
	SCE_JAM_DEFAULT,
	SCE_JAM_COMMENT,
	SCE_JAM_NUMBER,
	SCE_JAM_STRING,
	SCE_JAM_KEYWORD,
	SCE_JAM_OPERATOR,
	SCE_JAM_IDENTIFIER,
	SCE_JAM_VARIABLE
};

LexicalClass lexicalClasses[] = {
	// Lexer Jam SCLEX_JAM SCE_JAM_:
	0, "SCE_JAM_DEFAULT", "default", "White space",
	1, "SCE_JAM_COMMENT", "comment line", "Comment",
	2, "SCE_JAM_NUMBER", "literal numeric", "Number",
	3, "SCE_JAM_STRING", "literal string", "String",
	4, "SCE_JAM_KEYWORD", "keyword", "Keyword",
	5, "SCE_JAM_OPERATOR", "operator", "Operators",
	6, "SCE_JAM_IDENTIFIER", "identifier", "Identifiers",
	7, "SCE_JAM_VARIABLE", "variable", "Variables",
};

inline bool IsANumber(const char* s)
{
	size_t index;
	try {
		std::stoi(s, &index);
	} catch (std::invalid_argument&) {
		return false;
	}
	if(index != strlen(s))
		return false;
	return true;
}

struct OptionsJam {
	bool fold;

	OptionsJam() {
		fold = false;
	}
};

static const char *const jamWordListDesc[] = {
	"Keywords",
	0
};

struct OptionSetJam : public OptionSet<OptionsJam> {
	OptionSetJam() {
		DefineWordListSets(jamWordListDesc);
	}
};

class LexJam : public DefaultLexer {
	WordList keywords;
	OptionsJam options;
	OptionSetJam osJam;
public:
	explicit LexJam() :
		DefaultLexer(lexicalClasses, ELEMENTS(lexicalClasses)) {
	}
	virtual ~LexJam() override {
	}
	void SCI_METHOD Release() override {
		delete this;
	}
	int SCI_METHOD Version() const override {
		return lvRelease4;
	}
	const char *SCI_METHOD PropertyNames() override {
		return osJam.PropertyNames();
	}
	int SCI_METHOD PropertyType(const char *name) override {
		return osJam.PropertyType(name);
	}
	const char *SCI_METHOD DescribeProperty(const char *name) override {
		return osJam.DescribeProperty(name);
	}
	Sci_Position SCI_METHOD PropertySet(const char *key, const char *val) override;
	const char *SCI_METHOD DescribeWordListSets() override {
		return osJam.DescribeWordListSets();
	}
	Sci_Position SCI_METHOD WordListSet(int n, const char *wl) override;
	void SCI_METHOD Lex(Sci_PositionU startPos, Sci_Position lengthDoc, int initStyle, IDocument *pAccess) override;
	void SCI_METHOD Fold(Sci_PositionU startPos, Sci_Position lengthDoc, int initStyle, IDocument *pAccess) override;
	void * SCI_METHOD PrivateCall(int operation, void *pointer) override {
		return 0;
	}
	int SCI_METHOD LineEndTypesSupported() override {
		return SC_LINE_END_TYPE_DEFAULT;
	}
	int SCI_METHOD AllocateSubStyles(int styleBase, int numberStyles) override;
	int SCI_METHOD SubStylesStart(int styleBase) override;
	int SCI_METHOD SubStylesLength(int styleBase) override;
	int SCI_METHOD StyleFromSubStyle(int subStyle) override;
	int SCI_METHOD PrimaryStyleFromStyle(int style) override;
	void SCI_METHOD FreeSubStyles() override;
	void SCI_METHOD SetIdentifiers(int style, const char *identifiers) override;
	int SCI_METHOD DistanceToSecondaryStyles() override;
	const char * SCI_METHOD GetSubStyleBases() override;

	static ILexer4 *LexerFactory() {
		return new LexJam();
	}
};

Sci_Position SCI_METHOD LexJam::PropertySet(const char *key, const char *val) {
	if (osJam.PropertySet(&options, key, val)) {
		return 0;
	}
	return -1;
}

Sci_Position SCI_METHOD LexJam::WordListSet(int n, const char *wl) {
	WordList *wordListN = 0;
	switch (n) {
	case 0:
		wordListN = &keywords;
		break;
	}
	Sci_Position firstModification = -1;
	if (wordListN) {
		WordList wlNew;
		wlNew.Set(wl);
		if (*wordListN != wlNew) {
			wordListN->Set(wl);
			firstModification = 0;
		}
	}
	return firstModification;
}

void SCI_METHOD LexJam::Lex(Sci_PositionU startPos, Sci_Position lengthDoc, int initStyle, IDocument *pAccess) {
	Accessor styler(pAccess, NULL);
	StyleContext sc(startPos, lengthDoc, initStyle, styler);
	
	kwType kwLast = kwOther;
	int varLastStyle = SCE_JAM_DEFAULT;
	for(; sc.More(); sc.Forward()) {
		switch(sc.state) {
			case SCE_JAM_COMMENT: {
				if (sc.ch == '\r' || sc.ch == '\n') {
					sc.SetState(SCE_JAM_DEFAULT);
				}
			} break;
			case SCE_JAM_STRING: {
				if (sc.Match('\"') && sc.chPrev != '\\') {
					sc.ForwardSetState(SCE_JAM_DEFAULT);
				} else if(sc.Match("$(")) {
					sc.SetState(SCE_JAM_VARIABLE);
					varLastStyle = SCE_JAM_STRING;
				}
			} break;
			case SCE_JAM_NUMBER: {
				sc.SetState(SCE_JAM_DEFAULT);
			} break;
			case SCE_JAM_OPERATOR: {
				sc.SetState(SCE_JAM_DEFAULT);
			} break;
			case SCE_JAM_VARIABLE: {
				if(sc.ch == ')') {
					sc.ForwardSetState(varLastStyle);
				}
			} break;
			case SCE_JAM_IDENTIFIER: {
				if(IsASpaceOrTab(sc.ch) || sc.ch == '\n' || sc.ch == '\r' || sc.ch == ']') {
					char s[100];
					sc.GetCurrent(s, sizeof(s));
					int style = SCE_JAM_IDENTIFIER;
					if (kwLast == kwLocal || kwLast == kwFor) {
						style = SCE_JAM_VARIABLE;
					} else if (keywords.InList(s)) {
						style = SCE_JAM_KEYWORD;
					} else if (IsANumber(s)) {
						style = SCE_JAM_NUMBER;
					}
					sc.ChangeState(style);
					sc.SetState(SCE_JAM_DEFAULT);
					kwLast = kwOther;
					if(style == SCE_JAM_KEYWORD) {
						if(strcmp(s, "local") == 0) {
							kwLast = kwLocal;
						} else if(strcmp(s, "for") == 0) {
							kwLast = kwFor;
						}
					}
				}
			} break;
		}
		if(sc.state == SCE_JAM_DEFAULT) {
			if (sc.Match('#')) {
				sc.SetState(SCE_JAM_COMMENT);
			} else if (sc.Match('\"') && sc.chPrev != '\\') {
				sc.SetState(SCE_JAM_STRING);
			} else if (IsASCII(sc.ch) && (isoperator(static_cast<char>(sc.ch)) || sc.ch == '@')) {
				sc.SetState(SCE_JAM_OPERATOR);
			} else if(isalnum(sc.ch)) {
				sc.SetState(SCE_JAM_IDENTIFIER);
			} else if(sc.ch == '$') {
				varLastStyle = SCE_JAM_DEFAULT;
				sc.SetState(SCE_JAM_VARIABLE);
				sc.Forward();
			}
		}
	}
	sc.Complete();
}

void SCI_METHOD LexJam::Fold(Sci_PositionU, Sci_Position, int, IDocument *) {
}

int SCI_METHOD LexJam::AllocateSubStyles(int, int) {
	return -1;
}

int SCI_METHOD LexJam::SubStylesStart(int) {
	return -1;
}

int SCI_METHOD LexJam::SubStylesLength(int) {
	return 0;
}

int SCI_METHOD LexJam::StyleFromSubStyle(int subStyle) {
	return subStyle;
}

int SCI_METHOD LexJam::PrimaryStyleFromStyle(int style) {
	return style;
}

void SCI_METHOD LexJam::FreeSubStyles() {
}

void SCI_METHOD LexJam::SetIdentifiers(int, const char *) {
}

int SCI_METHOD LexJam::DistanceToSecondaryStyles() {
	return 0;
}

const char * SCI_METHOD LexJam::GetSubStyleBases() {
	return styleSubable;
}

extern "C" {

int EXT_LEXER_DECL GetLexerCount()
{
	return 1;
}

void EXT_LEXER_DECL GetLexerName(unsigned int Index, char *name, int buflength)
{
	// below useless evaluation(s) to supress "not used" warnings
	Index;
	// return as much of our lexer name as will fit (what's up with Index?)
	if (buflength > 0) {
		buflength--;
		int n = strlen(LexerName);
		if (n > buflength)
			n = buflength;
		memcpy(name, LexerName, n), name[n] = '\0';
	}
}

LexerFactoryFunction EXT_LEXER_DECL GetLexerFactory(unsigned int index) {
	if (index == 0)
		return LexJam::LexerFactory;
	else
		return 0;
}

} // extern "C"
