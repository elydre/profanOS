#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~#
#   === Makefile : 2024 ===                                                   #
#                                                                             #
#    Makefile entry for profanOS                                   .pi0iq.    #
#                                                                 d"  . `'b   #
#    This file is part of profanOS and is released under          q. /|\  "   #
#    the terms of the GNU General Public License                   `// \\     #
#                                                                  //   \\    #
#   === elydre : https://github.com/elydre/profanOS ===         #######  \\   #
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~#

.PHONY: help elf iso miso disk bdisk run erun krun srun clean fclean addons gaddons

PY_BUILD = tools/maketool.py
PY_ADDON = tools/addons.py

# list off available commands
help:
	@python3 $(PY_BUILD) help

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
bdisk:
	python3 $(PY_BUILD) bdisk

# run kernel in qemu
run:
	python3 $(PY_BUILD) run

# run iso in qemu
erun:
	python3 $(PY_BUILD) erun

# run iso in qemu with kvm acceleration
krun:
	python3 $(PY_BUILD) krun

# run iso in qemu with audio
srun:
	python3 $(PY_BUILD) srun

# install all addons
addons:
	python3 $(PY_ADDON) -g

gaddons:
	python3 $(PY_ADDON) -a

# clean all build files
clean:
	rm -Rf out/ extracted/ include/addons/
	rm -Rf *.iso *.elf *.bin

# remove git ignored and discard all changes
fclean: clean
	@echo "This will remove all untracked files and discard all changes"
	@echo "Are you sure you want to continue? [y/N]"
	@read -r input; \
	if [ "$$input" = "y" ] || [ "$$input" = "Y" ]; then \
		git clean -fdx; \
		git reset --hard; \
	else \
		echo "Aborting"; \
	fi
