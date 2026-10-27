# libc16 Linux build.
#
#   make          build one static library per memory model (decision 11):
#                 build/s/LIBC16.LIB (small), build/c/LIBC16C.LIB (compact)
#                 and build/l/LIBC16L.LIB (large), and stage them, with the
#                 headers, in build/stage (the release layout)
#   make test     build the test programs against each library and run the
#                 checks in tests/ (ABI, link maps, 8086 instructions,
#                 selective linking) and the programs under DOSBox-X
#   make audit    compare each library's code with Open Watcom's runtime
#                 library for the same model and print only scores
#                 (decision 6)
#   make dist     build/dist/libc16-VERSION.zip and its .sha256, from the
#                 staged tree and the audit, and check the package
#   make install PREFIX=DIR
#                 copy the staged tree to DIR
#   make clean    remove build/s, build/c, build/l, build/stage and
#                 build/dist (fetched tools in build/tools are kept)
#
# MODEL=s, c or l limits lib, test and audit to one model, for example
# `make MODEL=c test`.
#
# Needs Perl 5.42 as `perl` on PATH, a C compiler and make (to build JWasm),
# git, curl and xz, and dosbox-x for the tests. JWasm and Open Watcom are
# fetched into build/tools.

VERSION := $(shell cat VERSION)

BUILD  := build
TOOLS  := $(BUILD)/tools.env
MODELS := s c l

# The library of each model, its name in a release's lib/, and the macro
# that selects the model in the assembly members (crt/model.inc), the same
# one wcc predefines.
LIBNAME_s := LIBC16.LIB
LIBNAME_c := LIBC16C.LIB
LIBNAME_l := LIBC16L.LIB
ASMDEF_s  := __SMALL__
ASMDEF_c  := __COMPACT__
ASMDEF_l  := __LARGE__
LIBS      := $(foreach m,$(MODELS),$(BUILD)/$(m)/$(LIBNAME_$(m)))

# Library members are compiled for the 8086, with no stack checks, and see
# no headers or libraries of Open Watcom's: -x ignores INCLUDE and WATCOM,
# and -zl leaves out default-library records. Members also see libc16's own
# headers, so that public functions are defined __cdecl as they are
# declared. Each model adds -mMODEL (wcc) or -D its macro (JWasm).
WCFLAGS_ALL := -q -zq -zl -zld -x -0 -os -s -bt=dos -wx -we -Iinclude
JWFLAGS_ALL := -q -Icrt

ifndef MODEL

all: lib stage

lib test audit:
	set -e; for m in $(MODELS); do $(MAKE) --no-print-directory MODEL=$$m $@; done

$(LIBS): FORCE
	$(MAKE) --no-print-directory MODEL=$(word 2,$(subst /, ,$@)) lib

else

ifeq ($(filter $(MODEL),$(MODELS)),)
$(error MODEL must be one of $(MODELS))
endif

MB  := $(BUILD)/$(MODEL)
OBJ := $(MB)/obj
LIB := $(MB)/$(LIBNAME_$(MODEL))

WCFLAGS := $(WCFLAGS_ALL) -m$(MODEL)
JWFLAGS := $(JWFLAGS_ALL) -D$(ASMDEF_$(MODEL))

