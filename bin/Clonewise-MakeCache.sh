#!/bin/bash

if [ $# -ne 1 ]; then
	echo "Usage: $0 distro" > /dev/stderr
	exit 1
fi

if [ $(id -u) != "0" ]; then
	echo "Need to be superuser to make cache" > /dev/stderr
	exit 1
fi

packages=$(cat /var/lib/Clonewise/distros/$1/packages|cut -d/ -f2|sort -u)
for p in $packages; do
	Clonewise $p > /var/lib/Clonewise/distros/$1/cache/$p
done
touch /var/lib/Clonewise/distros/$1/cache/.done
