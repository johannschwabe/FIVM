#!/bin/zsh

# Initialize associative arrays
typeset -A max_regular
typeset -A max_size

# Read CustomHashmapSizes.txt and fill the associative arrays
while IFS=',' read -r query regular size; do
  max_regular[$query]=$regular
  max_size[$query]=$size
done < CustomHashmapSizes.txt
# Retrieve the macros for the provided query
query=$1
regular=${max_regular[$query]}
size=${max_size[$query]}
#echo "-DMAX_REGULAR=$regular"
#echo "-DMAX_SIZE=$size"
printf "-DMAXREGULAR=%s -DMAXSIZE=%s" "$regular" "$size"
