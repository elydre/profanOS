gcc -c zapps/commands/test.c -o out/zapps/commands/test.o -m32 -ffreestanding -Wall -Wextra -fno-exceptions -fno-stack-protector -march=i686 -Wno-unused -Werror -I include/zlibs
ld -m elf_i386 -T test.ld -o out/zapps/commands/test.pe out/make/zentry.o out/zapps/commands/test.o
objcopy -O binary out/zapps/commands/test.pe out/zapps/commands/test.bin -j .text -j .data -j .rodata -j .bss
