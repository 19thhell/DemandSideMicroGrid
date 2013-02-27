#!/bin/bash
# Author:itxx00@gmail.com
if test $# -le 0
    then echo "Parameters unmatched!"
    exit 1
fi
#rm -rf step$1
for((i=1;i<=10;i++))
do
    dir=resi${i}"_step"$1
    rm -rf $dir
    mkdir $dir
done;
for((i=1;i<=10;i++))
do
    dir=indu${i}"_step"$1
    rm -rf $dir
    mkdir $dir
done;
for((i=1;i<=10;i++))
do
    dir=comm${i}"_step"$1
    rm -rf $dir
    mkdir $dir
done;
