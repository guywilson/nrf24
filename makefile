###############################################################################
#                                                                             #
# MAKEFILE for wctl2                                                          #
#                                                                             #
# (c) Guy Wilson 2023                                                         #
#                                                                             #
###############################################################################

# Version number for WCTL
MAJOR_VERSION = 0
MINOR_VERSION = 1

# Directories
SOURCE = src
BUILD = build
DEP = dep

# What is our target
TARGET = nrf24

# Tools
VBUILD = vbuild
C = gcc
LINKER = gcc

# postcompile step
PRECOMPILE = @ mkdir -p $(BUILD) $(DEP)
# postcompile step
POSTCOMPILE = @ mv -f $(DEP)/$*.Td $(DEP)/$*.d

CFLAGS = -c -O2 -Wall -pedantic -I/Users/guy/Library/include
DEPFLAGS = -MT $@ -MMD -MP -MF $(DEP)/$*.Td

# Libraries
STDLIBS =
EXTLIBS = -llgpio -lstrutils

COMPILE.c = $(C) $(CFLAGS) $(DEPFLAGS) -o $@
LINK.o = $(LINKER) $(STDLIBS) -o $@

CSRCFILES = $(wildcard $(SOURCE)/*.c)
OBJFILES = $(patsubst $(SOURCE)/%.c, $(BUILD)/%.o, $(CSRCFILES))
DEPFILES = $(patsubst $(SOURCE)/%.c, $(DEP)/%.d, $(CSRCFILES))

all: $(TARGET)

# Compile C/C++ source files
#
$(TARGET): $(OBJFILES)
	$(LINK.o) $^ $(EXTLIBS)

$(BUILD)/%.o: $(SOURCE)/%.c
$(BUILD)/%.o: $(SOURCE)/%.c $(DEP)/%.d
	$(PRECOMPILE)
	$(COMPILE.c) $<
	$(POSTCOMPILE)

.PRECIOUS = $(DEP)/%.d
$(DEP)/%.d: ;

-include $(DEPFILES)

install: $(TARGET)
	cp $(TARGET) /usr/local/bin

version:
	$(VBUILD) -incfile wctl.ver -template version.c.template -out $(SOURCE)/version.c -major $(MAJOR_VERSION) -minor $(MINOR_VERSION)

clean:
	rm -r $(BUILD)
	rm -r $(DEP)
	rm $(TARGET)
