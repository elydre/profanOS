run:
	gcc -c test1.c -o test1.o -g -ffreestanding -Wall -Wextra -fno-exceptions -m32 -fno-pie -I ./../../../zapps -I ./
	gcc -c test2.c -o test2.o -g -ffreestanding -Wall -Wextra -fno-exceptions -m32 -fno-pie -I ./../../../zapps -I ./
	ld -m elf_i386 -e main -shared -T link.ld ./test1.o ./test2.o -o test.o
	ld -m elf_i386 -e main -o test.pe test.o
	objcopy -O binary test.pe test.bin -j .text -j .data -j .rodata -j .bss
	make clean

fullclean:
	make clean
	rm -Rf ./*.bin

clean:
	rm -Rf ./*.full
	rm -Rf ./*.o
	rm -Rf ./*.pe