#!/bin/sh
export PROFAN_TOOLS="$(realpath ./bin_tools)"

set -e

mkdir -p $PROFAN_TOOLS

# xorriso

if [ ! -f $PROFAN_TOOLS/bin/xorriso ]; then
    mkdir -p tmp
    cd tmp
    wget -4 https://ftp.wayne.edu/gnu/xorriso/xorriso-1.5.4.tar.gz
    tar xvf xorriso-1.5.4.tar.gz
    cd xorriso-1.5.4
    
    ./configure --prefix=$PROFAN_TOOLS
    make -j$(nproc)
    make install
    cd ../..
    rm -rf tmp
fi

# grub

if [ ! -f $PROFAN_TOOLS/bin/grub-mkrescue ]; then
    mkdir -p tmp
    cd tmp
    wget -4 https://ftp.wayne.edu/gnu/grub/grub-2.06.tar.gz
    tar xvf grub-2.06.tar.gz
    cd grub-2.06
    
    #./configure --prefix=$PROFAN_TOOLS
    find . -name "Makefile" -exec sed -i 's/-Werror//g' {} +
    CFLAGS="-std=gnu11 -Wno-error -Wno-error=dangling-pointer -Wno-unterminated-string-initialization -Wno-array-bounds" ./configure --prefix=$PROFAN_TOOLS
    make -j$(nproc)
    make install
    cd ../..
    rm -rf tmp
fi

cat > make_cross << END
#!/bin/sh
export PATH="${PROFAN_TOOLS}"':'"${PATH}"
make $@
END

chmod +x make_cross
