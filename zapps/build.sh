build_prog() {
    echo "build $1..."
    gcc -g -ffreestanding -Wall -Wextra -fno-exceptions -m32 -c $1.c -o $1.o
    ld -m elf_i386 -e start -o $1.pe $1.o -Ttext 0x1000
    objcopy -O binary $1.pe $1.bin
    # delete null bytes at the end of the file
    sed '$ s/\x00*$//' $1.bin > $1.bin.tmp
    mv $1.bin.tmp $1.bin
}

build_prog togame

rm *.o *.pe