SOURCES := $(wildcard crt/*.asm crt/*.c)
MEMBERS := $(patsubst %,$(OBJ)/%.obj,$(basename $(notdir $(SOURCES))))

lib: $(LIB)

$(LIB): $(MEMBERS)
	rm -f $@
	. ./$(TOOLS) && PATH=$$WATCOM/binl64:$$PATH && wlib -q -n -b $@ $(addprefix +,$^)

$(OBJ)/%.obj: crt/%.asm crt/model.inc $(TOOLS)
	mkdir -p $(@D)
	. ./$(TOOLS) && "$$JWASM" $(JWFLAGS) -Fo$@ $<

$(OBJ)/%.obj: crt/%.c $(wildcard crt/*.h include/*.h include/sys/*.h) $(TOOLS)
	mkdir -p $(@D)
	. ./$(TOOLS) && PATH=$$WATCOM/binl64:$$PATH && wcc $(WCFLAGS) -fo=$@ $<

# Test programs are built as a consumer builds (decision 2): the default
# register convention, the library's model, libc16's headers only, and no
# Open Watcom library.
# Each directory tests/SUITE with a cases file is a suite for tests/run-dos;
# every tests/SUITE/NAME.c is a program, linked with tests/support/testio.c,
# except the startup programs, whose rules follow.
TEST      := $(MB)/tests
TCFLAGS   := $(WCFLAGS) -Itests/support
HEADERS   := $(wildcard include/*.h include/sys/*.h) tests/support/testio.h
SUITES    := $(patsubst tests/%/cases,%,$(wildcard tests/*/cases))
STARTUP   := status dirty bss memory args stktop nullstr exitst init argfar
# ownent has its own entry point, ownstart, instead of the core startup.
OWNENT    := $(TEST)/startup/ownent.EXE $(TEST)/plus/ownent.EXE
TEST_EXES := $(patsubst %,$(TEST)/startup/%.EXE,$(STARTUP) memstk)
SUITE_SRCS := $(filter-out tests/startup/%,$(wildcard $(patsubst %,tests/%/*.c,$(SUITES))))
SUITE_EXES := $(patsubst tests/%.c,$(TEST)/%.EXE,$(SUITE_SRCS))
TEST_OBJS := $(patsubst %,$(TEST)/startup/%.obj,$(STARTUP)) $(TEST)/startup/codepad.obj \
	$(TEST)/startup/ownstart.obj $(TEST)/startup/ownent.obj \
	$(TEST)/startup/inittab.obj \
	$(TEST)/io/seekfail.obj $(TEST)/dos/esafter.obj \
	$(TEST)/support/testio.obj \
	$(SUITE_EXES:.EXE=.obj)

$(TEST)/%.obj: tests/%.c $(HEADERS) $(TOOLS)
	mkdir -p $(@D)
	. ./$(TOOLS) && PATH=$$WATCOM/binl64:$$PATH && wcc $(TCFLAGS) -fo=$@ $<

$(TEST)/%.obj: tests/%.asm crt/model.inc $(TOOLS)
	mkdir -p $(@D)
	. ./$(TOOLS) && "$$JWASM" $(JWFLAGS) -Fo$@ $<

# Selective linking (DESIGN.md, section 5): each program is linked again in
# TEST/plus against a copy of the library with an extra member that nothing
# references, tests/plus/unused.c, and must come out identical.
PLUS      := $(TEST)/plus
PLUS_LIB  := $(PLUS)/$(LIBNAME_$(MODEL))
PLUS_EXES := $(patsubst $(TEST)/startup/%,$(PLUS)/%,$(TEST_EXES))

$(PLUS)/%.obj: tests/plus/%.c $(TOOLS)
	mkdir -p $(@D)
	. ./$(TOOLS) && PATH=$$WATCOM/binl64:$$PATH && wcc $(WCFLAGS) -fo=$@ $<

$(PLUS_LIB): $(LIB) $(PLUS)/unused.obj
	cp $(LIB) $@
	. ./$(TOOLS) && PATH=$$WATCOM/binl64:$$PATH && wlib -q -b $@ +$(PLUS)/unused.obj

# Each program's objects; memstk is memory with 4 KiB more stack.
$(foreach d,$(TEST)/startup $(PLUS),$(d)/memory.EXE $(d)/memstk.EXE): $(TEST)/startup/memory.obj
$(foreach d,$(TEST)/startup $(PLUS),$(d)/args.EXE $(d)/memory.EXE $(d)/memstk.EXE $(d)/stktop.EXE \
	$(d)/init.EXE $(d)/argfar.EXE): $(TEST)/support/testio.obj
$(foreach d,$(TEST)/startup $(PLUS),$(d)/memstk.EXE): LDOPTS := option stack=6144
$(patsubst %,$(TEST)/startup/%.EXE,$(STARTUP)): $(TEST)/startup/%.EXE: $(TEST)/startup/%.obj
$(patsubst %,$(PLUS)/%.EXE,$(STARTUP)): $(PLUS)/%.EXE: $(TEST)/startup/%.obj
# nullstr is linked first, then codepad, which ends the code at a paragraph.
$(foreach d,$(TEST)/startup $(PLUS),$(d)/nullstr.EXE): $(TEST)/startup/codepad.obj
# init also links its own entries of the init table.
$(foreach d,$(TEST)/startup $(PLUS),$(d)/init.EXE): $(TEST)/startup/inittab.obj

$(SUITE_EXES): $(TEST)/%.EXE: $(TEST)/%.obj $(TEST)/support/testio.obj
$(OWNENT): $(TEST)/startup/ownstart.obj $(TEST)/startup/ownent.obj $(TEST)/support/testio.obj

# io also links a hook that makes chosen seeks fail.
$(TEST)/io/io.EXE: $(TEST)/io/seekfail.obj

# dos also links a helper that calls int86x with a chosen ES.
$(TEST)/dos/dos.EXE: $(TEST)/dos/esafter.obj

# bigstk runs heap.c's checks with a larger stack. The heap programs
# share tests/heap/check.h.
$(TEST)/heap/bigstk.obj: tests/heap/heap.c
$(filter $(TEST)/heap/%,$(SUITE_EXES:.EXE=.obj)): tests/heap/check.h
$(TEST)/heap/bigstk.EXE: LDOPTS := option stack=8192

$(TEST_EXES) $(SUITE_EXES) $(TEST)/startup/ownent.EXE: LINKLIB := $(LIB)
$(PLUS_EXES) $(PLUS)/ownent.EXE: LINKLIB := $(PLUS_LIB)
$(TEST_EXES) $(SUITE_EXES) $(TEST)/startup/ownent.EXE: $(LIB)
$(PLUS_EXES) $(PLUS)/ownent.EXE: $(PLUS_LIB)

# No default libraries: the map must show only the objects and the library
# (scripts/check-map), and X.MEMBERS lists the library members linked.
$(TEST_EXES) $(PLUS_EXES) $(SUITE_EXES) $(OWNENT): scripts/check-map
	mkdir -p $(@D)
	. ./$(TOOLS) && PATH=$$WATCOM/binl64:$$PATH && wlink option quiet, nodefaultlibs, \
	  map=$(@:.EXE=.MAP) $(LDOPTS) format dos name $@ $(foreach o,$(filter %.obj,$^),file $(o)) \
	  library $(LINKLIB)
	scripts/check-map $(@:.EXE=.MAP) $(LINKLIB) $(filter %.obj,$^) > $(@:.EXE=.MEMBERS)

# A suite's members file may have a variant for one model, MEMBERS.MODEL.
members = $(firstword $(wildcard tests/$(1)/members.$(MODEL)) tests/$(1)/members)

test: $(TEST_EXES) $(PLUS_EXES) $(SUITE_EXES) $(OWNENT)
	@echo '=== model $(MODEL)'
	. ./$(TOOLS) && scripts/check-8086 $(MEMBERS) $(TEST_OBJS) $(PLUS)/unused.obj
	. ./$(TOOLS) && PATH=$$WATCOM/binl64:$$PATH && tests/check-abi $(MODEL) $(TEST)/abi $(TCFLAGS)
	tests/check-members $(TEST)/startup $(call members,startup)
	tests/check-selective $(TEST)/startup $(PLUS)
	tests/run-dos $(TEST)/startup tests/startup/cases
	tests/startup/check-memory $(TEST)/startup
	tests/startup/check-null $(TEST_EXES:.EXE=.MAP) $(SUITE_EXES:.EXE=.MAP)
	set -e; for s in $(filter-out startup,$(SUITES)); do \
	  m=$$(ls tests/$$s/members.$(MODEL) tests/$$s/members 2>/dev/null | head -1); \
	  if [ -n "$$m" ]; then tests/check-members $(TEST)/$$s $$m; fi; \
	  tests/run-dos $(TEST)/$$s tests/$$s/cases; \
	done

# The similarity audit (decision 6) against the DOS runtime library and the
# math library of the same model in the pinned Open Watcom snapshot, and its
# loose DOS objects. Only scores are printed.
AUDIT := $(MB)/audit.txt

audit: $(LIB) scripts/similarity-audit $(TOOLS)
	. ./$(TOOLS) && scripts/similarity-audit $(LIB) $$WATCOM/lib286/dos/clib$(MODEL).lib \
	  $$WATCOM/lib286/math$(MODEL).lib $$WATCOM/lib286/dos/*.obj > $(AUDIT).tmp \
	  || { cat $(AUDIT).tmp; exit 1; }
	mv $(AUDIT).tmp $(AUDIT)
	cat $(AUDIT)

endif

# Paths of the fetched tools, as shell assignments.
$(TOOLS): scripts/fetch-jwasm scripts/fetch-watcom $(wildcard patches/jwasm/*.patch)
	mkdir -p $(@D)
	jwasm=$$(scripts/fetch-jwasm) && watcom=$$(scripts/fetch-watcom) \
	  && printf 'JWASM=%s\nWATCOM=%s\nexport WATCOM\n' "$$jwasm" "$$watcom" > $@

# The release layout (README.md, "Using a release"), the same for a
# release ZIP and a checkout: consumers point LIBC16_ROOT at either.
STAGE := $(BUILD)/stage
DOCS  := docs/ABI.md docs/CONSUMERS.md docs/DESIGN.md docs/LICENSE-PROVENANCE.md \
	$(wildcard docs/spec/*.md)
HDRS  := $(wildcard include/*.h include/sys/*.h)

stage: $(LIBS) $(HDRS) $(DOCS) VERSION LICENSE README.md scripts/fetch-watcom \
		scripts/build-info scripts/fetch-jwasm Makefile
	rm -rf $(STAGE).tmp
	mkdir -p $(STAGE).tmp/lib $(STAGE).tmp/tools
	cp VERSION LICENSE README.md $(STAGE).tmp/
	cp $(LIBS) $(STAGE).tmp/lib/
	cp -R include $(STAGE).tmp/
	mkdir -p $(STAGE).tmp/docs/spec
	for f in $(DOCS); do cp $$f $(STAGE).tmp/$$f; done
	cp scripts/fetch-watcom $(STAGE).tmp/tools/
	scripts/build-info '$(WCFLAGS_ALL)' '$(JWFLAGS_ALL)' > $(STAGE).tmp/BUILD.txt
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
	scripts/make-dist $(STAGE) $(foreach m,$(MODELS),$(BUILD)/$(m)/audit.txt) libc16-$(VERSION) $(ZIP)
	cd $(DIST) && sha256sum libc16-$(VERSION).zip > libc16-$(VERSION).zip.sha256
	scripts/check-dist $(ZIP) $(VERSION)

clean:
	rm -rf $(foreach m,$(MODELS),$(BUILD)/$(m)) $(STAGE) $(DIST)

FORCE:

.PHONY: all lib test audit stage install dist clean FORCE
.DELETE_ON_ERROR:
