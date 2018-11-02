## Haiku Generic Makefile v2.6 ## 

TYPE = SHARED

LEXERS = $(wildcard Lex*.cxx)
SRCS = $(wildcard lexlib/*.cxx)

SYSTEM_INCLUDE_PATHS = $(shell findpaths -e B_FIND_PATH_HEADERS_DIRECTORY scintilla)
LOCAL_INCLUDE_PATHS = lexlib

## Include the Makefile-Engine
DEVEL_DIRECTORY := \
	$(shell findpaths -r "makefile_engine" B_FIND_PATH_DEVELOP_DIRECTORY)
include $(DEVEL_DIRECTORY)/etc/makefile-engine

$(OBJ_DIR)/%.d : %.cxx
	mkdir -p $(OBJ_DIR); \
	mkdepend $(LOC_INCLUDES) -p .cxx:$(OBJ_DIR)/%n.o -m -f "$@" $<

$(OBJ_DIR)/%.o : %.cxx
	$(C++) -c $< $(INCLUDES) $(CFLAGS) -o "$@"

TARGETS = $(addprefix $(OBJ_DIR)/,$(basename $(LEXERS)))

$(TARGETS): % : $(OBJ_DIR) $(OBJS) %.o
	$(LD) -o "$@" $(OBJS) $@.o $(LDFLAGS)
	$(MIMESET) -f "$@"

.PHONY: all
all: $(TARGETS)
rmlibs ::
	-rm -f $(TARGETS)