#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------

ifeq ($(strip $(DEVKITPRO)),)
$(error "Please set DEVKITPRO in your environment. export DEVKITPRO=<path to>/devkitpro")
endif

TOPDIR ?= $(CURDIR)
include $(DEVKITPRO)/devkitA64/base_rules

all: 0_saltedkernel_90000000.bin

therainsdowninafrica/therainsdowninafrica.bin:
	@cd therainsdowninafrica && make

hashtagblessed/data/therainsdowninafrica.bin: therainsdowninafrica/therainsdowninafrica.bin
	@cp $< $@

hashtagblessed/hashtagblessed.bin: hashtagblessed/data/therainsdowninafrica.bin
	@cd hashtagblessed && make

hook1_900006CC.bin: hook1.s
	@$(AS) $< -o hook.out
	@$(OBJCOPY) -O binary hook.out $@
	@rm hook.out

patch1.bin: patch1.s
	@$(AS) $< -o patch.out
	@$(OBJCOPY) -O binary patch.out $@
	@rm patch.out

patch2.bin: patch2.s
	@$(AS) $< -o patch.out
	@$(OBJCOPY) -O binary patch.out $@
	@rm patch.out

0_saltedkernel_90000000.bin: 0_kernel_90000000.bin hook1_900006CC.bin patch1.bin patch2.bin hashtagblessed/hashtagblessed.bin
	@python2 kernelpatch.py 0_kernel_90000000.bin hook1_900006CC.bin patch1.bin patch2.bin hashtagblessed/hashtagblessed.bin $@
	@echo Patched to $@
	
clean:
	@cd therainsdowninafrica && make clean
	@cd hashtagblessed && make clean
	@rm -f hook1_900006CC.bin patch1.bin patch2.bin 0_saltedkernel_90000000.bin
