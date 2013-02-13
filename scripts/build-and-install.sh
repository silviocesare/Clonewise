#!/bin/bash

rm -rf ~/Clonewise
cd ~
sudo apt-get install git openmpi-bin libopenmpi-dev g++ make libxerces-c-dev libxerces-c3.1 libfuzzy-dev weka
git clone https://github.com/silviocesare/Clonewise.git
cd ~/Clonewise
make
sudo make install
