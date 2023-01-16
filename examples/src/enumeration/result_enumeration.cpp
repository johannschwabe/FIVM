#include <string>
#include <utility>
#include <vector>
#include <iostream>
#include <sstream>
#include <fstream>

std::string tabbing(int nr){
  return {static_cast<char>((nr) * 4), ' '};
}

struct ViewConfig{
  std::string view_name;
  std::string access_vars;
  ViewConfig(std::string view_name, const std::string& unparsed_access_vars): view_name(std::move(view_name)) {
    access_vars = getAccessVariable(unparsed_access_vars);
  }
  static std::string getAccessVariable(const std::string& config) {
    std::string res;
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
  std::string getStructName() const {
    return "V_" + view_name + "_entry";
  };

  std::string getViewAccessFunctionName() const {
    return "get_V_" + view_name;
  };
};

class Config {
  std::string basename;
  std::string filename;
  std::string base_file;
  bool grouped;
  std::vector<ViewConfig* >* config = new std::vector<ViewConfig* >();

public:
  explicit Config(const std::string& config_file_name){
    std::ifstream config_file(config_file_name);
    std::getline(config_file, filename);
    std::getline(config_file, basename);
    std::getline(config_file, base_file);
    std::string _grouped;
    std::getline(config_file, _grouped);
    grouped = _grouped != "-";

    while (true){
      std::string line;
      std::getline(config_file, line);
      if(!config_file) break;
      std::istringstream f(line);
      std::string view_name;
      std::string access_vars;
      getline(f, view_name, '|');
      getline(f, access_vars, '\n');
      config->push_back(new ViewConfig(view_name, access_vars));
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
    res += "#include \"" + base_file + ".hpp\"\n\n";
    res += start;
    int var_name_iter = 1;
    int tabbing_iter = 0;

    if(grouped) {
      res += "    const auto& top_level_view = data.get_V_" + basename + "();\n";
      res += "    V_" + basename + "_entry* r0 = top_level_view.head;\n";
      res += "    while (r0 != nullptr) {\n";
      res += "        auto ring = r0->__av;\n";
      res += "        r0 = r0->nxt;\n";
      tabbing_iter += 1;
    } else {
      res += "    const auto& ring = data.get_V_" + basename + "();\n";
    }
    res += tabbing(tabbing_iter) + "for (auto &t0 : ring.store) {\n";
    tabbing_iter += 1;
    res += tabbing(tabbing_iter) + "auto &key_0 = t0.first;\n";
    res += tabbing(tabbing_iter) + "auto payload_0 = t0.second;\n";

    for (auto view: *config) {
      const std::string nr = std::to_string(var_name_iter);
      res += tabbing(tabbing_iter) + view->getStructName() + " e" + nr + ";\n";
      res += tabbing(tabbing_iter) + "const auto& rel" + nr + " = data." + view->getViewAccessFunctionName() +
             "().getValueOrDefault(e" + nr + ".modify(" + view->access_vars + "));\n";
      res += tabbing(tabbing_iter) + "for (auto &t" + nr + " : rel" + nr + ".store) {\n";
      tabbing_iter += 1;
      res += tabbing(tabbing_iter) + "auto &key_" + nr + " = t" + nr + ".first;\n";
      res += tabbing(tabbing_iter) + "auto &payload_" + nr + " = t" + nr + ".second;\n";
      ++var_name_iter;
    }
    std::string combined_key = tabbing(tabbing_iter) + "auto combined_key = std::tuple_cat(";
    std::string combined_value = tabbing(tabbing_iter) + "auto combined_value = ";
    for (int j = 0; j < var_name_iter; ++j) {
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
    res += tabbing(tabbing_iter) + "output_size++;\n";
    res += tabbing(tabbing_iter) + "if (print_result) std::cout << combined_key << \" -> \" << combined_value << std::endl;\n";

    auto end_brackets = std::string(tabbing_iter + 1, '}');
    res += end_brackets;
    res += "\n #endif";
    std::cout << res << std::endl;
    return res;
  }
};


int main(int argc, char** argv) {
  if(argc == 1) {
    std::cout << "No Config file given" << std::endl;
    return 1;
  }
  if(argc > 2) {
    std::cout << "Invalid Config file linked" << std::endl;
    return 1;
  }
  std::cout << "Config File: " << argv[1] << std::endl;
  //auto conf = new Config("src/enumeration/housing_factorized_enumeration_config.txt");
  auto conf = new Config(argv[1]);
  conf->generate();
}