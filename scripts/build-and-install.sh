#!/bin/bash

rm -rf ~/Clonewise
cd ~
apt-get install git openmpi-bin libopenmpi-dev g++ make libxerces-c-dev libxerces-c3.1 libfuzzy-dev weka
git clone http://github.com/siliocesare/Clonewise.git
cd ~/Clonewise
make
sudo make install
