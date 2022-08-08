OUT_DIR = out

# First rule is run by default
bin_image:
	python3 maketool.py bin_image

info:
	@python3 maketool.py help

install:
	sudo apt-get update
	sudo apt-get install -y gcc nasm make qemu-system-i386 genisoimage python3

# some variables to build the ISO.
FILESIZE = $(shell stat -c%s "profanOS.bin")
SECTORSIZE = 512
DIVIDED = $(shell echo $$(($(FILESIZE) / $(SECTORSIZE))))
FLOPPYSECS = 2876
FILLERSIZE = $(shell echo $$(($(FLOPPYSECS) - $(DIVIDED))))

iso: bin_image
	mkdir -p $(OUT_DIR)
	$(info Filesize: $(FILESIZE), No. of sectors: $(DIVIDED))
	dd if="/dev/zero" bs=$(SECTORSIZE) count=$(FILLERSIZE) of="$(OUT_DIR)/_filler.img"
	$(info Filler created.)
	cat profanOS.bin $(OUT_DIR)/_filler.img > $(OUT_DIR)/_floppy.img
	$(info Secondary image created.)
	touch $(OUT_DIR)/_floppy.catalog
	mkisofs -input-charset iso8859-1 -r -b "$(OUT_DIR)/_floppy.img" -c "$(OUT_DIR)/_floppy.catalog" -o "profanOS.iso" .

# create a 1MB hdd.bin file
hdd:
	dd if=/dev/zero of=HDD.bin bs=1024 count=1024

run: bin_image hdd
	qemu-system-i386 -drive file=profanOS.bin,if=floppy,format=raw -drive file=HDD.bin,format=raw -boot order=a

irun: iso hdd
	qemu-system-i386 -cdrom profanOS.iso -drive file=HDD.bin,format=raw -boot order=d 

clean:
	rm -Rf out/

fullclean: clean
	rm -Rf *.bin *.iso

# ld -m elf_i386 -G -o out/kernel.bin -Ttext 0x1000 out/kernel_entry.o out/shell.o out/interrupt.o out/switch.o out/task.o out/kernel.o out/screen.o out/keyboard.o out/ata.o out/isr.o out/ports.o out/idt.o out/timer.o out/mem.o out/string.o --oformat binary
# ld -m elf_i386 -G -o out/kernel.bin -Ttext 0x1000 out/kernel_entry.o out/kernel.o out/shell.o out/task.o out/switch.o out/keyboard.o out/screen.o out/ata.o out/idt.o out/isr.o out/ports.o out/timer.o out/interrupt.o out/mem.o out/string.o --oformat binary