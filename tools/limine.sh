LIMINE_REPO="https://github.com/limine-bootloader/limine"

LIMINE_CFG="\
TIMEOUT=3

:profanOS

PROTOCOL=multiboot1

CMDLINE=root=/ emergency=yes
KERNEL_PATH=boot:///kernel.elf
MODULE_PATH=boot:///initrd.bin

:profanOS no-rd

PROTOCOL=multiboot1

CMDLINE=root=/ emergency=yes
KERNEL_PATH=boot:///kernel.elf
"

# check if kernel.elf and initrd.bin exist
if [ ! -f kernel.elf ]; then
    echo "ERROR: kernel.elf required!"
    exit 1
fi

if [ ! -f initrd.bin ]; then
    echo "ERROR: initrd.bin required!"
    exit 1
fi

# check if out/ directory exists
if [ ! -d out ]; then
    echo "| out/ directory not found, creating"
    mkdir out
fi

# check if limine directory exists in out/
if [ ! -d out/limine ]; then
    echo "| limine directory not found, cloning"
    git clone $LIMINE_REPO --depth=1 -b v3.0-binary out/limine/ > /dev/null
else
    echo "| limine directory found, updating"
    git -C out/limine pull > /dev/null
fi

# check if limine-deploy exists
if [ ! -f out/limine/limine-install ]; then
    echo "| limine-install not found, building"
    make -C out/limine > /dev/null
fi

# check if out/isodir already exists
if [ -d out/isodir ]; then
    echo "| out/isodir already exists, removing contents"
    rm -rf out/isodir/*
else
    echo "| out/isodir not found, creating"
    mkdir out/isodir
fi

# copy kernel.elf and initrd.bin to out/isodir
echo "| copying kernel.elf and initrd.bin to out/isodir"
cp kernel.elf out/isodir
cp initrd.bin out/isodir

# copy limine.sys limine-cd.bin to out/isodir
echo "| copying limine.sys and limine-cd.bin to out/isodir"
cp out/limine/limine.sys out/isodir
cp out/limine/limine-cd.bin out/isodir

# create limine.cfg in out/isodir
echo "| creating limine.cfg in out/isodir"
echo "$LIMINE_CFG" > out/isodir/limine.cfg

# check if profanOS.iso already exists
if [ -f profanOS.iso ]; then
    echo "| profanOS.iso already exists, removing"
    rm profanOS.iso
fi

# create profanOS.iso
echo "| creating profanOS.iso"
xorriso -as mkisofs -b limine-cd.bin -no-emul-boot -boot-load-size 4 \
        -boot-info-table --protective-msdos-label out/isodir -o profanOS.iso > /dev/null 2>&1

# check if profanOS.iso exists
if [ ! -f profanOS.iso ]; then
    echo "ERROR: failed to create profanOS.iso"
    exit 1
else
    echo "| profanOS.iso created successfully"
fi

# deploy limine to profanOS.iso
echo "| deploying limine to profanOS.iso"
./out/limine/limine-deploy profanOS.iso > /dev/null 2>&1

# check if limine was deployed successfully
if [ $? -ne 0 ]; then
    echo "ERROR: failed to deploy limine to profanOS.iso"
    exit 1
else
    echo "| limine deployed successfully"
fi

echo "profanOS.iso ($(du -h profanOS.iso | cut -f1)B) created successfully"
