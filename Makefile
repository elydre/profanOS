.PHONY: info elf iso miso disk srcdisk run erun krun clean fullclean

PY_BUILD = tools/maketool.py

# list off available commands
info:
	python3 $(PY_BUILD) help

# build kernel
elf:
	python3 $(PY_BUILD) elf

# create iso with grub
iso:
	python3 $(PY_BUILD) iso

# create full iso with grub
miso:
	python3 $(PY_BUILD) miso

# build disk image
disk:
	python3 $(PY_BUILD) disk

# build disk image with source
srcdisk:
	python3 $(PY_BUILD) srcdisk

xtrdisk:
	python3 $(PY_BUILD) xtrdisk

# run kernel in qemu
run:
	python3 $(PY_BUILD) run

# run iso in qemu
erun:
	python3 $(PY_BUILD) erun

# run iso in qemu with kvm acceleration
krun:
	python3 $(PY_BUILD) krun

# clean out/ directory
clean:
	rm -Rf out/

# clean all build files
fullclean: clean
	rm -Rf *.iso *.elf *.bin
	rm -Rf extracted/
