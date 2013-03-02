#!/bin/bash
# Author:itxx00@gmail.com
if test $# -le 0
    then echo "Parameters unmatched!"
    exit 1
fi
tdir1=resistagestep$1
mkdir $tdir1
for ((i=1;i<=10;i++))
do
    dir=resi${i}"_step"$1
    mv $dir $tdir1
done;
tdir2=industagestep$1
mkdir $tdir2
for ((i=1;i<=10;i++))
do
    dir=indu${i}"_step"$1
    mv $dir $tdir2
done;
tdir3=commstagestep$1
mkdir $tdir3
for ((i=1;i<=10;i++))
do
    dir=comm${i}"_step"$1
    mv $dir $tdir3
done;
