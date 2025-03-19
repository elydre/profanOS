#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~#
#   === limine.sh : 2024 ===                                                  #
#                                                                             #
#    Script to create a bootable ISO with limine bootloader        .pi0iq.    #
#                                                                 d"  . `'b   #
#    This file is part of profanOS and is released under          q. /|\  "   #
#    the terms of the GNU General Public License                   `// \\     #
#                                                                  //   \\    #
#   === elydre : https://github.com/elydre/profanOS ===         #######  \\   #
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~#

LIMINE_REPO="https://github.com/limine-bootloader/limine"

LIMINE_CONF="\
timeout: 3

/profanOS
protocol: multiboot1
kernel_path: boot():/kernel.elf
module_path: boot():/initrd.bin
resolution: 1024x768x32

/profanOS text
protocol: multiboot1
kernel_path: boot():/kernel.elf
module_path: boot():/initrd.bin
textmode: yes
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

echo "BUILDING LIMINE -------------------------------"

mkdir -p out
rm -rf out/limine
git clone $LIMINE_REPO --depth=1 -b v8.x-binary out/limine/ > /dev/null

make -C out/limine > /dev/null

# copy files to iso directory
rm -rf out/isodir
mkdir out/isodir

cp kernel.elf out/isodir
cp initrd.bin out/isodir

cp out/limine/limine-bios-cd.bin out/isodir
cp out/limine/limine-bios.sys out/isodir
echo "$LIMINE_CONF" > out/isodir/limine.conf

echo
echo "CREATING profanOS.iso -------------------------"

xorriso -as mkisofs -b limine-bios-cd.bin -no-emul-boot out/isodir -o profanOS.iso

# check if profanOS.iso exists
if [ ! -f profanOS.iso ]; then
    echo "ERROR: failed to create profanOS.iso"
    exit 1
fi

echo "DEPLOYING LIMINE ------------------------------"

./out/limine/limine bios-install profanOS.iso

# check if limine was deployed successfully
if [ $? -ne 0 ]; then
    exit 1
fi

echo
echo "DONE ------------------------------------------"
echo "profanOS.iso ($(du -h profanOS.iso | cut -f1)B) created successfully"
