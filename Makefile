
PATH := $(DEVKITARM)/bin:$(PATH)

# --- Project details -----

PROJ := GBAJam2022
BUILDDIR := build
SRCDIR := source
TARGET := $(BUILDDIR)/$(PROJ)

SRCS := $(wildcard $(SRCDIR)/*.c)
OBJS := $(patsubst $(SRCDIR)/%.c, $(BUILDDIR)/%.o, $(SRCS))

# --- Build defines -----

PREFIX := arm-none-eabi-
CC := $(PREFIX)gcc
LD := $(PREFIX)gcc
OBJCOPY := $(PREFIX)objcopy

ARCH := -mthumb-interwork -mthumb
SPECS := -specs=gba.specs

DEBUG_FLAGS := -g -O0 -D __DEBUG__
CFLAGS := $(ARCH) -std=c99 -Wall -O3 -fno-strict-aliasing -IC:/devkitPro/devkitARM/arm-none-eabi/include -I./source
LDFLAGS := $(ARCH) $(SPECS)


.PHONY : build clean

# --- Build -----
# Build process starts here
build: $(TARGET).gba

# Strip and fix header (step 3, 4)
$(TARGET).gba : $(TARGET).elf
	$(OBJCOPY) -v -O binary $< $@
	-@gbafix $@

# Link (step 2)
$(TARGET).elf : $(OBJS)
	$(LD) $^ $(LDFLAGS) -o $@

# Compile (step 1)
$(BUILDDIR)/%.o : $(SRCDIR)/%.c
	$(CC) -c $< $(CFLAGS) $(DEBUG_FLAGS) -o $@
	#$(CC) -c $< $(CFLAGS) -o $@


# --- Clean -----

clean: 
	@rm -fv build/*.gba
	@rm -fv build/*.elf
	@rm -fv build/*.o

#EOF
