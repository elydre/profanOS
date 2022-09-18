# First rule is run by default
elf_image:
	python3 maketool.py elf_image

iso: elf_image
	@python3 maketool.py iso


hdd:
	@python3 maketool.py hddf

info:
	@python3 maketool.py help


run: elf_image
	@python3 maketool.py hdd
	qemu-system-i386 -kernel profanOS.elf -drive file=HDD.bin,format=raw -boot order=a

irun: iso
	@python3 maketool.py hdd
	qemu-system-i386 -cdrom profanOS.iso -drive file=HDD.bin,format=raw -boot order=d


clean:
	rm -Rf out/ isodir/

fullclean: clean
	rm -Rf *.iso *.elf *.bin
