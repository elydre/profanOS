.PHONY: elf iso disk disk-src run irun kirun info clean fullclean

# build kernel
elf:
	python3 maketool.py elf_image

# create iso with grub
iso: elf
	python3 maketool.py iso

# build disk image with zapps
disk:
	python3 maketool.py diskf

disk-src:
	python3 maketool.py disk_src

# list off available commands
info:
	python3 maketool.py help

# run kernel in qemu
run: elf
	python3 maketool.py run

# run iso in qemu
irun: elf
	python3 maketool.py irun

# run iso in qemu kvm
kirun: elf
	python3 maketool.py kirun

# clean out/ directory
clean:
	rm -Rf out/

# clean all build files
fullclean: clean
	rm -Rf *.iso *.elf *.bin
