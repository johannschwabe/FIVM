cmake_minimum_required(VERSION 3.24)
project(FIVM2)

set(CMAKE_CXX_STANDARD 17)

include_directories(src/lib ../backend/lib src tobe-generated)
add_executable(FIVM src/main.cpp generated/cpp/housing/housing_factorized_join_SINGLE.hpp src/application/housing/application_housing_factorized_join.hpp src/enumeration/result_enumeration.cpp src/application/housing/application_housing_factorized_join_generic.hpp src/application/housing/application_housing_base.hpp src/ring/ring_factorized.hpp)
