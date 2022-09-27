# build kernel
elf:
	python3 maketool.py elf_image

# create iso with grub
iso:
	python3 maketool.py elf_image
	python3 maketool.py iso

# build disk image with zapps
disk:
	python3 maketool.py diskf

# list off available commands
info:
	python3 maketool.py help

# run kernel in qemu
run:
	python3 maketool.py run

# run iso in qemu
irun:
	python3 maketool.py irun

# clean out/ directory
clean:
	rm -Rf out/

# clean all build files
fullclean: clean
	rm -Rf *.iso *.elf *.bin
