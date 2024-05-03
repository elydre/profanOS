#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~#
#   === install.sh : 2024 ===                                                 #
#                                                                             #
#    Shell script to write an ISO to a disk                        .pi0iq.    #
#                                                                 d"  . `'b   #
#    This file is part of profanOS and is released under          q. /|\  "   #
#    the terms of the GNU General Public License                   `// \\     #
#                                                                  //   \\    #
#   === elydre : https://github.com/elydre/profanOS ===         #######  \\   #
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~#

echo "NOTE: do not use this script unknowingly"
echo "it may damage your computer and cause you"
echo "to lose data, use at your own risk"
echo ""

# get the number of arguments
# we need 2 arguments
if [ $# -ne 2 ]; then
    echo "USAGE: $0 <disk> <iso>"
    exit 1
fi

# get the arguments
DISK=$1
ISO=$2

# check if the disk exists
if [ ! -b $DISK ]; then
    echo "ERROR: Disk $DISK does not exist"
    exit 1
fi

# check if the iso exists
if [ ! -f $ISO ]; then
    echo "ERROR: ISO $ISO does not exist"
    exit 1
fi

# check if we are running as root
if [ "$(id -u)" != "0" ]; then
    echo "ERROR: This script must be run as root"
    exit 1
fi

# get info about the disk to give the user more info
DISK_MODEL=$(hdparm -I $DISK | grep "Model Number:" | cut -d ":" -f 2)

echo "This will erase all data on $DISK"
echo "Disk model: $DISK_MODEL"
echo ""

# get the user confirmation
echo "Are you sure you want to continue? (y/n)"
read answer
if [ "$answer" != "y" ]; then
    echo "Aborting"
    exit 1
fi

dd bs=16M if=$ISO of=$DISK status=progress oflag=sync && sync

if [ $? -eq 0 ]; then
    echo "Disk $DISK was successfully written"
else
    echo "ERROR: Something went wrong"
    exit 1
fi
