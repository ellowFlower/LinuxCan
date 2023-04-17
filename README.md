# LinuxCan
Minimal linux with CAN protocol support running inside a virtual machine (QEMU). Includes a CAN protocol demo user application. Host running Ubuntu 22.04.
Some details like installing qemu are omitted.

```
export BUILDS=~/mini-linux
mkdir -p $BUILDS
```

## Linux kernel
```
cd $BUILDS
# download kernel source (pick whatever version you like)
wget https://cdn.kernel.org/pub/linux/kernel/v6.x/linux-6.2.11.tar.xz
tar -xf linux-6.2.11.tar.xz
export LINUX_BUILD=$BUILDS/linux
# Copy the running kernel configuration, and apply the defaults for new settings added by the new kernel version
make olddefconfig
# adapt config, otherwise raises error about certificates 
scripts/config --set-str SYSTEM_TRUSTED_KEYS ""
scripts/config --set-str SYSTEM_REVOCATION_KEYS ""
make menuconfig # enable CAN support (CAN bus subsystem support)
# create bzImage
time make -j "$(nproc)" bzImage
export LINUX_IMAGE=<path to created bzImage>
```

## Busybox
```
cd ~/
# download busybox (version can differ)
wget https://busybox.net/downloads/busybox-1.36.0.tar.bz2
# unzip
tar -xjf busybox-1.36.0.tar.bz2
export BUSYBOX=~/busybox-1.36.0
export BUSYBOX_BUILD=$BUILDS/busybox
mkdir -p $BUSYBOX_BUILD
cd $BUSYBOX
make O=$BUSYBOX_BUILD defconfig
cd $BUSYBOX_BUILD
make menuconfig
# Busybox Settings ---> Build Options ---> Build BusyBox as a static binary (no shared libs) ---> yes
time make -j "$(nproc)"
make install
```

Create an initramfs:
```
export INITRAMFS_BUILD=$BUILDS/initramfs
mkdir -p $INITRAMFS_BUILD
cd $INITRAMFS_BUILD
mkdir -p bin sbin etc proc sys usr/bin usr/sbin
cp -a $BUSYBOX_BUILD/_install/* .
```
Add initialisation script `$INITRAMFS_BUILD/init`:
```
#!/bin/sh
/bin/mount -t devtmpfs devtmpfs /dev
/bin/mount -t proc none /proc
/bin/mount -t sysfs none /sys
exec 0</dev/console
exec 1>/dev/console
exec 2>/dev/console
cat <<!


Boot took $(cut -d' ' -f1 /proc/uptime) seconds

        _       _     __ _                  
  /\/\ (_)_ __ (_)   / /(_)_ __  _   ___  __
 /    \| | '_ \| |  / / | | '_ \| | | \ \/ /
/ /\/\ \ | | | | | / /__| | | | | |_| |>  < 
\/    \/_|_| |_|_| \____/_|_| |_|\__,_/_/\_\ 


Welcome to mini-linux


!
exec /bin/sh
```

Create the initramfs archive:
```
chmod +x init
find . -print0 | cpio --null -ov --format=newc \
  | gzip -9 > $BUILDS/initramfs.cpio.gz
```

## Toolchain
Create a toolchain with crosstool-NG:
```
cd ~/
git clone https://github.com/crosstool-ng/crosstool-ng
cd crosstool-ng
./bootstrap
./configure --prefix=~/
make
make install
export PATH="${PATH}:~/bin"
ct-ng list-samples
ct-ng show-x86_64-unknown-linux-gnu
ct-ng x86_64-unknown-linux-gnu
ct-ng build
# find toolchain in 
```

## Buildroot
```
cd ~/
# download (version can differ)
wget https://buildroot.org/downloads/buildroot-2023.02.tar.gz
# unzip
tar -xjf buildroot-2023.02.tar.gz
export BUILDROOT=~/buildroot-2023.02
export BUILDROOT_BUILD=$BUILDS/buildroot
mkdir -p $BUILDROOT_BUILD
cd $BUILDROOT_BUILD
touch Config.in external.mk
echo 'name: mini-linux' > external.desc
echo 'desc: minimal linux system with buildroot' >> external.desc
mkdir configs overlay
cd $BUILDROOT
make O=$BUILDROOT_BUILD BR2_EXTERNAL=$BUILDROOT_BUILD qemu_x86_64_defconfig
cd $BUILDROOT_BUILD
make menuconfig
```

Configure Buildroot according the following:
```
Build options ---> Location to save buildroot config ---> $(BR2_EXTERNAL)/configs/mini-linux_defconfig
Build options ---> Download dir ---> ~/buildroot_dl
Build options ---> Number of jobs to run simultaneously (0 for auto) ---> 8
Build options ---> Enable compiler cache ---> yes
Build options ---> Compiler cache location ---> ~/buildroot_ccache
Toolchain ---> Toolchain type ---> External toolchain
Toolchain ---> Toolchain ---> Custom toolchain
Toolchain ---> Toolchain origin ---> Pre-installed toolchain
Toolchain ---> Toolchain path ---> ~/x-tools/x86_64-unknown-linux-gnu
Toolchain ---> Toolchain prefix ---> x86_64-unknown-linux-gnu
Toolchain ---> External toolchain kernel headers series ---> 6.1.x or later
Toolchain ---> External toolchain C library ---> glibc
Toolchain ---> Toolchain has C++ support? ---> yes
Toolchain ---> Toolchain has RPC support ---> no
System configuration ---> System hostname ---> mini-linux
System configuration ---> System banner ---> Welcome to mini-linux
System configuration ---> Run a getty (login prompt) after boot ---> TTY port ---> ttyS0
System configuration ---> Root filesystem overlay directories ---> $(BR2_EXTERNAL)/overlay
Kernel ---> Linux Kernel ---> no
Filesystem images ---> cpio the root filesystem (for use as an initial RAM filesystem) ---> yes
Filesystem images ---> cpio the root filesystem (for use as an initial RAM filesystem) ---> Compression method ---> gzip
```

Save the configuration and build:
```
make savedefconfig
time make -j "$(nproc)"
```

## CAN demo user application
Create a new directory for the application:
```
export APPS=$BUILDS/apps
mkdir -p $APPS
cd $APPS
```
Add the files `can_demo.c  can_demo.h  canreceive.c  cantransmit.c  Makefile` to the directory `$APPS`.

Compile the application, copy it in the Buildroot overlay directory and build the root filesystem:
```
make
cp can_demo $BUILDROOT_BUILD/overlay
cd $BUILDROOT_BUILD
time make -j "$(nproc)"
```

## Run mini-linux
```

```
