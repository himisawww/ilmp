BUILD_DIR := build
STATIC_LIBRARY := $(BUILD_DIR)/libilmp.o
SHARED_LIBRARY := $(BUILD_DIR)/libilmp.so
TARGET := $(BUILD_DIR)/ilmPi
INSTALL_DIR := /usr/lib
INSTALL_PATH := $(INSTALL_DIR)/libilmp.so

ASMFLAGS := -w+all
CFLAGS   := -O2 -fwrapv -fno-strict-aliasing -fkeep-inline-functions
CPPFLAGS := -O2 -fwrapv -fno-strict-aliasing

#================  CPP  ================

CPP_SRCDIR := ilmPi
CPP_SOURCES := $(wildcard $(CPP_SRCDIR)/*.cpp)

#===============  BUILD  ===============
.SECONDARY:
static: $(STATIC_LIBRARY) $(CPP_SOURCES)
	g++ $(CPPFLAGS) $(CPP_SOURCES) $(STATIC_LIBRARY) -o $(TARGET)
shared: $(SHARED_LIBRARY) $(CPP_SOURCES)
	g++ $(CPPFLAGS) $(CPP_SOURCES) -L $(BUILD_DIR)/ -lilmp -o $(TARGET)
install:
	cp -i $(SHARED_LIBRARY) $(INSTALL_PATH)
	ldconfig
uninstall:
	rm -i $(INSTALL_PATH)
	ldconfig

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)

#=============      C      =============

C_SRCDIR := ilmp
C_SOURCES := $(wildcard $(C_SRCDIR)/*.c)
C_OBJECTS := $(patsubst $(C_SRCDIR)/%.c,$(BUILD_DIR)/%.c.o,$(C_SOURCES))

$(BUILD_DIR)/%.c.o: $(C_SRCDIR)/%.c
	g++ -c -fPIC $(CFLAGS) $< -o $@
	
#=============     ASM     =============

MASM_SRCDIR  := ilmp/masm
MASM_SOURCES := $(wildcard $(MASM_SRCDIR)/*)
MASM_UNITS   := $(wildcard $(MASM_SRCDIR)/*.asm)
NASM_SOURCES := $(patsubst $(MASM_SRCDIR)/%,$(BUILD_DIR)/%,$(MASM_SOURCES))
NASM_OBJECTS := $(patsubst $(MASM_SRCDIR)/%.asm,$(BUILD_DIR)/%.asm.o,$(MASM_UNITS))
ASM_LIBRARY  := $(BUILD_DIR)/ilmp_asm.o

# patch masm to nasm
$(BUILD_DIR)/%: $(MASM_SRCDIR)/% | $(BUILD_DIR)
	@cat $< | sed -E\
	 -e 's/\r//g'\
	 -e 's/\t/ /g'\
	 -e 's/.+ TEXTEQU *<;>.*//g'\
	 -e 's/^win.*//g'\
	 -e 's/asm_windows/asm_linux/g'\
	 -e 's/^\.code/default rel\nsection .text/g'\
	 -e 's/^end//g'\
	 -e 's/^([_a-zA-Z][_a-zA-Z0-9]*) proc/\1:/g'\
	 -e 's/^([_a-zA-Z][_a-zA-Z0-9]*) endp/global \1/g'\
	 -e 's/^(.+) TEXTEQU *<([^>]*)>/%define \1 \2/g'\
	 -e 's/^include *<([^>]+)>/%include"\1"/g'\
	 -e 's/(lab_[_a-zA-Z0-9]+)/.\1/g'\
	 -e 's/ ptr */ /g'\
	 -e 's/:near//g'\
	 | awk 1 ORS='\t'\
	 | sed -E -e 's/^(.*)%define +ifq +\t%include"([^"]+)"(.*)$$/\1%include"\2_ifq"\3/g'\
	 -e 's/\t/\n/g' > $@
	@grep '.lab_ent::' $@ | if [ ! -z "$$(cat)" ]; then sed -i -E 's/\.lab_ent:?/lab_ent/g' $@; fi

$(BUILD_DIR)/%.asm.o: $(BUILD_DIR)/%.asm $(NASM_SOURCES)
	@cat $< | sed -n -E 's/%include"([^"]+)_ifq"/\1/p' | tee $@ | if [ ! -z "$$(cat)" ]; then\
	 sed 's/ifq//g' $(BUILD_DIR)/$$(cat $@) > $(BUILD_DIR)/$$(cat $@)_ifq ;\
	 sed -i 's/ifq/;/g' $(BUILD_DIR)/$$(cat $@);\
	 fi
	nasm -f elf64 -i $(BUILD_DIR) $(ASMFLAGS) $< -o $@
	
#=============  LIBRARIES  =============

$(SHARED_LIBRARY): $(NASM_OBJECTS) $(C_OBJECTS)
	ld -no-pie -static -e 0 -o $(ASM_LIBRARY) $(NASM_OBJECTS)
	ld -static -shared -e 0 -o $(SHARED_LIBRARY) $(C_OBJECTS) $(ASM_LIBRARY)
	strip $(SHARED_LIBRARY)

$(STATIC_LIBRARY): $(NASM_OBJECTS) $(C_OBJECTS)
	ld -r -static -e 0 -o $(STATIC_LIBRARY) $(NASM_OBJECTS) $(C_OBJECTS)
