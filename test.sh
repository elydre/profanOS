gcc -c zapps/commands/deluge.c -o out/zapps/commands/deluge.o -m32 -ffreestanding -Wall -Wextra -fno-exceptions -fno-stack-protector -march=i686 -Wno-unused -Werror -I include/zlibs
ld -m elf_i386 -T deluge.ld -o out/zapps/commands/deluge.pe out/make/zentry.o out/zapps/commands/deluge.o
objcopy -O binary out/zapps/commands/deluge.pe out/zapps/commands/deluge.bin -j .text -j .data -j .rodata -j .bss
