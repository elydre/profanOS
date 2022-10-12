run:
	gcc -c test2.c -o test2.o -g -ffreestanding -Wall -Wextra -fno-exceptions -m32 -fno-pie -I ./ -I ./../../
	gcc -c test.c -o test.o -g -ffreestanding -Wall -Wextra -fno-exceptions -m32 -fno-pie -I ./ -I ./../../
	ld -m elf_i386 -r test.o test2.o -o final.o
	ld -m elf_i386 -e main -o final.pe final.o
	objcopy -O binary final.pe final.full -j .text -j .data -j .rodata -j .bss
	sed 's/\x0//g' final.full > final.bin
	make clean

fullclean:
	make clean
	rm -Rf ./*.bin

clean:
	rm -Rf ./*.full
	rm -Rf ./*.o
	rm -Rf ./*.pe