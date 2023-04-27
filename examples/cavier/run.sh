echo $1
g++ -O3 -DNDEBUG -Wall -Wno-unused-variable -std=c++17 -pedantic src/result_enumeration.cpp -o cavier/app_generator
./cavier/app_generator cavier/config/$1/$1.txt cavier/application/$1/$1.hpp

if [ $2 = "batch" ]; then
    echo "Batch"
    ../bin/run_backend.sh --batch -o cavier/cpp/$1/$1.hpp cavier/m3/$1/$1_BATCH.m3
    echo g++ -O3 -DNDEBUG -Wall -Wno-unused-variable -std=c++17 -pedantic src/main.cpp -I ../backend/lib -I src -I src/lib -DBATCH_SIZE=1000 -include cavier/cpp/$1/$1.hpp -include cavier/application/$1/$1.hpp -o bin/$1/$1_BATCH_1000; \
    g++ -O3 -DNDEBUG -Wall -Wno-unused-variable -std=c++17 -pedantic src/main.cpp -I ../backend/lib -I src -I src/lib -DBATCH_SIZE=1000 -include cavier/cpp/$1/$1.hpp -include cavier/application/$1/$1.hpp -o bin/$1/$1_BATCH_1000; \
else
    echo "Single"
    ../bin/run_backend.sh --single -o cavier/cpp/$1/$1.hpp cavier/m3/$1/$1_SINGLE.m3
    echo g++ -O3 -DNDEBUG -Wall -Wno-unused-variable -std=c++17 -pedantic src/main.cpp -I ../backend/lib -I src -I src/lib -include cavier/cpp/$1/$1.hpp -include cavier/application/$1/$1.hpp -o bin/$1/$1_SINGLE; \
    g++ -O3 -DNDEBUG -Wall -Wno-unused-variable -std=c++17 -pedantic src/main.cpp -I ../backend/lib -I src -I src/lib -include cavier/cpp/$1/$1.hpp -include cavier/application/$1/$1.hpp -o bin/$1/$1_SINGLE; \

fi