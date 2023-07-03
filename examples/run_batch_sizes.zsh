#!/bin/zsh

batchSizes=(100 1000 10000 100000 1000000)
for i in $batchSizes ; do
    zsh run_all.zsh 5 all "$i"
done