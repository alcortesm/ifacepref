#!/bin/sh

# this script generates:
#  - client.rootfs: the client disk image of dhcplab
#  - server.rootfs: the server disk image of dhcplab
#  - run-dhcplab: a script to launch dhcplab

# A NOTE ABOUT DEBOOTSTRAPING FROM A LOCAL REPOSITORY
#
# Running this script means downloading several hundreds MB from
# the Debian repository. This may be time-consumeing or even
# expensive depending on your internet connection.
# 
# You can create and use a local repository instead using
# apt-move:
#
# # create a temp dir for the debs you need
# DEBS_DIR=/tmp/debs
# mkdir $DEBS_DIR
# 
# # download the debs you need
# INCLUDE=ssh,less,valgrind,udev,tshark,tcpdump,udhcpd,udhcpc,psmisc
# DISTRO=lenny
# /usr/sbin/debootstrap --download-only --include=$INCLUDE $DISTRO $DEBS_DIR http://ftp.debian.org/debian
#
# # build an ad-hoc apt-move.conf file
# echo "#  Configuration file for the apt-move script.
# # See the apt-move(8) manpage for information about these settings.
# APTSITES="/all/"
# LOCALDIR=/mirrors/debian
# DIST=${DISTRO}
# PKGTYPE=binary
# FILECACHE=${DEBS_DIR}/var/cache/apt/archives
# LISTSTATE=${DEBS_DIR}/var/lib/apt/lists
# DELETE=no
# MAXDELETE=20
# COPYONLY=yes
# PKGCOMP=gzip
# CONTENTS=yes
# GPGKEY=" > ${DEBS_DIR}/apt-move.conf
# 
# # create a system-wide repo dir
# sudo mkdir -p /mirrors/debian
# 
# # Update the local index 
# sudo apt-move -c ${DEBS_DIR}/apt-move.conf get
#
# # create the local repo
# sudo apt-move -c ${DEBS_DIR}/apt-move.conf move
#
# # create the local package & release files
# sudo apt-move -c ${DEBS_DIR}/apt-move.conf packages
#
# # the new local repo is at /mirrors/debian



# CONFIGURE THIS

debianVersion=lenny
rootfsSize=450 # in MB
timezone=Europe/Madrid
umlPath=~/local/linux-src/linux-2.6.29/linux
umlModulesPath=~/local/linux-src/linux-2.6.29/modules/lib/modules
rootfs_install_dir=~/local/uml-rootfs/ifacepref
launchscript_file=~/bin/uml-ifacepref
mkdir -p ${rootfs_install_dir}
[ ! -d ${rootfs_install_dir} ] && echo "${rootfs_install_dir}: dir not found" && exit 1
[ ! -r ${rootfs_install_dir} ] && echo "${rootfs_install_dir}: read permission denied" && exit 1
[ ! -w ${rootfs_install_dir} ] && echo "${rootfs_install_dir}: write permission denied" && exit 1
[ ! -x ${rootfs_install_dir} ] && echo "${rootfs_install_dir}: exec permission denied" && exit 1

