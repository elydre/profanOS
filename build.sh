CCFLAGS="-m32 -ffreestanding -Wall -Wextra -fno-exceptions -fno-stack-protector -march=i686 -Wno-unused -I include/zlibs"
LDFLAGS="-m elf_i386 -T tools/zlink.ld"

PROJET="demo"

gcc $CCFLAGS -c test.c -o test.o
ld $LDFLAGS -o $PROJET.elf out/make/zentry.o test.o

objcopy -O binary $PROJET.elf $PROJET.bin -j .text -j .data -j .rodata -j .bss
rm *.o *.elf

cp demo.bin out/zapps/commands/
