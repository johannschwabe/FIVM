g++ -O3 -DNDEBUG -Wall -Wno-unused-variable -std=c++17 -pedantic src/result_enumeration.cpp -o cavier/app_generator || exit 1;
mkdir -p "cavier/application/$1"
mkdir -p "bin/$1"

../bin/run_backend.sh --batch -o cavier/cpp/$1/$1.hpp cavier/m3/$1/$1_BATCH.m3

for file in cavier/config/$1/*; do
  filename=$(basename "$file" .txt)
  echo $filename
  echo ../cavier/app_generator "$file" "cavier/application/$1/$filename.hpp"
  ./cavier/app_generator "$file" "cavier/application/$1/$filename.hpp"

  echo g++ -O3 -DNDEBUG -Wall -Wno-unused-variable -std=c++17 -pedantic src/main.cpp -I ../backend/lib -I src -I src/lib -DBATCH_SIZE=1000 -include cavier/cpp/$1/$1.hpp -include cavier/application/$1/$filename.hpp -o bin/$1/${filename}_BATCH_1000; \
  g++ -O3 -DNDEBUG -Wall -Wno-unused-variable -std=c++17 -pedantic src/main.cpp -I ../backend/lib -I src -I src/lib -DBATCH_SIZE=1000 -include cavier/cpp/$1/$1.hpp -include cavier/application/$1/$filename.hpp -o bin/$1/CAVIER_${filename}_BATCH_1000 || exit 1; \
  if [ $2 = "exe" ]; then
    ./bin/$1/CAVIER_${filename}_BATCH_1000 --no-output
  fi
done


