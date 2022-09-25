build_prog() {
    echo "build $1..."
    gcc -g -ffreestanding -Wall -Wextra -fno-exceptions -m32 -c $1.c -o $1.o
    ld -m elf_i386 -e start -o $1.pe $1.o
    objcopy -O binary $1.pe $1.bin
}

build_prog togame

rm *.o *.pe
