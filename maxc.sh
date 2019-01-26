#!/bin/sh

./maxc $1 > a.s

outfile="a.out"

gcc a.s -o $outfile

if [ -e "a.s" ]; then
    rm a.s
fi
