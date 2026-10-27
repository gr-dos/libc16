# libc16 Linux build.
#
#   make          build the static library build/LIBC16.LIB and stage it,
#                 with the headers, in build/stage (the release layout)
#   make test     build the test programs against it and run the checks in
#                 tests/ (ABI, link maps, 8086 instructions, selective linking)
#                 and the programs under DOSBox-X
#   make audit    compare the library's code with Open Watcom's runtime
#                 library and print only scores (decision 6)
#   make dist     build/dist/libc16-VERSION.zip and its .sha256, from the
#                 staged tree and the audit, and check the package
#   make install PREFIX=DIR
#                 copy the staged tree to DIR
#   make clean    remove build/obj, build/tests, build/stage, build/dist
#                 and build/LIBC16.LIB (fetched tools in build/tools are kept)
#
# Needs Perl 5.42 as `perl` on PATH, a C compiler and make (to build JWasm),
# git, curl and xz, and dosbox-x for the tests. JWasm and Open Watcom are
# fetched into build/tools.

VERSION := $(shell cat VERSION)

BUILD := build
OBJ   := $(BUILD)/obj
LIB   := $(BUILD)/LIBC16.LIB
TOOLS := $(BUILD)/tools.env

# Library members are compiled for the small model and the 8086, with no
# stack checks, and see no headers or libraries of Open Watcom's: -x ignores
# INCLUDE and WATCOM, and -zl leaves out default-library records.
# Members also see libc16's own headers, so that public functions are
# defined __cdecl as they are declared.
WCFLAGS := -q -zq -zl -zld -x -ms -0 -os -s -bt=dos -wx -we -Iinclude
JWFLAGS := -q

