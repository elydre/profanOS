# First rule is run by default
bin_image:
	python3 maketool.py bin_image

regular: bin_image
	mkdir -p out
	mv profanOS.img out/profanOS-irregular.img
	dd if=/dev/zero of=profanOS.img bs=1k count=1440
	dd if=out/profanOS-irregular.img of=profanOS.img conv=notrunc

info:
	@python3 maketool.py help

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
