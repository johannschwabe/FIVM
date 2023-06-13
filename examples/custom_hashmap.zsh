#!/bin/zsh

# Initialize associative arrays
typeset -A max_regular
typeset -A max_size

# Read CustomHashmapSizes.txt and fill the associative arrays
while IFS=',' read -r query regular size; do
  max_regular[$query]=$regular
  max_size[$query]=$size
done < CustomHashmapSizes.txt
if [[ $1 == *\*_unordered* ]]; then
    # Split at '*_unordered'
    first_part=${1%%\*_unordered*}
    second_part=${1##*\*_unordered}
elif [[ $1 == *\** ]]; then
    # Split at the asterisk
    first_part=${1%%\**}
    second_part=${1##*\*}
else
    # If there is no asterisk or '*_unordered', use the original string as the first part and '1' as the second part
    first_part=$1
    second_part=1
fi

query=$first_part
regular=$((${max_regular[$query]} * $second_part))
size=$((${max_size[$query]} * $second_part))
#echo "-DMAX_REGULAR=$regular"
#echo "-DMAX_SIZE=$size"
printf "-DMAXREGULAR=%s -DMAXSIZE=%s" "$regular" "$size"
