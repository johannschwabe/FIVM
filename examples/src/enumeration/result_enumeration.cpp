#include <string>
#include <vector>
#include <iostream>
#include <sstream>

class Config {
public:
  const std::string basename = "get_V_postcode_HSIRDT1";
  std::string conf[6][3] = {
      {"house_H1",          "key_0[0]"},
      {"sainsburys_S1",     "key_0[0]"},
      {"typeeducation_I1",  "key_0[0]"},
      {"pricerangerest_R1", "key_0[0]"},
      {"unemployment_D1",   "key_0[0]"},
      {"nbbuslines_T1",     "key_0[0]"},
  };
  const std::string filename = "APPLICATION_HOUSING_FACTORIZED_JOIN";
  const std::string start = "\n"
                            "#include \"application_housing_base.hpp\"\n"
                            "\n"
                            "void Application::on_snapshot(dbtoaster::data_t& data) {\n"
                            "    on_end_processing(data, false);\n"
                            "}\n"
                            "\n"
                            "void Application::on_begin_processing(dbtoaster::data_t& data) {\n"
                            "}\n"
                            "\n"
                            "void Application::on_end_processing(dbtoaster::data_t& data, bool print_result) {\n"
                            "\n"
                            "    cout << endl << \"Enumerating factorized join result... \" << endl;\n"
                            "\n"
                            "    size_t output_size = 0; \n";

  std::string generate() {
    std::string res = "";
    std::string uppercase_filename = filename;
    std::transform(uppercase_filename.begin(), uppercase_filename.end(), uppercase_filename.begin(), toupper);
    res += "#ifndef " + uppercase_filename + "_HPP\n";
    res += "#define " + uppercase_filename + "_HPP\n";
    res += start;
    res += "    const auto& toplevel_view = data." + basename + "();\n";
    res += "    for (auto &t0 : toplevel_view.store) {\n"
           "        auto &key_0 = t0.first;\n"
           "        auto payload_0 = t0.second;\n";
    int iter = 1;
    auto tabbing = std::string((iter + 1) * 4, ' ');
    for (auto view: conf) {
      const std::string nr = std::to_string(iter);
      res += tabbing + getStructName(view[0]) + " e" + nr + ";\n";
      res += tabbing + "const auto& rel" + nr + " = data." + getViewAccessFunctionName(view[0]) +
             "().getValueOrDefault(e" + nr + ".modify(" + getAccessVariable(view[1]) + "));\n";
      res += tabbing + "for (auto &t" + nr + " : rel" + nr + ".store) {\n";
      tabbing = std::string((iter + 2) * 4, ' ');
      res += tabbing + "auto &key_" + nr + " = t" + nr + ".first;\n";
      res += tabbing + "auto &payload_" + nr + " = t" + nr + ".second;\n";
      ++iter;
    }
    std::string combined_key = tabbing + "auto combined_key = std::tuple_cat(";
    std::string combined_value = tabbing + "auto combined_value = ";
    for (int j = 0; j < iter; ++j) {
      combined_key += "key_" + std::to_string(j) + ", ";
      combined_value += "payload_" + std::to_string(j) + " * ";
    }
    combined_key.pop_back();
    combined_key.pop_back();
    combined_value.pop_back();
    combined_value.pop_back();
    combined_value.pop_back();
    res += combined_key + ");\n";
    res += combined_value + ";\n";
    res += tabbing + "output_size++;\n";
    res += tabbing + "if (print_result) std::cout << combined_key << \" -> \" << combined_value << std::endl;\n";

    auto end_brackets = std::string(iter + 1, '}');
    res += end_brackets;
    res += "\n #endif";
    std::cout << res << std::endl;
    return res;
  }

  std::string getAccessVariable(std::string config) {
    std::string res = "";
    std::istringstream f(config);
    std::string s;
    while (getline(f, s, ',')) {
      std::string var;
      std::string idx;
      std::istringstream f2(s);
      getline(f2, var, '[');
      getline(f2, idx, ']');
      res += "std::get<" + idx + ">(" + var + "), ";
    }
    res.pop_back();
    res.pop_back();
    return res;
  }

  std::string getStructName(std::string config) {
    return "V_" + config + "_entry";
  };

  std::string getViewAccessFunctionName(std::string config) {
    return "get_V_" + config;
  };
};

int main() {
  auto conf = new Config();
  conf->generate();
}