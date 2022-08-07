C_SOURCES = $(wildcard kernel/*.c drivers/*.c drivers/ata/*.c cpu/*.c libc/*.c)
HEADERS = $(wildcard kernel/*.h drivers/*.h drivers/ata/*.h cpu/*.h libc/*.h)

# Nice syntax for file extension replacement
OBJ = ${C_SOURCES:.c=.o cpu/interrupt.o kernel/switch.o}

OUT_DIR = out

# First rule is run by default
profanOS.bin: boot/bootsect.bin kernel.bin hdd.bin
	cat out/bootsect.bin out/kernel.bin > profanOS.bin

# some variables to build the ISO.

FILESIZE = $(shell stat -c%s "profanOS.bin")
SECTORSIZE = 512
DIVIDED = $(shell echo $$(($(FILESIZE) / $(SECTORSIZE))))
FLOPPYSECS = 2876
FILLERSIZE = $(shell echo $$(($(FLOPPYSECS) - $(DIVIDED))))


iso: profanOS.bin
	mkdir -p $(OUT_DIR)
	$(info Filesize: $(FILESIZE), No. of sectors: $(DIVIDED))
	dd if="/dev/zero" bs=$(SECTORSIZE) count=$(FILLERSIZE) of="$(OUT_DIR)/_filler.img"
	$(info Filler created.)
	cat profanOS.bin $(OUT_DIR)/_filler.img > $(OUT_DIR)/_floppy.img
	$(info Secondary image created.)
	touch $(OUT_DIR)/_floppy.catalog
	mkisofs -input-charset iso8859-1 -r -b "$(OUT_DIR)/_floppy.img" -c "$(OUT_DIR)/_floppy.catalog" -o "profanOS.iso" .


# '--oformat binary' deletes all symbols as a collateral, so we don't need
# to 'strip' them manually on this case
kernel.bin: boot/kernel_entry.o ${OBJ}
	@python3 maketool.py ld out/$@ $^

# create a 1MB hdd.bin file
hdd.bin:
	dd if=/dev/zero of=hdd.bin bs=1024 count=1024

run: profanOS.bin hdd.bin
	qemu-system-i386 -drive file=profanOS.bin,if=floppy,format=raw -drive file=hdd.bin,format=raw -boot order=a

iso_run: iso
	qemu-system-i386 -cdrom profanOS.iso -boot order=d

# Generic rules for wildcards
# To make an object, always compile from its .c
%.o: %.c ${HEADERS}
	@python3 maketool.py gcc $< $@

%.o: %.asm
	@python3 maketool.py nasm-elf $< $@

%.bin: %.asm
	@python3 maketool.py nasm-bin $< $@

clean:
	rm -rf out/

fullclean: clean
	rm -rf *.bin *.iso
