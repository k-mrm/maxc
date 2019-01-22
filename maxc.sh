#!/bin/sh

./maxc $1 > a.s
gcc a.s
