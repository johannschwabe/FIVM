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
    V_house_H1_entry e1;
    const auto &rel1 = data.get_V_house_H1().getValueOrDefault(e1.modify(std::get<0>(key_0)));
    for (auto &t1: rel1.store) {
      auto &key_1 = t1.first;
      auto &payload_1 = t1.second;
      V_sainsburys_S1_entry e2;
      const auto &rel2 = data.get_V_sainsburys_S1().getValueOrDefault(e2.modify(std::get<0>(key_0)));
      for (auto &t2: rel2.store) {
        auto &key_2 = t2.first;
        auto &payload_2 = t2.second;
        V_typeeducation_I1_entry e3;
        const auto &rel3 = data.get_V_typeeducation_I1().getValueOrDefault(e3.modify(std::get<0>(key_0)));
        for (auto &t3: rel3.store) {
          auto &key_3 = t3.first;
          auto &payload_3 = t3.second;
          V_pricerangerest_R1_entry e4;
          const auto &rel4 = data.get_V_pricerangerest_R1().getValueOrDefault(e4.modify(std::get<0>(key_0)));
          for (auto &t4: rel4.store) {
            auto &key_4 = t4.first;
            auto &payload_4 = t4.second;
            V_unemployment_D1_entry e5;
            const auto &rel5 = data.get_V_unemployment_D1().getValueOrDefault(e5.modify(std::get<0>(key_0)));
            for (auto &t5: rel5.store) {
              auto &key_5 = t5.first;
              auto &payload_5 = t5.second;
              V_nbbuslines_T1_entry e6;
              const auto &rel6 = data.get_V_nbbuslines_T1().getValueOrDefault(e6.modify(std::get<0>(key_0)));
              for (auto &t6: rel6.store) {
                auto &key_6 = t6.first;
                auto &payload_6 = t6.second;
                auto combined_key = std::tuple_cat(key_0, key_1, key_2, key_3, key_4, key_5, key_6);
                auto combined_value = payload_0 * payload_1 * payload_2 * payload_3 * payload_4 * payload_5 * payload_6;
                output_size++;
                if (print_result) std::cout << combined_key << " -> " << combined_value << std::endl;
              }
            }
          }
        }
      }
    }
  }
}

#endif
