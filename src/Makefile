TARGET := ftpservu

#VERBOSE   = 1

BUILD_DIR = objs

# Set up compiler for DevKitPro toolchain
ifeq ($(strip $(DEVKITPPC)),)
  $(error "Please set DEVKITPPC in your environment.")
endif

ifeq ($(strip $(DEVKITPRO)),)
  $(error "Please set DEVKITPRO in your environment.")
endif

export PATH := $(PATH):$(DEVKITPPC)/bin
PREFIX := powerpc-eabi-

CC      := $(PREFIX)gcc
CXX     := $(PREFIX)g++
AS      := $(PREFIX)as
AR      := $(PREFIX)ar
OBJCOPY := $(PREFIX)objcopy
STRIP   := $(PREFIX)strip
NM      := $(PREFIX)nm
LD      := $(CXX)

# Set make verbosity
ifneq ($(VERBOSE), 1)
  Q := @
endif

# Object files included in project
OBJ :=
OBJ += ftpservu.o
OBJ += network.o
OBJ += ftp.o
OBJ += input.o
OBJ += iobuffer.o
OBJ += commands.o
OBJ += console.o
OBJ += logging.o
OBJ += main.o
OBJ += dtp/dtp.o
OBJ += dtp/passive.o
OBJ += dtp/active.o
OBJ += hbl.o
OBJ += fs/sd_fat_devoptab.o
OBJ += fs/fs_utils.o
OBJ += vfs/vfs.o
OBJ += vfs/vfs_path.o
OBJ += vfs/vfs_pathent.o
OBJ += system/memory.o
OBJ += system/exception_handler.o
OBJ += system/missing_libc_functions.o
OBJ += commands/allo.o
OBJ += commands/appe.o
OBJ += commands/cdup.o
OBJ += commands/cwd.o
OBJ += commands/dele.o
OBJ += commands/list.o
OBJ += commands/mkd.o
OBJ += commands/mode.o
OBJ += commands/nlst.o
OBJ += commands/noop.o
OBJ += commands/pass.o
OBJ += commands/pasv.o
OBJ += commands/port.o
OBJ += commands/pwd.o
OBJ += commands/quit.o
OBJ += commands/rein.o
OBJ += commands/rest.o
OBJ += commands/retr.o
OBJ += commands/rmd.o
OBJ += commands/rnfr.o
OBJ += commands/rnto.o
OBJ += commands/site.o
OBJ += commands/size.o
OBJ += commands/stor.o
OBJ += commands/syst.o
OBJ += commands/type.o
OBJ += commands/user.o

OBJ := $(addprefix $(BUILD_DIR)/, $(OBJ))

DEFINES :=

INCDIRS := -I. -Iinclude -I$(DEVKITPRO)/portlibs/ppc/include
LIBDIRS := -L. -L$(DEVKITPRO)/portlibs/ppc/lib

CFLAGS_COMMON := -O3 -mwup -mcpu=750 -meabi -mhard-float

CFLAGS  := $(CFLAGS_COMMON) -ffast-math -Werror=implicit-function-declaration
CXXFLAGS = $(CFLAGS_COMMON) -fno-rtti -fno-exceptions
LDFLAGS := $(CFLAGS_COMMON) -Wl,--gc-sections
ASFLAGS := $(CFLAGS_COMMON) -mregnames

LIBS := -lm -lfat -liosuhax

RPX_OBJ = $(BUILD_DIR)/system/stubs_rpl.o
HBL_ELF_OBJ = $(BUILD_DIR)/system/dynamic.o $(BUILD_DIR)/system/stubs_elf.o

RPX_LDFLAGS := -pie -fPIE
RPX_LDFLAGS += -z common-page-size=64 -z max-page-size=64
RPX_LDFLAGS += -T link_rpl.ld -nostartfiles

HBL_ELF_LDFLAGS := -T link_elf.ld

TARGETS := $(TARGET).rpx $(TARGET).elf

