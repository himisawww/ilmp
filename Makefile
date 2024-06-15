BUILD_DIR := build
SHARED_LIBRARY := $(BUILD_DIR)/libilmp.so
TARGET := $(BUILD_DIR)/ilmPi

CFLAGS := -Ofast -fwrapv -fno-strict-aliasing -fkeep-inline-functions

#================  CPP  ================

CPP_SRCDIR := ilmPi
CPP_SOURCES := $(wildcard $(CPP_SRCDIR)/*.cpp)

.SECONDARY:
$(TARGET): $(SHARED_LIBRARY) $(CPP_SOURCES)
	g++ $(CFLAGS) $(CPP_SOURCES) -L $(BUILD_DIR)/ -lilmp -o $@

#===============  BUILD  ===============

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)

#=============  SHAREDLIB  =============

C_SRCDIR := ilmp
C_SOURCES := $(wildcard $(C_SRCDIR)/*.c)
C_OBJECTS := $(patsubst $(C_SRCDIR)/%.c,$(BUILD_DIR)/%.c.o,$(C_SOURCES))

$(BUILD_DIR)/%.c.o: $(C_SRCDIR)/%.c
	g++ -c -fPIC $(CFLAGS) $< -o $@

MASM_SRCDIR  := ilmp/masm
MASM_SOURCES := $(wildcard $(MASM_SRCDIR)/*)
MASM_UNITS   := $(wildcard $(MASM_SRCDIR)/*.asm)
NASM_SOURCES := $(patsubst $(MASM_SRCDIR)/%,$(BUILD_DIR)/%,$(MASM_SOURCES))
NASM_OBJECTS := $(patsubst $(MASM_SRCDIR)/%.asm,$(BUILD_DIR)/%.asm.o,$(MASM_UNITS))
ASM_LIBRARY  := $(BUILD_DIR)/ilmp_asm.o

# patch masm to nasm
$(BUILD_DIR)/%: $(MASM_SRCDIR)/% | $(BUILD_DIR)
	@cat $< \
	| sed 's/^\.code/default rel\r\nsection .text/g'\
	| sed 's/^end//g' | sed -E 's/^([_a-zA-Z][_a-zA-Z0-9]*) proc/\1:/g'\
	| sed -E 's/^([_a-zA-Z][_a-zA-Z0-9]*) endp/global \1/g'\
	| sed -E 's/^(.+) TEXTEQU +<([^>]*)>/%define \1 \2/g'\
	| sed -E 's/^include *<([^>]+)>/%include"\1"/g'\
	| sed -E 's/(lab_[_a-zA-Z0-9]+)/.\1/g'\
	| sed -E 's/ ptr */ /g'\
	| sed -E 's/:near//g'\
	> $@
	@grep '.lab_ent::' $@ | if [ ! -z $$(cat) ]; then sed -i -E 's/\.lab_ent:?/lab_ent/g' $@; fi

$(BUILD_DIR)/%.asm.o: $(BUILD_DIR)/%.asm $(NASM_SOURCES)
	nasm -f elf64 -i $(BUILD_DIR) $< -o $@

$(SHARED_LIBRARY): $(NASM_OBJECTS) $(C_OBJECTS)
	ld -no-pie -static -e 0 -o $(ASM_LIBRARY) $(NASM_OBJECTS)
	ld -static -shared -e 0 -o $(SHARED_LIBRARY) $(C_OBJECTS) $(ASM_LIBRARY)
	strip $(SHARED_LIBRARY)
