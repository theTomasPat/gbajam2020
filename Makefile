
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

CFLAGS := $(ARCH) -O2 -Wall -fno-strict-aliasing
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
$(OBJS) : $(SRCS)
	$(CC) -c $< $(CFLAGS) -o $@


# --- Clean -----

clean: 
	@rm -fv build/*.gba
	@rm -fv build/*.elf
	@rm -fv build/*.o

#EOF
