#ifndef APPLICATION_HOUSING_FACTORIZED_JOIN_HPP
#define APPLICATION_HOUSING_FACTORIZED_JOIN_HPP

#include "application_housing_base.hpp"

void Application::on_snapshot(dbtoaster::data_t &data) {
  on_end_processing(data, false);
}

void Application::on_begin_processing(dbtoaster::data_t &data) {
}

void Application::on_end_processing(dbtoaster::data_t &data, bool print_result) {

  cout << endl << "Enumerating factorized join result... " << endl;

  size_t output_size = 0;
  const auto &toplevel_view = data.get_V_postcode_HSIRDT1();
  for (auto &t0: toplevel_view.store) {
    auto &key_0 = t0.first;
    auto payload_0 = t0.second;
    auto combined_key = std::tuple_cat(key_0);
    auto combined_value = payload_0;
    output_size++;
    if (print_result) std::cout << combined_key << " -> " << combined_value << std::endl;
  }
}

#endif