DEPFLAGS = -MT $@ -MMD -MP -MF $(BUILD_DIR)/$*.depend

ELF2RPL := wut/elf2rpl/elf2rpl

all: $(TARGETS)

%: $(BUILD_DIR)/%
	cp $< $@

$(BUILD_DIR)/%.o: %.cpp %.depend
	@$(if $(Q), echo CXX $<,)
	@$(mkdir -p $(dir $@)
	$(Q)$(CXX) -c -o $@ $< $(CXXFLAGS) $(DEFINES) $(INCDIRS) $(DEPFLAGS)

$(BUILD_DIR)/%.o: %.c %.depend
	@$(if $(Q), echo CC $<,)
	@mkdir -p $(dir $@)
	$(Q)$(CC) -c -o $@ $< $(CFLAGS) $(DEFINES) $(INCDIRS) $(DEPFLAGS)

$(BUILD_DIR)/%.o: %.S %.depend
	@$(if $(Q), echo AS $<,)
	@mkdir -p $(dir $@)
	$(Q)$(CC) -c -o $@ $< $(ASFLAGS) $(DEFINES) $(INCDIRS) $(DEPFLAGS)

$(BUILD_DIR)/%.o: %.s %.depend
	@$(if $(Q), echo AS $<,)
	@mkdir -p $(dir $@)
	$(Q)$(CC) -c -o $@ $< $(ASFLAGS) $(INCDIRS) $(DEPFLAGS)

%.a:
	@$(if $(Q), echo AR $<,)
	@mkdir -p $(dir $@)
	$(Q)$(AR) -rc $@ $^

%.depend: ;
%.last: ;

$(ELF2RPL):
	@$(if $(Q), echo MAKE $@,)
	$(Q)$(MAKE) -C wut/elf2rpl/

$(BUILD_DIR)/$(TARGET).elf: $(OBJ) $(HBL_ELF_OBJ) link_elf.ld .$(TARGET).elf.last
	@$(if $(Q), echo LD $@,)
	@touch .$(TARGET).elf.last
	$(Q)$(LD) $(OBJ) $(HBL_ELF_OBJ) $(LDFLAGS) $(HBL_ELF_LDFLAGS) $(LIBDIRS) $(LIBS) -o $@

$(BUILD_DIR)/$(TARGET).rpx.elf: $(OBJ) $(RPX_OBJ) link_elf.ld
	@$(if $(Q), echo LD $@,)
	$(Q)$(LD) $(OBJ) $(RPX_OBJ) $(LDFLAGS) $(RPX_LDFLAGS) $(LIBDIRS) $(LIBS) -o $@

$(BUILD_DIR)/$(TARGET).rpx: $(BUILD_DIR)/$(TARGET).rpx.elf $(ELF2RPL) .$(TARGET).rpx.last
	@$(if $(Q), echo ELF2RPL $@,)
	@touch .$(TARGET).rpx.last
	$(Q)-$(ELF2RPL) $< $@

compile: $(OBJ) $(HBL_ELF_OBJ)

elf: $(BUILD_DIR)/$(TARGET).elf
	cp $(BUILD_DIR)/$(TARGET).elf .

rpx: $(BUILD_DIR)/$(TARGET).rpx
	cp $(BUILD_DIR)/$(TARGET).rpx .

clean:
	rm -f $(OBJ) $(RPX_OBJ) $(HBL_ELF_OBJ) $(TARGET).elf $(TARGET).rpx.elf $(TARGET).rpx
	rm -f .$(TARGET).elf.last .$(TARGET).rpx.elf.last .$(TARGET).rpx.last
	rm -f $(OBJ:.o=.depend) $(RPX_OBJ:.o=.depend) $(HBL_ELF_OBJ:.o=.depend)

-include $(OBJ:.o=.depend) $(RPX_OBJ:.o=.depend) $(HBL_ELF_OBJ:.o=.depend)

