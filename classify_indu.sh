#!/bin/bash
# Author:itxx00@gmail.com
if test $# -le 0
    then echo "Parameters unmatched!"
    exit 1
fi
tdir=originindustage2step$1
mkdir $tdir
for ((i=1;i<=10;i++))
do
    dir=indu${i}"_step"$1
    mv $dir $tdir
done;
