.PHONY: elf iso disk disk-src run irun kirun info clean fullclean

PY_BUILD = build/maketool.py

# build kernel
elf:
	python3 $(PY_BUILD) elf_image

# create iso with grub
iso: elf
	python3 $(PY_BUILD) iso

# build disk image with zapps
disk:
	python3 $(PY_BUILD) diskf

disk-src:
	python3 $(PY_BUILD) disk_src

# list off available commands
info:
	python3 $(PY_BUILD) help

# run kernel in qemu
run: elf
	python3 $(PY_BUILD) run

# run iso in qemu
irun: elf
	python3 $(PY_BUILD) irun

# run iso in qemu kvm
kirun: elf
	python3 $(PY_BUILD) kirun

# clean out/ directory
clean:
	rm -Rf out/

# clean all build files
fullclean: clean
	rm -Rf *.iso *.elf *.bin
