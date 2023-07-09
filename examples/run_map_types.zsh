#!/bin/zsh

map_types=(2 3 4 5)
for i in $map_types ; do
    zsh run_all.zsh 5 all 1000 $i
done