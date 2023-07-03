#!/bin/zsh

g++ -O3 -DNDEBUG -Wall -Wno-unused-variable -std=c++17 -pedantic src/result_enumeration.cpp -o cavier/app_generator || exit 1;
mkdir -p "cavier/application/$1"
mkdir -p "bin/$1"

batchSize=1000
if [[ $3 != "" ]]; then
  batchSize=$3
fi

for file in cavier/config/$1/*; do
  filename=$(basename "$file" .txt)
  echo $filename
  echo ./cavier/app_generator "$file" "cavier/application/$1/$filename.hpp"
  ./cavier/app_generator "$file" "cavier/application/$1/$filename.hpp"
  ../bin/run_backend.sh --batch -o "cavier/cpp/$1/$filename.hpp" "cavier/m3/$1/$filename.m3"

  customhashmapargs=($(zsh custom_hashmap.zsh "$filename"))
  echo g++ -O3 -DNDEBUG -Wall -Wno-unused-variable -std=c++17 -pedantic -Wno-unused-local-typedefs src/main.cpp -I ../backend/lib -I src -I src/lib -I src/TLXInstall/include -L src/TLXInstall/lib -ltlx -DBATCH_SIZE=$batchSize -include cavier/cpp/$1/$filename.hpp -include cavier/application/$1/$filename.hpp -o bin/$1/CAVIER_${filename}_BATCH_1000 "${customhashmapargs[@]}" || exit 1;
  g++ -O3 -DNDEBUG -Wall -Wno-unused-variable -std=c++17 -pedantic -Wno-unused-local-typedefs src/main.cpp -I ../backend/lib -I src -I src/lib -I src/TLXInstall/include -L src/TLXInstall/lib -ltlx -DBATCH_SIZE=$batchSize -include cavier/cpp/$1/$filename.hpp -include cavier/application/$1/$filename.hpp -o bin/$1/CAVIER_${filename}_BATCH_1000 "${customhashmapargs[@]}" || exit 1;
  if [ $2 = "exe" ]; then
    echo ./bin/$1/CAVIER_${filename}_BATCH_1000
    ./bin/$1/CAVIER_${filename}_BATCH_1000
  elif [[ "$2" =~ ^-r[0-9]+ ]]; then
    r_value="${2:2}"
    echo ./bin/$1/CAVIER_${filename}_BATCH_1000 -r "$r_value"
    ./bin/$1/CAVIER_${filename}_BATCH_1000 -r "$r_value"
  fi
done