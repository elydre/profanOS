# First rule is run by default
bin_image:
	python3 maketool.py bin_image

info:
	@python3 maketool.py help

install:
	sudo apt-get update
	sudo apt-get install -y gcc nasm make qemu-system-i386 python3

hdd:
	@python3 maketool.py hddf

run: bin_image
	@python3 maketool.py hdd
	qemu-system-i386 -drive file=profanOS.img,if=floppy,format=raw -drive file=HDD.bin,format=raw -boot order=a

irun: iso
	@python3 maketool.py hdd
	qemu-system-i386 -cdrom profanOS.iso -drive file=HDD.bin,format=raw -boot order=d 

clean:
	rm -Rf out/

fullclean: clean
	rm -Rf *.bin *.iso *.img
