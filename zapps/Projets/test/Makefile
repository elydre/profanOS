run:
	gcc -g -ffreestanding -Wall -Wextra -fno-exceptions -m32 -c ./test.c -o ./test.o -I ./../../../zapps
	ld -m elf_i386 -e main -o ./test.pe ./test.o
	objcopy -O binary ./test.pe ./test.full -j .text -j .data -j .rodata -j .bss
	sed '$ s\x00*$/\\' ./test.full > ./test.bin
	make clean

fullclean:
	make clean
	rm -Rf ./*.bin

clean:
	rm -Rf ./*.full
	rm -Rf ./*.o
	rm -Rf ./*.pe