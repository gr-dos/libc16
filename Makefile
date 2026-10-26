# libc16 Linux build.
#
#   make          build the static library build/LIBC16.LIB
#   make test     build the test programs against it and run the checks in
#                 tests/ (link maps, 8086 instructions) and the programs
#                 under DOSBox-X
#   make clean    remove build/obj, build/tests and build/LIBC16.LIB
#                 (fetched tools in build/tools are kept)
#
# Needs Perl 5.42 as `perl` on PATH, a C compiler and make (to build JWasm),
# git, curl and xz, and dosbox-x for the tests. JWasm and Open Watcom are
# fetched into build/tools.

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

# Test programs are built as a consumer builds (decision 2): the default
# register convention, libc16's headers only, and no Open Watcom library.
TEST     := $(BUILD)/tests
TCFLAGS  := -q -zq -zl -zld -x -ms -0 -os -s -bt=dos -wx -we -Itests/support
STARTUP  := status dirty bss memory args
TEST_EXES := $(patsubst %,$(TEST)/startup/%.EXE,$(STARTUP) memstk)
TEST_OBJS := $(patsubst %,$(TEST)/startup/%.obj,$(STARTUP)) $(TEST)/support/testio.obj

$(TEST)/support/%.obj: tests/support/%.c tests/support/testio.h $(TOOLS)
	mkdir -p $(@D)
	. ./$(TOOLS) && PATH=$$WATCOM/binl64:$$PATH && wcc $(TCFLAGS) -fo=$@ $<

$(TEST)/startup/%.obj: tests/startup/%.c tests/support/testio.h $(TOOLS)
	mkdir -p $(@D)
	. ./$(TOOLS) && PATH=$$WATCOM/binl64:$$PATH && wcc $(TCFLAGS) -fo=$@ $<

# Each program's objects; memstk is memory with 4 KiB more stack.
$(TEST)/startup/memory.EXE $(TEST)/startup/memstk.EXE: $(TEST)/startup/memory.obj
$(TEST)/startup/args.EXE $(TEST)/startup/memory.EXE $(TEST)/startup/memstk.EXE: \
	$(TEST)/support/testio.obj
$(TEST)/startup/memstk.EXE: LDOPTS := option stack=6144
$(patsubst %,$(TEST)/startup/%.EXE,$(STARTUP)): $(TEST)/startup/%.EXE: $(TEST)/startup/%.obj

# No default libraries: the map must show only the objects and LIBC16.LIB
# (scripts/check-map), and X.MEMBERS lists the library members linked.
$(TEST_EXES): $(LIB) scripts/check-map
	mkdir -p $(@D)
	. ./$(TOOLS) && PATH=$$WATCOM/binl64:$$PATH && wlink option quiet, nodefaultlibs, \
	  map=$(@:.EXE=.MAP) $(LDOPTS) format dos name $@ $(foreach o,$(filter %.obj,$^),file $(o)) \
	  library $(LIB)
	scripts/check-map $(@:.EXE=.MAP) $(LIB) $(filter %.obj,$^) > $(@:.EXE=.MEMBERS)

test: $(TEST_EXES)
	. ./$(TOOLS) && scripts/check-8086 $(MEMBERS) $(TEST_OBJS)
	tests/check-members $(TEST)/startup tests/startup/members
	tests/run-dos $(TEST)/startup tests/startup/cases
	tests/startup/check-memory $(TEST)/startup

# Paths of the fetched tools, as shell assignments.
$(TOOLS): scripts/fetch-jwasm scripts/fetch-watcom $(wildcard patches/jwasm/*.patch)
	mkdir -p $(@D)
	jwasm=$$(scripts/fetch-jwasm) && watcom=$$(scripts/fetch-watcom) \
	  && printf 'JWASM=%s\nWATCOM=%s\nexport WATCOM\n' "$$jwasm" "$$watcom" > $@

clean:
	rm -rf $(OBJ) $(BUILD)/tests $(LIB)

.PHONY: all test clean
.DELETE_ON_ERROR:
