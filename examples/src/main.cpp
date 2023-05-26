#include <iostream>

#include "basefiles/application.hpp"

int main(int argc, char** argv) {
    
    int opt_num_runs = 1;
    bool opt_print_result = false;
    bool opt_count_view_size = false;
    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], "--num-runs") == 0 || strcmp(argv[i], "-r") == 0) {
            opt_num_runs = atoi(argv[i + 1]);
        }
        if (strcmp(argv[i], "--output") == 0){
          opt_print_result = true;
        }
        if (strcmp(argv[i], "--sizes") == 0){
          opt_count_view_size = true;
        }
    }

#ifndef __APPLE__
    cpu_set_t  mask;
    CPU_ZERO(&mask);
    CPU_SET(0, &mask);
    sched_setaffinity(0, sizeof(mask), &mask);
#endif

    Application app;
    app.run(opt_num_runs, opt_print_result, opt_count_view_size);
    
    return 0;
}