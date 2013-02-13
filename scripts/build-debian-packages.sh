#!/bin/bash

DEBEMAIL="silvio.cesare@gmail.com"
DEBFULLNAME="Silvio Cesare"
export DEBEMAIL DEBFULLNAME
rm -rf ~/Clonewise
cd ~
sudo apt-get -y install git openmpi-bin libopenmpi-dev g++ make libxerces-c-dev libxerces-c3.1 libfuzzy-dev weka dh-make
git clone https://github.com/silviocesare/Clonewise.git
VERSION="$(cat ~/Clonewise/VERSION)"
mv ~/Clonewise ~/Clonewise-$VERSION
tar czvf ~/Clonewise-$VERSION.tar.gz ~/Clonewise-$VERSION
cp -R ~/Clonewise-$VERSION ~/clonewise-configs-$VERSION
cp -R ~/Clonewise-$VERSION ~/clonewise-bins-$VERSION
cd ~/clonewise-bins-$VERSION
dh_make -s -f ../Clonewise-${VERSION}.tar.gz --copyright gpl3
mkdir debian/patches
cp ~/clonewise-configs-${VERSION}/patches/debian-install-bins.patch debian/patches
cd ~/clonewise-configs-$VERSION
dh_make -i -f ../Clonewise-${VERSION}.tar.gz --copyright gpl3
mkdir debian/patches
cp ~/clonewise-configs-${VERSION}/patches/debian-install-configs.patch debian/patches
echo config/clones/weka/model >> debian/source/include-binaries
echo config/clones/weka/model2 >> debian/source/include-binaries
