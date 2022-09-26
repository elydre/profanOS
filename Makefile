# First rule is run by default
elf_image:
	python3 maketool.py elf_image

iso:
	python3 maketool.py elf_image
	python3 maketool.py iso


disk:
	python3 maketool.py diskf

info:
	python3 maketool.py help


run:
	python3 maketool.py run

irun:
	python3 maketool.py irun


clean:
	rm -Rf out/

fullclean: clean
	rm -Rf *.iso *.elf *.bin
