PROFANDIR = ../../..

CCFLAGS = -m32 -ffreestanding -Wall -Wextra -fno-exceptions -fno-stack-protector -march=i686 -Wno-unused -I $(PROFANDIR)/include/zlibs
LDFLAGS = -m elf_i386 -T $(PROFANDIR)/tools/zlink.ld

PROJET = demo

build:
	gcc $(CCFLAGS) -c file1.c -o file1.o
	gcc $(CCFLAGS) -c file2.c -o file2.o

	ld $(LDFLAGS) -o $(PROJET).elf $(PROFANDIR)/out/make/zentry.o file1.o file2.o
	objcopy -O binary $(PROJET).elf $(PROJET).bin -j .text -j .data -j .rodata -j .bss
	rm *.o *.elf

fullclean:
	rm *.bin *.o *.elf
