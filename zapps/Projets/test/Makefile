run:
	gcc -c test1.c -o test1.o -g -ffreestanding -Wno-unused -Wall -Wextra -fno-exceptions -m32 -I ./../..
	gcc -c test2.c -o test2.o -g -ffreestanding -Wno-unused -Wall -Wextra -fno-exceptions -m32 -I ./../..
	ld -m elf_i386 -e main -o test.pe ./test1.o ./test2.o
	objcopy -O binary test.pe test.bin -j .text -j .data -j .rodata -j .bss
	make clean

fullclean:
	make clean
	rm -Rf ./*.bin

clean:
	rm -Rf ./*.full
	rm -Rf ./*.o
	rm -Rf ./*.pe