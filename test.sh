gcc -c zapps/sys/deluge.c -o out/zapps/sys/deluge.o -m32 -ffreestanding -Wall -Wextra -fno-exceptions -fno-stack-protector -march=i686 -Wno-unused -Werror -I include/zlibs
ld -m elf_i386 -T deluge.ld -o out/zapps/sys/deluge.pe out/make/zentry.o out/zapps/sys/deluge.o
objcopy -O binary out/zapps/sys/deluge.pe out/zapps/sys/deluge.bin -j .text -j .data -j .rodata -j .bss

gcc -shared -fPIC -nostdlib -nostartfiles -nodefaultlibs -o sys_dir/user/libtest.so sys_dir/user/lib.c -m32 -ffreestanding -Wall -Wextra -fno-exceptions -fno-stack-protector -march=i686 -Wno-unused -Werror -I include/zlibs
gcc -c sys_dir/user/test.c -o sys_dir/user/test.o -m32 -ffreestanding -Wall -Wextra -fno-exceptions -fno-stack-protector -march=i686 -Wno-unused -Werror -I include/zlibs
ld -m elf_i386 -T tools/zlink.ld -o sys_dir/user/test.elf sys_dir/user/test.o -L sys_dir/user -ltest