# parse first and only argument to choose between network download
# or local download of packages
if [ $# == 0 ]
then
    debianFTP=http://ftp.debian.org/debian
elif [ $# == 1 ]
    then
    if [ $1 == "local" ]
    then
        debianFTP=file:///mirrors/debian
    else
        echo "$1: Bad argument" >/dev/stderr && exit 1
    fi
else
    echo "Bad number of arguments" >/dev/stderr && exit 1
fi
    


# DON'T EDIT BELLOW THIS LINE IF YOU DON'T
# KNOW WHAT YOU ARE DOING

# First, generate de run script
echo '#!/bin/sh

BASE=~/local/uml-rootfs/ifacepref
TMP=${BASE}/tmp

HUB1=${TMP}/hub1.ctl
HUB2=${TMP}/hub2.ctl
SCOW=${TMP}/server.cow
CCOW=${TMP}/client.cow
SERVER=${BASE}/server.rootfs
CLIENT=${BASE}/client.rootfs

TERM=/usr/bin/xterm
XTERM=/usr/bin/xterm
SWITCH=/usr/bin/uml_switch
UML='${umlPath}'

# check base & tmp dir
[ ! -d ${BASE} ] && echo "${BASE}: dir not found" && exit 1
[ ! -r ${BASE} ] && echo "${BASE}: read permission denied" && exit 1
[ ! -w ${BASE} ] && echo "${BASE}: write permission denied" && exit 1
[ ! -x ${BASE} ] && echo "${BASE}: exec permission denied" && exit 1
mkdir -p ${TMP}
[ ! -d ${TMP} ] && echo "${TMP}: dir not found" && exit 1
[ ! -r ${TMP} ] && echo "${TMP}: read permission denied" && exit 1
[ ! -w ${TMP} ] && echo "${TMP}: write permission denied" && exit 1
[ ! -x ${TMP} ] && echo "${TMP}: exec permission denied" && exit 1

# remove leftover from previous incantations
[ -f ${HUB1} ] && [ ! -w ${HUB1} ] && echo "${HUB1}: write permission denied" && exit 1
[ -f ${HUB1} ] && rm -f ${HUB1}
[ -f ${HUB2} ] && [ ! -w ${HUB2} ] && echo "${HUB2}: write permission denied" && exit 1
[ -f ${HUB2} ] && rm -f ${HUB2}
[ -f ${SCOW} ] && [ ! -w ${SCOW} ] && echo "${SCOW}: write permission denied" && exit 1
[ -f ${SCOW} ] && rm -f ${SCOW}
[ -f ${CCOW} ] && [ ! -w ${CCOW} ] && echo "${CCOW}: write permission denied" && exit 1
[ -f ${CCOW} ] && rm -f ${CCOW}

# check root file systems
[ ! -f ${SERVER} ] && echo "${SERVER}: file not found" && exit 1
[ ! -r ${SERVER} ] && echo "${SERVER}: read permission denied" && exit 1
[ ! -f ${CLIENT} ] && echo "${CLIENT}: file not found" && exit 1
[ ! -r ${CLIENT} ] && echo "${CLIENT}: read permission denied" && exit 1

# check commands
[ ! -f ${XTERM} ] && echo "${XTERM}: command not found" && exit 1
[ ! -x ${XTERM} ] && echo "${XTERM}: exec permission denied" && exit 1
[ ! -f ${TERM} ] && echo "${TERM}: command not found" && exit 1
[ ! -x ${TERM} ] && echo "${TERM}: exec permission denied" && exit 1
[ ! -f ${SWITCH} ] && echo "${SWITCH}: command not found" && exit 1
[ ! -x ${SWITCH} ] && echo "${SWITCH}: exec permission denied" && exit 1
[ ! -f ${UML} ] && echo "${UML}: command not found" && exit 1
[ ! -x ${UML} ] && echo "${UML}: exec permission denied" && exit 1

# echo the commands we are going to run
echo "${TERM} -bg black -fg white -e ${SWITCH} -hub -unix ${HUB1} &"
echo "${TERM} -bg black -fg white -e ${SWITCH} -hub -unix ${HUB2} &"
echo "${TERM} -bg black -fg white -e ${UML} mem=64M ubd0=${SCOW},${SERVER} eth0=daemon,fe:fd:00:00:02:00,unix,${HUB1} eth1=daemon,fe:fd:00:00:02:01,unix,${HUB2} eth2=daemon,fe:fd:00:00:02:02,unix,${HUB2} eth3=daemon,fe:fd:00:00:02:03,unix,${HUB2} eth4=daemon,fe:fd:00:00:02:04,unix,${HUB2} con=pty con0=fd:0,fd:1 con1=xterm xterm=${XTERM},-T,-e umid=server &"
echo "${TERM} -bg black -fg white -e ${UML} mem=64M ubd0=${CCOW},${CLIENT} eth0=daemon,fe:fd:00:00:03:00,unix,${HUB1} eth1=daemon,fe:fd:00:00:02:01,unix,${HUB2} eth2=daemon,fe:fd:00:00:02:02,unix,${HUB2} eth3=daemon,fe:fd:00:00:02:03,unix,${HUB2} eth4=daemon,fe:fd:00:00:02:04,unix,${HUB2} con=pty con0=fd:0,fd:1 con1=xterm xterm=${XTERM},-T,-e umid=client &"

# run the virtual network and hosts
${TERM} -bg black -fg white -e ${SWITCH} -hub -unix ${HUB1} &
${TERM} -bg black -fg white -e ${SWITCH} -hub -unix ${HUB2} &
sleep 1
${TERM} -bg black -fg white -e ${UML} mem=64M ubd0=${SCOW},${SERVER} eth0=daemon,fe:fd:00:00:02:00,unix,${HUB1} eth1=daemon,fe:fd:00:00:02:01,unix,${HUB2} eth2=daemon,fe:fd:00:00:02:02,unix,${HUB2} eth3=daemon,fe:fd:00:00:02:03,unix,${HUB2} eth4=daemon,fe:fd:00:00:02:04,unix,${HUB2} con=pty con0=fd:0,fd:1 con1=xterm xterm=${XTERM},-T,-e umid=server &
${TERM} -bg black -fg white -e ${UML} mem=64M ubd0=${CCOW},${CLIENT} eth0=daemon,fe:fd:00:00:03:00,unix,${HUB1} eth1=daemon,fe:fd:00:00:03:01,unix,${HUB2} eth2=daemon,fe:fd:00:00:03:02,unix,${HUB2} eth3=daemon,fe:fd:00:00:03:03,unix,${HUB2} eth4=daemon,fe:fd:00:00:03:04,unix,${HUB2} con=pty con0=fd:0,fd:1 con1=xterm xterm=${XTERM},-T,-e umid=client &

' > ${launchscript_file}
chmod u+x ${launchscript_file}

echo "########## ${launchscript_file} script succesfully generated"









# Generate the client image (./client.rootfs)

name=client
packages=ssh,less,valgrind,udev,tshark,tcpdump,psmisc
numEths=4 # Number of ethernet cards on the client, not counting eth0

image=${rootfs_install_dir}/$name.rootfs

echo "########## Refreshing sudo passwd"
sudo touch /tmp/bla
sudo rm /tmp/bla

## umount the image from previous invocations
mount 2>/dev/null | grep -e "^$image .*$" >/dev/null
if [ $? -eq 0 ]
then
    sudo umount $image
fi

## remove the image from previous invocations
[ -f $image ] && [ ! -w $image ] && echo "$image: write permission denied" && exit 1
[ -f $image ] && [ -w $image ] && rm $image

echo "########## Creating the $name image"
dd if=/dev/zero of=$image bs=1M count=$rootfsSize
sudo mkfs.ext3 -F $image

mountPoint=/mnt/$name
if [ ! -e $mountPoint ]
then
    echo "########## Making mount point $mountPoint"
    sudo mkdir -p $mountPoint
fi
echo "########## Mounting $image"
sudo mount -o loop $image $mountPoint

echo "########## Installing base system"
sudo debootstrap --include=$packages $debianVersion $mountPoint $debianFTP

echo "########## Configuring base system"
# hostname
sudo bash -c "echo $name > $mountPoint/etc/hostname"

# timezone
sudo bash -c "echo $timezone > $mountPoint/etc/timezone"
sudo rm $mountPoint/etc/localtime
sudo ln -sf /usr/share/zoneinfo/$timezone $mountPoint/etc/localtime

# /etc/udev/rules.d/z25_persistent-net.rules
sudo bash -c "echo \"# This file was automatically generated by the /lib/udev/write_net_rules
# program, probably run by the persistent-net-generator.rules rules file.
#
# You can modify it, as long as you keep each rule on a single line.
# MAC addresses must be written in lowercase.
\" > $mountPoint/etc/udev/rules.d/z25_persistent-net.rules"

for i in `seq 0 $numEths`
do
    sudo bash -c "echo SUBSYSTEM=='\"'net'\"', DRIVERS=='\"'?*'\"', ATTRS{address}=='\"'fe:fd:00:00:03:0$i'\"', NAME='\"'eth$i'\"' >> $mountPoint/etc/udev/rules.d/z25_persistent-net.rules"
    sudo bash -c "echo "" >> $mountPoint/etc/udev/rules.d/z25_persistent-net.rules"
done

# /etc/network/interfaces
sudo bash -c "echo \"# Used by ifup(8) and ifdown(8). See the interfaces(5) manpage or
# /usr/share/doc/ifupdown/examples for more information.

auto lo
iface lo inet loopback

auto eth0
iface eth0 inet static
        address 192.168.1.20
        netmask 255.255.255.0
        broadcast 192.168.100.255

auto eth1
iface eth1 inet static
        address 192.168.2.21
        netmask 255.255.255.0
        broadcast 192.168.101.255

auto eth2
iface eth2 inet static
        address 192.168.2.22
        netmask 255.255.255.0
        broadcast 192.168.102.255

auto eth3
iface eth3 inet static
        address 192.168.2.23
        netmask 255.255.255.0
        broadcast 192.168.103.255
        \" > $mountPoint/etc/network/interfaces"


# /etc/fstab
sudo mkdir $mountPoint/mnt/host-tmp

sudo bash -c "echo \"# PRECONFIGURED FSTAB

/dev/ubda   /               ext3    defaults        0       1
none        /proc           proc    defaults        0       0
none        /dev/shm        tmpfs   defaults        0       0
none        /mnt/host-tmp   hostfs  defaults,/tmp   0       0
\" > $mountPoint/etc/fstab"

# /etc/hosts
sudo bash -c "echo \"127.0.0.1    localhost
192.168.1.20    client
192.168.1.10    server\" > $mountPoint/etc/hosts"

# /root/bashrc
sudo bash -c "echo PATH=\\\$PATH:/mnt/host-tmp >> $mountPoint/root/.bashrc"

# Root password
#sudo chroot $mountPoint passwd

# generate public key pair
sudo chroot $mountPoint ssh-keygen -q -N '' -f /root/.ssh/id_rsa
sudo cp $mountPoint/root/.ssh/id_rsa.pub id_rsa.pub

# Copying kernel modules
sudo cp -Rp $umlModulesPath/* $mountPoint/lib/modules

# Done with the base system, unmounting
sudo umount $image
sudo rmdir $mountPoint

echo "########## $name image succesfully generated"




















# Now, generate de server image

name=server
packages=ssh,less,udev,tshark,tcpdump,psmisc
numEths=4 # Number of ethernet cards on the server, not counting eth0

image=${rootfs_install_dir}/$name.rootfs

## umount the image from previous invocations
mount 2>/dev/null | grep -e "^$image .*$" >/dev/null
if [ $? -eq 0 ]
then
    sudo umount $image
fi

## remove the image from previous invocations
[ -f $image ] && [ ! -w $image ] && echo "$image: write permission denied" && exit 1
[ -f $image ] && [ -w $image ] && rm $image

dd if=/dev/zero of=$image bs=1M count=$rootfsSize
sudo mkfs.ext3 -F $image

mountPoint=/mnt/$name
if [ ! -e $mountPoint ]
then
    echo Making mount point $mountPoint
    sudo mkdir $mountPoint
fi
echo Mounting server.rootfs
sudo mount -o loop $image $mountPoint

echo Installing base system.
sudo debootstrap --include=$packages $debianVersion $mountPoint $debianFTP

echo Configuring base system.
# hostname
sudo bash -c "echo $name > $mountPoint/etc/hostname"

# timezone
sudo bash -c "echo $timezone > $mountPoint/etc/timezone"
sudo rm $mountPoint/etc/localtime
sudo ln -sf /usr/share/zoneinfo/$timezone $mountPoint/etc/localtime

# /etc/udev/rules.d/z25_persistent-net.rules
sudo bash -c "echo \"# This file was automatically generated by the /lib/udev/write_net_rules
# program, probably run by the persistent-net-generator.rules rules file.
#
# You can modify it, as long as you keep each rule on a single line.
# MAC addresses must be written in lowercase.
\" > $mountPoint/etc/udev/rules.d/z25_persistent-net.rules"

for i in `seq 0 $numEths`
do
    sudo bash -c "echo SUBSYSTEM=='\"'net'\"', DRIVERS=='\"'?*'\"', ATTRS{address}=='\"'fe:fd:00:00:02:0$i'\"', NAME='\"'eth$i'\"' >> $mountPoint/etc/udev/rules.d/z25_persistent-net.rules"
    sudo bash -c "echo "" >> $mountPoint/etc/udev/rules.d/z25_persistent-net.rules"
done

# /etc/network/interfaces
address=10

sudo bash -c "echo \"# Used by ifup(8) and ifdown(8). See the interfaces(5) manpage or
# /usr/share/doc/ifupdown/examples for more information.

auto lo
iface lo inet loopback

auto eth0
iface eth0 inet static
        address 192.168.1.10
        netmask 255.255.255.0
        broadcast 192.168.100.255

auto eth1
iface eth1 inet static
        address 192.168.2.11
        netmask 255.255.255.0
        broadcast 192.168.100.255

auto eth2
iface eth2 inet static
        address 192.168.2.12
        netmask 255.255.255.0
        broadcast 192.168.100.255

auto eth3
iface eth3 inet static
        address 192.168.2.13
        netmask 255.255.255.0
        broadcast 192.168.100.255
        \" > $mountPoint/etc/network/interfaces"

# /etc/fstab
sudo mkdir $mountPoint/mnt/host

sudo bash -c "echo \"# PRECONFIGURED FSTAB

/dev/ubda   /               ext3    defaults        0       1
none        /proc           proc    defaults        0       0
none        /dev/shm        tmpfs   defaults        0       0
\" > $mountPoint/etc/fstab"

# /etc/hosts
sudo bash -c "echo \"127.0.0.1    localhost

192.168.1.20    client
192.168.2.21    client1
192.168.2.22    client2
192.168.2.23    client3

192.168.1.10    server
192.168.2.11    server1
192.168.2.12    server2
192.168.2.13    server3
\" > $mountPoint/etc/hosts"

# Root password
#echo Setting \"root\" as the root password for $name 
#sudo chroot $mountPoint passwd

echo Now generating the server public key pair
sudo chroot $mountPoint mkdir /root/.ssh
sudo cp id_rsa.pub $mountPoint/root/.ssh/authorized_keys
sudo rm -f id_rsa.pub

echo "########## Storing server host rsa pub key on server.rsa.pub"
cat $mountPoint/etc/ssh/ssh_host_rsa_key.pub | cut -d' ' -f2 > server.rsa.pub


# Copying kernel modules
sudo cp -Rp $umlModulesPath/* $mountPoint/lib/modules

# Done with the base system, unmounting
sudo umount $image
sudo rmdir $mountPoint







echo "########## Adding server host rsa pub key to client known hosts"
name=client
image=${rootfs_install_dir}/$name.rootfs
mountPoint=/mnt/$name
if [ ! -e $mountPoint ]
then
    echo "########## Making mount point $mountPoint"
    sudo mkdir $mountPoint
fi
echo "########## Mounting $image"
sudo mount -o loop $image $mountPoint

echo "########## recovering the server rsa pub key"
pubkey=`cat server.rsa.pub`
sudo bash -c "echo \"server,192.168.100.10 ssh-rsa $pubkey
\" > $mountPoint/root/.ssh/known_hosts"
sudo chroot $mountPoint ssh-keygen -H
sudo chroot $mountPoint rm /root/.ssh/known_hosts.old
sudo rm -f server.rsa.pub
echo "########## known_host updated"

# Done with the base system, unmounting
sudo umount $image
sudo rmdir $mountPoint

