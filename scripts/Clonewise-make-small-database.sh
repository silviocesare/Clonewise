#!/bin/bash

if [ $# -ne 1 ]; then
	echo Usage: $0 dir
	exit 1
fi

BASE=$1
if [ ! -d $BASE ]; then
	echo $0: $BASE not a directory
	exit 1
fi
THIS=$1-small
mkdir $THIS
mkdir $THIS/depends
mkdir $THIS/info
mkdir $THIS/cache
mkdir $THIS/cache-embedded
mkdir $THIS/signatures
mkdir $THIS/downloads
mkdir $THIS/features
cp $BASE/embdedded-code-copies $THIS/
cp $BASE/embdedded-code-copies.txt $THIS/
cp /dev/null $THIS/packages
cat $BASE/embedded-code-copies|cut -d/ -f1|sort -u|while read p; do
	grep /$p$ $BASE/packages >> $THIS/packages
	grep /$p$ $BASE|cut -d/ -f1|while read q; do
		cp $BASE/depends/$q $THIS/depends/
	done
	cp $BASE/info/$p $THIS/info/$p
	cp $BASE/cache/$p $THIS/cache/
	cp $BASE/cache-embedded/$p $THIS/cache-embedded/
	cp $BASE/signatures/$p $THIS/signatures/
done
