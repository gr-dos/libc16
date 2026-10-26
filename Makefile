# libc16 Linux build.
#
#   make          build the static library build/LIBC16.LIB
#   make clean    remove build/obj and build/LIBC16.LIB
#                 (fetched tools in build/tools are kept)
#
# Needs a C compiler and make (to build JWasm), git, curl and xz. JWasm and
# Open Watcom are fetched into build/tools.

BUILD := build
OBJ   := $(BUILD)/obj
LIB   := $(BUILD)/LIBC16.LIB
TOOLS := $(BUILD)/tools.env

# Library members are compiled for the small model and the 8086, with no
# stack checks, and see no headers or libraries of Open Watcom's: -x ignores
# INCLUDE and WATCOM, and -zl leaves out default-library records.
WCFLAGS := -q -zq -zl -zld -x -ms -0 -os -s -bt=dos -wx -we
JWFLAGS := -q

SOURCES := $(wildcard crt/*.asm crt/*.c)
MEMBERS := $(patsubst %,$(OBJ)/%.obj,$(basename $(notdir $(SOURCES))))

all: $(LIB)

$(LIB): $(MEMBERS)
	rm -f $@
	. ./$(TOOLS) && PATH=$$WATCOM/binl64:$$PATH && wlib -q -n -b $@ $(addprefix +,$^)

$(OBJ)/%.obj: crt/%.asm $(TOOLS)
	mkdir -p $(@D)
	. ./$(TOOLS) && "$$JWASM" $(JWFLAGS) -Fo$@ $<

$(OBJ)/%.obj: crt/%.c $(wildcard crt/*.h) $(TOOLS)
	mkdir -p $(@D)
	. ./$(TOOLS) && PATH=$$WATCOM/binl64:$$PATH && wcc $(WCFLAGS) -fo=$@ $<

# Paths of the fetched tools, as shell assignments.
$(TOOLS): scripts/fetch-jwasm scripts/fetch-watcom $(wildcard patches/jwasm/*.patch)
	mkdir -p $(@D)
	jwasm=$$(scripts/fetch-jwasm) && watcom=$$(scripts/fetch-watcom) \
	  && printf 'JWASM=%s\nWATCOM=%s\nexport WATCOM\n' "$$jwasm" "$$watcom" > $@

clean:
	rm -rf $(OBJ) $(LIB)

.PHONY: all clean
.DELETE_ON_ERROR:
