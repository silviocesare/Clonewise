#!/bin/bash

rm -rf ~/Clonewise
cd ~
sudo apt-get install git openmpi-bin libopenmpi-dev g++ make libxerces-c-dev libxerces-c3.1 libfuzzy-dev weka dh-make
git clone https://github.com/silviocesare/Clonewise.git
VERSION="$(cat ~/Clonewise/VERSION)"
mv ~/Clonewise ~/Clonewise-$VERSION
tar czvf ~/Clonewise-$VERSION.tar.gz ~/Clonewise-$VERSION
cp -R ~/Clonewise-$VERSION ~/Clonewise-configs-$VERSION
mv ~/Clonewise-$VERSION ~/Clonewise-bins-$VERSION
cd ~/Clonewise-bins-$VERSION
dh_make -f ../$PACKAGENAME.tar.gz
mkdir debian/patches
cp ~/Clonewise-configs-$VERSION/patches/debian-install-bins.patch debian/patches
cd ~/Clonewise-configs-$VERSION
dh_make -f ../$PACKAGENAME.tar.gz
mkdir debian/patches
cp ~/Clonewise-configs-$VERSION/patches/debian-install-configs.patch debian/patches
