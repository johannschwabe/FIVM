#include <string>
#include <utility>
#include <vector>
#include <iostream>
#include <sstream>
#include <fstream>

std::string tabbing(int nr){
  return std::string(nr * 2, ' ');
}
// split the input variable by comma
std::vector<std::string>* split(std::string input){
  std::vector<std::string> *splitted = new std::vector<std::string>();
  std::string res = "";
  std::stringstream ss(input);
  std::string token;
  while(std::getline(ss, token, ',')) {
    splitted->push_back(token);
  }
  return splitted;
}

std::string join(std::vector<std::string>* splitted, std::string delimiter){
  std::string res = "";
  for(auto it = splitted->begin(); it != splitted->end(); ++it) {
    res += *it + delimiter;
  }
  return res;
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

struct Query{
  std::vector<ViewConfig*> *views;
  std::string query_name;
  int nr_views;
  bool call_batch_update;
  Query(std::string query_name, int nr_views, bool call_batch_update): query_name(query_name), nr_views(nr_views), call_batch_update(call_batch_update){
    views = new std::vector<ViewConfig*>();
  }

};

class Config {
  std::string filename;
  std::string dataset;
  std::vector<Query* >* queries = new std::vector<Query* >();

public:
  explicit Config(const std::string& config_file_name){
    std::ifstream config_file(config_file_name);
    std::getline(config_file, filename);
    std::getline(config_file, dataset);

    std::string query_list;
    std::getline(config_file, query_list);
    std::istringstream q(query_list);
    while(true){
      std::string query_name;
      std::string nr_views;
      std::string call_batch_update;
      getline(q,query_name, '|');
      if(!q) break;
      getline(q,nr_views,'|');
      getline(q,call_batch_update,'|');
      std::cout << query_name << " " << nr_views << std::endl;
      queries->push_back(new Query(query_name, std::stoi(nr_views), call_batch_update == "1" ));
    }
    for (auto &query : *queries) {
      for (int i = 0; i < query->nr_views; ++i) {
        std::string line;
        std::getline(config_file, line);
        if (!config_file) break;
        std::istringstream f(line);
        std::string view_name;
        std::string access_vars;
        std::string payload_vars;
        std::string payload_view;
        getline(f, view_name, '|');
        getline(f, access_vars, '|');
        getline(f, payload_vars, '|');
        getline(f, payload_view, '\n');
        query->views->push_back(new ViewConfig(view_name, access_vars, payload_vars, payload_view == "1"));
      }
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
  std::string generate_function(Query* query){
    std::vector<std::string> all_vars = std::vector<std::string>();
    std::string res = "void enumerate_" + query->query_name + "(dbtoaster::data_t &data, bool print_result) {\n";
    res += "    size_t output_size = 0; \n";
    std::string update_type = "DELTA_";
    if(query->call_batch_update){
      res += "    std::vector<"+update_type+query->query_name+"_entry> update = std::vector<"+update_type+query->query_name+"_entry>();";
    }
    int var_name_iter = 1;
    int tabbing_iter = 0;
    auto config = query->views;
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
    std::string combined_key = tabbing(tabbing_iter) + "auto combined_entry = " + update_type+query->query_name+"_entry(" + config->at(0)->payload_vars + ", ";

    auto splitted = split(config->at(0)->payload_vars);
    all_vars.insert(all_vars.end(), splitted->begin(), splitted->end());
    delete splitted;
    std::string combined_value = tabbing(tabbing_iter) + "auto combined_value =";
    auto view = config->begin();
    std::advance(view, 1);
    for (; view != config->end(); ++view) {
      const std::string nr = std::to_string(var_name_iter);
      res += tabbing(tabbing_iter) + (*view)->getStructName() + " e" + nr + ";\n";
      res += tabbing(tabbing_iter) + "const auto& rel" + nr + " = data." + (*view)->getViewAccessFunctionName() +
             "().getValueOrDefault(e" + nr + ".modify(" + (*view)->access_vars + "));\n";
      if((*view)->payload_vars == ""){
        res += tabbing(tabbing_iter) + "auto &payload_" + nr + " = rel" + nr + ";\n";
      } else {
        res += tabbing(tabbing_iter) + "for (auto &t" + nr + " : rel" + nr + ".store) {\n";
        tabbing_iter += 1;
        res += tabbing(tabbing_iter) + "auto &" + (*view)->payload_vars + " = std::get<0>(t" + nr + ".first);\n";
        res += tabbing(tabbing_iter) + "auto &payload_" + nr + " = t" + nr + ".second;\n";
        combined_key += (*view)->payload_vars + ", ";

        splitted = split((*view)->payload_vars);
        all_vars.insert(all_vars.end(), splitted->begin(), splitted->end());

        delete splitted;      }
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
    res += combined_value + ";\n";
    if(query->call_batch_update){
      res += combined_key + ", combined_value);\n";
    }
    res += tabbing(tabbing_iter) + "output_size++;\n";
    res += tabbing(tabbing_iter) + "if (print_result) std::cout << " + join(&all_vars, " << \",\" << ")+ "\"-> \" << combined_value << std::endl;\n";
    if(query->call_batch_update) {
      res += tabbing(tabbing_iter) + "update.push_back(combined_entry);\n";
    }

    auto end_brackets = std::string(tabbing_iter, '}');
    res += end_brackets;
    if(query->call_batch_update) {
      res += "data.on_batch_update_" + query->query_name + "(update.begin(), update.end());\n}\n";
    }
    else {
      res += "}\n";
    }
    return res;
  }
  std::string generate() {
    std::string res;
    std::string uppercase_filename = filename;
    std::transform(uppercase_filename.begin(), uppercase_filename.end(), uppercase_filename.begin(), toupper);
    res += "#ifndef " + uppercase_filename + "_HPP\n";
    res += "#define " + uppercase_filename + "_HPP\n";
    res += "#include \"../src/basefiles/application_" + filename + "_base.hpp\"\n\n";
    //iterate over queries and generate enumeration function for them
    for(auto query : *queries){
      res += generate_function(query);
      res += "\n";
    }
    res += start;
    for(auto query : *queries){
      res += "    cout << \"Enumerating " + query->query_name + "... \" << endl;\n";
      res += "    enumerate_" + query->query_name + "(data, print_result);\n";
    }

    res += "}\n #endif";
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