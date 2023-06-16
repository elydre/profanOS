CCFLAGS="-m32 -ffreestanding -Wall -Wextra -fno-exceptions -fno-stack-protector -march=i686 -Wno-unused -I include/zlibs"
LINKER_FLAGS="-b elf32i386 -nostdlib -T tools/zlink.ld"

PROJET="demo"

tcc $CCFLAGS -c test.c -o test.o
tcc $CCFLAGS -c tools/zentry.c -o zentry.o
./vlink $LINKER_FLAGS -o $PROJET.elf zentry.o test.o

objcopy -O binary $PROJET.elf $PROJET.bin -j .text -j .data -j .rodata

cp demo.bin out/zapps/commands/
