#!/bin/bash

# Function to find the next available file name
get_next_filename() {
  local file="config"
  local suffix=1

  while [[ -e "${file}_${suffix}.txt" ]]; do
    ((suffix++))
  done

  echo "${file}_${suffix}.txt"
}

# Check if config.txt exists
if [[ -e "config.txt" ]]; then
  # Find the next available file name
  new_filename=$(get_next_filename)
  echo $new_filename
  # Rename config.txt to the new file name
  mv "config.txt" "$new_filename"
fi

# Read the comma-separated lists from the file run_params.txt
IFS=',' read -ra list1 <<< $(sed -n 1p run_params.txt)
IFS=',' read -ra list2 <<< $(sed -n 2p run_params.txt)

make clean

# Execute the shell script cavier/run.sh" for each element in the first list
for item in $list1; do
  #CAVIER
  echo cavier/run.sh "$item" "exe"
  bash cavier/run.sh "$item" "exe"

  #FIVM
  echo make DATASET="$item"
  make DATASET="$item"
  for file in ./bin/"$item/$item"*_BATCH_1000; do
    echo "$file" --no-output
    "$file" --no-output
  done

done