build_prog() {
    echo "build $1.o..."
    gcc -g -ffreestanding -Wall -Wextra -fno-exceptions -m32 -c $1.c -o $1.o
    echo "build $1.pe..."
    ld -m elf_i386 -e start -o $1.pe $1.o -Ttext 0x1000
    echo "build $1.bin..."
    objcopy -O binary $1.pe $1.bin
    echo "delete null bytes..."
    sed '$ s/\x00*$//' $1.bin > $1.bin.tmp
    mv $1.bin.tmp $1.bin
    echo "the size of $1.bin is $(stat -c %s $1.bin) bytes"
}

build_prog togame

rm *.o *.pe
