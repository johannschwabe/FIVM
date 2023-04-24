#include <string>
#include <utility>
#include <vector>
#include <iostream>
#include <sstream>
#include <fstream>

std::string tabbing(int nr){
  return std::string(nr * 2, ' ');
}

struct ViewConfig{
  std::string access_vars;
  std::string payload_vars;
  bool payload_view;
  std::string view_name;
  ViewConfig(std::string view_name, const std::string& access_vars, const std::string& payload_vars, const bool payload_view):
    access_vars(access_vars), payload_vars(payload_vars), payload_view(payload_view),view_name(view_name)
    {}

  std::string getStructName() const {
    return "V_" + view_name + "_entry";
  };

  std::string getViewAccessFunctionName() const {
    return "get_V_" + view_name;
  };
};

class Config {
  std::string filename;
  std::string dataset;
  std::vector<ViewConfig* >* config = new std::vector<ViewConfig* >();

public:
  explicit Config(const std::string& config_file_name){
    std::ifstream config_file(config_file_name);
    std::getline(config_file, filename);
    std::getline(config_file, dataset);

    while (true){
      std::string line;
      std::getline(config_file, line);
      if(!config_file) break;
      std::istringstream f(line);
      std::string view_name;
      std::string access_vars;
      std::string payload_vars;
      std::string payload_view;
      getline(f, view_name, '|');
      getline(f, access_vars, '|');
      getline(f, payload_vars, '|');
      getline(f, payload_view, '\n');
      config->push_back(new ViewConfig(view_name, access_vars, payload_vars, payload_view == "1"));
    }
  }

  const std::string start = "void Application::on_snapshot(dbtoaster::data_t& data) {\n"
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
    std::string res;
    std::string uppercase_filename = filename;
    std::transform(uppercase_filename.begin(), uppercase_filename.end(), uppercase_filename.begin(), toupper);
    res += "#ifndef " + uppercase_filename + "_HPP\n";
    res += "#define " + uppercase_filename + "_HPP\n";
    res += "#include \"../src/basefiles/application_" + dataset + "_base.hpp\"\n\n";
    res += start;
    int var_name_iter = 1;
    int tabbing_iter = 0;
    if(config->at(0)->access_vars != "") {
      res += "    const auto& top_level_view = data." + config->at(0)->getViewAccessFunctionName() + "();\n";
      res += "    " + config->at(0)->getStructName() + "* r0 = top_level_view.head;\n";
      res += "    while (r0 != nullptr) {\n";
      res += "        auto ring = r0->__av;\n";
      res += "        r0 = r0->nxt;\n";
      tabbing_iter += 1;
    } else {
      res += "    const auto& ring = data." + config->at(0)->getViewAccessFunctionName() + "();\n";
    }
    res += tabbing(tabbing_iter) + "for (auto &t0 : ring.store) {\n";
    tabbing_iter += 1;
    res += tabbing(tabbing_iter) + "auto &" + config->at(0)->payload_vars + " = std::get<0>(t0.first);\n";
    res += tabbing(tabbing_iter) + "auto payload_0 = t0.second;\n";
    std::string combined_key = tabbing(tabbing_iter) + "auto combined_key = std::tuple_cat(t0.first, ";
    std::string combined_value = tabbing(tabbing_iter) + "auto combined_value =";
    auto view = config->begin();
    std::advance(view, 1);
    for (; view != config->end(); ++view) {
      const std::string nr = std::to_string(var_name_iter);
      res += tabbing(tabbing_iter) + (*view)->getStructName() + " e" + nr + ";\n";
      res += tabbing(tabbing_iter) + "const auto& rel" + nr + " = data." + (*view)->getViewAccessFunctionName() +
             "().getValueOrDefault(e" + nr + ".modify(" + (*view)->access_vars + "));\n";
      res += tabbing(tabbing_iter) + "for (auto &t" + nr + " : rel" + nr + ".store) {\n";
      tabbing_iter += 1;
      res += tabbing(tabbing_iter) + "auto &" + (*view)->payload_vars + " = std::get<0>(t" + nr + ".first);\n";
      res += tabbing(tabbing_iter) + "auto &payload_" + nr + " = t" + nr + ".second;\n";
      combined_key += "t" + nr + ".first, ";
      if((*view)->payload_view) {
        combined_value += "payload_" + nr + " * ";
      }
      ++var_name_iter;
    }

    combined_key.pop_back();
    combined_key.pop_back();
    combined_value.pop_back();
    combined_value.pop_back();
    combined_value.pop_back();
    res += combined_key + ");\n";
    res += combined_value + ";\n";
    res += tabbing(tabbing_iter) + "output_size++;\n";
    res += tabbing(tabbing_iter) + "if (print_result) std::cout << combined_key << \" -> \" << combined_value << std::endl;\n";

    auto end_brackets = std::string(tabbing_iter + 1, '}');
    res += end_brackets;
    res += "\n #endif";
    return res;
  }
};


int main(int argc, char** argv) {
  if(argc == 1) {
    std::cout << "No Config file given" << std::endl;
    return 1;
  }
  if(argc > 3) {
    std::cout << "Invalid Config file linked" << std::endl;
    return 1;
  }
  auto conf = new Config(argv[1]);
  auto res = conf->generate();
  if (argc == 3){
    std::ofstream file(argv[2]);
    file << res;
    file.close();
  } else {
    std::cout << res;
  }
}