SOURCES := $(wildcard crt/*.asm crt/*.c)
MEMBERS := $(patsubst %,$(OBJ)/%.obj,$(basename $(notdir $(SOURCES))))

all: $(LIB) stage

$(LIB): $(MEMBERS)
	rm -f $@
	. ./$(TOOLS) && PATH=$$WATCOM/binl64:$$PATH && wlib -q -n -b $@ $(addprefix +,$^)

$(OBJ)/%.obj: crt/%.asm $(TOOLS)
	mkdir -p $(@D)
	. ./$(TOOLS) && "$$JWASM" $(JWFLAGS) -Fo$@ $<

$(OBJ)/%.obj: crt/%.c $(wildcard crt/*.h include/*.h include/sys/*.h) $(TOOLS)
	mkdir -p $(@D)
	. ./$(TOOLS) && PATH=$$WATCOM/binl64:$$PATH && wcc $(WCFLAGS) -fo=$@ $<

# Test programs are built as a consumer builds (decision 2): the default
# register convention, libc16's headers only, and no Open Watcom library.
# Each directory tests/SUITE with a cases file is a suite for tests/run-dos;
# every tests/SUITE/NAME.c is a program, linked with tests/support/testio.c,
# except the startup programs, whose rules follow.
TEST      := $(BUILD)/tests
TCFLAGS   := -q -zq -zl -zld -x -ms -0 -os -s -bt=dos -wx -we -Iinclude -Itests/support
HEADERS   := $(wildcard include/*.h include/sys/*.h) tests/support/testio.h
SUITES    := $(patsubst tests/%/cases,%,$(wildcard tests/*/cases))
STARTUP   := status dirty bss memory args stktop nullstr
TEST_EXES := $(patsubst %,$(TEST)/startup/%.EXE,$(STARTUP) memstk)
SUITE_SRCS := $(filter-out tests/startup/%,$(wildcard $(patsubst %,tests/%/*.c,$(SUITES))))
SUITE_EXES := $(patsubst tests/%.c,$(TEST)/%.EXE,$(SUITE_SRCS))
TEST_OBJS := $(patsubst %,$(TEST)/startup/%.obj,$(STARTUP)) $(TEST)/startup/codepad.obj \
	$(TEST)/io/seekfail.obj \
	$(TEST)/support/testio.obj \
	$(SUITE_EXES:.EXE=.obj)

$(TEST)/%.obj: tests/%.c $(HEADERS) $(TOOLS)
	mkdir -p $(@D)
	. ./$(TOOLS) && PATH=$$WATCOM/binl64:$$PATH && wcc $(TCFLAGS) -fo=$@ $<

$(TEST)/%.obj: tests/%.asm $(TOOLS)
	mkdir -p $(@D)
	. ./$(TOOLS) && "$$JWASM" $(JWFLAGS) -Fo$@ $<

# Selective linking (DESIGN.md, section 5): each program is linked again in
# build/tests/plus against a copy of LIBC16.LIB with an extra member that
# nothing references, tests/plus/unused.c, and must come out identical.
PLUS      := $(TEST)/plus
PLUS_LIB  := $(PLUS)/LIBC16.LIB
PLUS_EXES := $(patsubst $(TEST)/startup/%,$(PLUS)/%,$(TEST_EXES))

$(PLUS)/%.obj: tests/plus/%.c $(TOOLS)
	mkdir -p $(@D)
	. ./$(TOOLS) && PATH=$$WATCOM/binl64:$$PATH && wcc $(WCFLAGS) -fo=$@ $<

$(PLUS_LIB): $(LIB) $(PLUS)/unused.obj
	cp $(LIB) $@
	. ./$(TOOLS) && PATH=$$WATCOM/binl64:$$PATH && wlib -q -b $@ +$(PLUS)/unused.obj

# Each program's objects; memstk is memory with 4 KiB more stack.
$(foreach d,$(TEST)/startup $(PLUS),$(d)/memory.EXE $(d)/memstk.EXE): $(TEST)/startup/memory.obj
$(foreach d,$(TEST)/startup $(PLUS),$(d)/args.EXE $(d)/memory.EXE $(d)/memstk.EXE $(d)/stktop.EXE): \
	$(TEST)/support/testio.obj
$(foreach d,$(TEST)/startup $(PLUS),$(d)/memstk.EXE): LDOPTS := option stack=6144
$(patsubst %,$(TEST)/startup/%.EXE,$(STARTUP)): $(TEST)/startup/%.EXE: $(TEST)/startup/%.obj
$(patsubst %,$(PLUS)/%.EXE,$(STARTUP)): $(PLUS)/%.EXE: $(TEST)/startup/%.obj
# nullstr is linked first, then codepad, which ends the code at a paragraph.
$(foreach d,$(TEST)/startup $(PLUS),$(d)/nullstr.EXE): $(TEST)/startup/codepad.obj

$(SUITE_EXES): $(TEST)/%.EXE: $(TEST)/%.obj $(TEST)/support/testio.obj

# io also links a hook that makes chosen seeks fail.
$(TEST)/io/io.EXE: $(TEST)/io/seekfail.obj

# bigstk runs heap.c's checks with a larger stack.
$(TEST)/heap/bigstk.obj: tests/heap/heap.c
$(TEST)/heap/bigstk.EXE: LDOPTS := option stack=8192

$(TEST_EXES) $(SUITE_EXES): LINKLIB := $(LIB)
$(PLUS_EXES): LINKLIB := $(PLUS_LIB)
$(TEST_EXES) $(SUITE_EXES): $(LIB)
$(PLUS_EXES): $(PLUS_LIB)

# No default libraries: the map must show only the objects and the library
# (scripts/check-map), and X.MEMBERS lists the library members linked.
$(TEST_EXES) $(PLUS_EXES) $(SUITE_EXES): scripts/check-map
	mkdir -p $(@D)
	. ./$(TOOLS) && PATH=$$WATCOM/binl64:$$PATH && wlink option quiet, nodefaultlibs, \
	  map=$(@:.EXE=.MAP) $(LDOPTS) format dos name $@ $(foreach o,$(filter %.obj,$^),file $(o)) \
	  library $(LINKLIB)
	scripts/check-map $(@:.EXE=.MAP) $(LINKLIB) $(filter %.obj,$^) > $(@:.EXE=.MEMBERS)

test: $(TEST_EXES) $(PLUS_EXES) $(SUITE_EXES)
	. ./$(TOOLS) && scripts/check-8086 $(MEMBERS) $(TEST_OBJS) $(PLUS)/unused.obj
	. ./$(TOOLS) && PATH=$$WATCOM/binl64:$$PATH && tests/check-abi $(TEST)/abi $(TCFLAGS)
	tests/check-members $(TEST)/startup tests/startup/members
	tests/check-selective $(TEST)/startup $(PLUS)
	tests/run-dos $(TEST)/startup tests/startup/cases
	tests/startup/check-memory $(TEST)/startup
	tests/startup/check-null $(TEST_EXES:.EXE=.MAP) $(SUITE_EXES:.EXE=.MAP)
	set -e; for s in $(filter-out startup,$(SUITES)); do \
	  if [ -e tests/$$s/members ]; then tests/check-members $(TEST)/$$s tests/$$s/members; fi; \
	  tests/run-dos $(TEST)/$$s tests/$$s/cases; \
	done

# Paths of the fetched tools, as shell assignments.
$(TOOLS): scripts/fetch-jwasm scripts/fetch-watcom $(wildcard patches/jwasm/*.patch)
	mkdir -p $(@D)
	jwasm=$$(scripts/fetch-jwasm) && watcom=$$(scripts/fetch-watcom) \
	  && printf 'JWASM=%s\nWATCOM=%s\nexport WATCOM\n' "$$jwasm" "$$watcom" > $@

# The similarity audit (decision 6) against the small-model DOS runtime
# library, the math library and the loose DOS objects of the pinned Open
# Watcom snapshot. Only scores are printed.
AUDIT := $(BUILD)/audit.txt

audit: $(LIB) scripts/similarity-audit $(TOOLS)
	. ./$(TOOLS) && scripts/similarity-audit $(LIB) $$WATCOM/lib286/dos/clibs.lib \
	  $$WATCOM/lib286/maths.lib $$WATCOM/lib286/dos/*.obj > $(AUDIT).tmp \
	  || { cat $(AUDIT).tmp; exit 1; }
	mv $(AUDIT).tmp $(AUDIT)
	cat $(AUDIT)

# The release layout (README.md, "Using a release"), the same for a
# release ZIP and a checkout: consumers point LIBC16_ROOT at either.
STAGE := $(BUILD)/stage
DOCS  := docs/ABI.md docs/CONSUMERS.md docs/DESIGN.md docs/LICENSE-PROVENANCE.md \
	$(wildcard docs/spec/*.md)
HDRS  := $(wildcard include/*.h include/sys/*.h)

stage: $(LIB) $(HDRS) $(DOCS) VERSION LICENSE README.md scripts/fetch-watcom \
		scripts/build-info scripts/fetch-jwasm Makefile
	rm -rf $(STAGE).tmp
	mkdir -p $(STAGE).tmp/lib $(STAGE).tmp/tools
	cp VERSION LICENSE README.md $(STAGE).tmp/
	cp $(LIB) $(STAGE).tmp/lib/
	cp -R include $(STAGE).tmp/
	mkdir -p $(STAGE).tmp/docs/spec
	for f in $(DOCS); do cp $$f $(STAGE).tmp/$$f; done
	cp scripts/fetch-watcom $(STAGE).tmp/tools/
	scripts/build-info '$(WCFLAGS)' '$(JWFLAGS)' > $(STAGE).tmp/BUILD.txt
	rm -rf $(STAGE)
	mv $(STAGE).tmp $(STAGE)

install: stage
	@test -n "$(PREFIX)" || { echo 'usage: make install PREFIX=DIR' >&2; exit 1; }
	mkdir -p $(PREFIX)
	cp -R $(STAGE)/. $(PREFIX)/

DIST := $(BUILD)/dist
ZIP  := $(DIST)/libc16-$(VERSION).zip

dist: stage audit scripts/make-dist scripts/check-dist
	mkdir -p $(DIST)
	scripts/make-dist $(STAGE) $(AUDIT) libc16-$(VERSION) $(ZIP)
	cd $(DIST) && sha256sum libc16-$(VERSION).zip > libc16-$(VERSION).zip.sha256
	scripts/check-dist $(ZIP) $(VERSION)

clean:
	rm -rf $(OBJ) $(BUILD)/tests $(LIB) $(STAGE) $(DIST) $(AUDIT)

.PHONY: all test audit stage install dist clean
.DELETE_ON_ERROR:
