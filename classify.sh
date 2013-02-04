#!/bin/bash
# Author:itxx00@gmail.com
tdir=step$1
mkdir $tdir
for ((i=1;i<=10;i++))
do
    dir=resi${i}"_step"$1
    mv $dir $tdir
done;
