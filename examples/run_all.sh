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
# Execute the shell script cavier/run.sh" for each element in the first list
for item in $list1; do
  echo cavier/run.sh "$item" "batch" "exe"
  bash cavier/run.sh "$item" "batch" "exe"
done

# Execute commands for each element in the second list
for item in $list2; do
  echo DATASET="$item"
  make DATASET="$item"
  echo ./bin/"$item"/"$item"_BATCH_1000 --no-output
  ./bin/"$item"/"$item"_BATCH_1000 --no-output
done
