#include <string>
#include <utility>
#include <vector>
#include <iostream>
#include <sstream>
#include <fstream>
#include <map>
#include <algorithm>

std::string tabbing(int nr) {
  return std::string(nr * 2, ' ');
}

// split the input variable by comma
std::vector<std::string> *split(std::string input, char delimiter = ',') {
  std::vector<std::string> *splitted = new std::vector<std::string>();
  std::string res = "";
  std::stringstream ss(input);
  std::string token;
  while (std::getline(ss, token, delimiter)) {
    splitted->push_back(token);
  }
  return splitted;
}

std::string join(std::vector<std::string> *splitted, std::string delimiter) {
  std::string res = "";
  for (auto it = splitted->begin(); it != splitted->end(); ++it) {
    res += *it + delimiter;
  }
  res.erase(res.end() - delimiter.size(), res.end());
  return res;
}


struct ViewConfig {
  std::string access_vars;
  std::string payload_vars;
  bool payload_view;
  std::string view_name;

  ViewConfig(std::string view_name, const std::string &access_vars, const std::string &payload_vars,
             const bool payload_view) :
      access_vars(access_vars), payload_vars(payload_vars), payload_view(payload_view), view_name(view_name) {}

  std::string getStructName() const {
    return "V_" + view_name + "_entry";
  };

  std::string getViewAccessFunctionName() const {
    return "get_V_" + view_name;
  };
};

struct Query {
  std::vector<ViewConfig *> *views;
  std::string query_name;
  int nr_views;
  bool call_batch_update;

  Query(std::string query_name, int nr_views, bool call_batch_update) : query_name(query_name), nr_views(nr_views),
                                                                        call_batch_update(call_batch_update) {
    views = new std::vector<ViewConfig *>();
  }

};

class Config {
  std::string filename;
  std::string dataset;
  std::string filetype;
  std::string executor;
  std::string propagation_size;
  std::vector<Query *> *queries = new std::vector<Query *>();
  std::vector<std::string> *relations;
  std::map<std::string, std::vector<std::string> > *query_atoms;
  std::map<std::string, std::vector<std::string> > *enumerated_relations;
  std::string write_to_config;

public:
  explicit Config(const std::string &config_file_name) {
    std::ifstream config_file(config_file_name);
    std::getline(config_file, filename);
    std::getline(config_file, dataset);
    std::getline(config_file, filetype);
    std::getline(config_file, executor);
    std::getline(config_file, propagation_size);

    std::string query_list;
    std::getline(config_file, query_list);
    std::istringstream q(query_list);
    while (true) {
      std::string query_name;
      std::string nr_views;
      std::string call_batch_update;
      getline(q, query_name, '|');
      if (!q) break;
      getline(q, nr_views, '|');
      getline(q, call_batch_update, '|');
      queries->push_back(new Query(query_name, std::stoi(nr_views), call_batch_update == "1"));
    }
    std::string relation_list;
    std::getline(config_file, relation_list);
    relations = split(relation_list, '|');

    query_atoms = new std::map<std::string, std::vector<std::string> >();
    std::string query_atom_list;
    std::getline(config_file, query_atom_list);
    auto query_atom_list_splitted = split(query_atom_list, '|');
    for (auto &query_atom: *query_atom_list_splitted) {
      auto splitted = split(query_atom, ':');
      auto query_name = splitted->at(0);
      auto atoms = split(splitted->at(1), ',');
      query_atoms->insert(std::pair<std::string, std::vector<std::string> >(query_name, *atoms));
    }

    enumerated_relations = new std::map<std::string, std::vector<std::string> >();
    std::string enumerated_relation_list;
    std::getline(config_file, enumerated_relation_list);
    auto enumerated_relations_list = split(enumerated_relation_list, '|');
    for (auto &enumerated_relation: *enumerated_relations_list) {
      auto splitted = split(enumerated_relation, ':');
      auto relation_name = splitted->at(0);
      auto variables = split(splitted->at(1), ',');
      enumerated_relations->insert(std::pair<std::string, std::vector<std::string> >(relation_name, *variables));
    }
    for (auto &query: *queries) {
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
    std::string fixed_filename;
    auto splitted_filename = split(filename, '-');
    if (splitted_filename->size() == 1) {
      fixed_filename = filename;
    } else {
      fixed_filename = splitted_filename->at(0);
    }
    write_to_config =
        "struct Measurement {\n std::string query_name;\n long update_time;\nstd::string relations;\nstd::string free_variables;\n long enumeration_time;\n long size_output; "
        "Measurement(std::string query_name, long update_time, std::string relations) : query_name(query_name), update_time(update_time), relations(relations), free_variables(\"\"), enumeration_time(0L), size_output(0L) {}\n"
        "};\n\n"
        "std::ofstream& operator<<(std::ofstream& os, const Measurement *measurement) {\n"
        "    os << measurement->query_name << \" | \"\n"
        "       << measurement->update_time << \" | \"\n"
        "       << measurement->enumeration_time << \" | \"\n"
        "       << measurement->size_output << \" | \"\n"
        "       << measurement->free_variables << \" | \"\n"
        "       << measurement->relations;\n"
        "    return os;\n"
        "}\n\n"
        "void write_to_config(std::vector<Measurement*>* measurements, std::ofstream& measurements_file, std::string relations){\n"
        "    measurements_file << \"" + fixed_filename + "|" + executor + "|\"<<\"" + dataset + "|\" << \"|\" << relations << \":\";"
        "for (size_t i = 0; i < measurements->size(); i++) {\n"
        "            measurements_file << measurements->at(i);\n"
        "            if (i != measurements->size() - 1) {\n"
        "                measurements_file << \" : \";\n"
        "            }\n"
        "        }"
        "    measurements_file << \"\\n\";\n"
        "}";
  };

  ~Config() {
    for (auto &query: *queries) {
      for (auto &view: *query->views) {
        delete view;
      }
      delete query->views;
      delete query;
    }
    delete queries;
    delete relations;
    for (auto &query_atom: *query_atoms) {
      delete &query_atom.second;
    }
    delete query_atoms;
    for (auto &enumerated_relation: *enumerated_relations) {
      delete &enumerated_relation.second;
    }
    delete enumerated_relations;
  }

  const std::string start = "void Application::on_snapshot(dbtoaster::data_t& data) {\n"
                            "    on_end_processing(data, false, false);\n"
                            "}\n"
                            "\n"
                            "void Application::on_begin_processing(dbtoaster::data_t& data) {\n"
                            "}\n"

                            "\n"
                            "void Application::on_end_processing(dbtoaster::data_t& data, bool print_result, bool count_view_size) {\n"
                            "\n"
                            "    cout << endl << \"Enumerating factorized join result... \" << endl;\n"
                            "\n"
                            "std::string relation_list = \"\";\n"
                            "  for(auto& rel: relations){\n"
                            "    relation_list += rel->get_name() + \",\";\n"
                            "  }\n"
                            "  relation_list = relation_list.substr(0, relation_list.size() - 1);"
                            "    std::vector<Measurement*> measurements;\n"
                            "    size_t output_size = 0; \n";


  std::string generate_application() {
    std::string base = "const string dataPath = \"data/" + dataset + "\";\n";
    base += "void Application::init_relations() {\n"
            "    clear_relations();\n\n";
    for (auto &relation: *relations) {
      std::string uppercase;
      std::transform(relation.begin(), relation.end(), std::back_inserter(uppercase),
                     [](unsigned char c) { return std::toupper(c); });
      base += "\t\t#if defined(RELATION_" + uppercase + "_STATIC)\n"
                                                        "        relations.push_back(std::unique_ptr<IRelation>(\n"
                                                        "            new EventDispatchableRelation<" + relation +
              "_entry>(\n"
              "                \"" + relation + "\", dataPath + \"/" + relation + "." + filetype + "\", '|', true,\n"
                                                                                                   "                [](dbtoaster::data_t& data) {\n"
                                                                                                   "                    return [&](" +
              relation + "_entry& t) {\n"
                         "                        data.on_insert_" + relation + "(t);\n"
                                                                                "                    };\n"
                                                                                "                }\n"
                                                                                "        )));\n"
                                                                                "    #elif defined(RELATION_" +
              uppercase + "_DYNAMIC) && defined(BATCH_SIZE)\n"
                          "        typedef const std::vector<DELTA_" + relation + "_entry>::iterator CIterator" +
              relation + ";\n"
                         "        relations.push_back(std::unique_ptr<IRelation>(\n"
                         "            new BatchDispatchableRelation<DELTA_" + relation + "_entry>(\n"
                                                                                         "                \"" +
              relation + "\", dataPath + \"/" + relation + "." + filetype + "\", '|', false,\n"
                                                                            "                [](dbtoaster::data_t& data) {\n"
                                                                            "                    return [&](CIterator" +
              relation +
              "& begin, CIterator" + relation + "& end) {\n"
                                                "                        data.on_batch_update_" + relation +
              "(begin, end);\n"
              "                    };\n"
              "                }\n"
              "        )));\n"
              "    #elif defined(RELATION_" + uppercase + "_DYNAMIC)\n"
                                                          "        relations.push_back(std::unique_ptr<IRelation>(\n"
                                                          "            new EventDispatchableRelation<" + relation +
              "_entry>(\n"
              "                \"" + relation + "\", dataPath + \"/" + relation + "." + filetype + "\", '|', false,\n"
                                                                                                   "                [](dbtoaster::data_t& data) {\n"
                                                                                                   "                    return [&](" +
              relation + "_entry& t) {\n"
                         "                        data.on_insert_" + relation + "(t);\n"
                                                                                "                    };\n"
                                                                                "                }\n"
                                                                                "        )));\n"
                                                                                "    #endif\n\n";
    }
    base += "}\n\n";

    return base;
  }

  std::string generate_struct(std::vector<std::string> *all_vars, std::string name) {
    std::string res = "struct " + name + "_entry {\n";
    for (auto var: *all_vars) {
      res += "    std::any " + var + ";\n";
    }
    res += "    long combined_value;\n";
    res += "};\n\n";
    return res;
  }

  std::string generate_function(Query *query) {
    std::vector<std::string> all_vars = std::vector<std::string>();
    std::string head =
        "long enumerate_" + query->query_name +
        "(dbtoaster::data_t &data, bool print_result, bool count_view_size, Measurement *measure) {\n";
    std::string res = "    size_t output_size = 0; \n";
    res += "std::ofstream view_sizes_file;\n";
    res += "if(count_view_size) {\nview_sizes_file = std::ofstream(\"sizes/" + filename + "_" + query->query_name +
           "_view_sizes.csv\");\n";

    auto it = std::next(query->views->begin()); // Iterator pointing to the second view
    auto view_0 = *it;
    res += "view_sizes_file << \"" + query->views->at(0)->view_name + ", \" << 0<< std::endl;\n";
    for (; it != query->views->end(); ++it) {
      auto view = *it;
      res += "view_sizes_file << \"" + view->view_name + ", \" << data." + view->getViewAccessFunctionName() +
             "().primary_index->count()<< std::endl;\n";
    }
    res += "view_sizes_file.close();\n}\n";
    res += "Stopwatch enumeration_timer;\nenumeration_timer.restart();\n";
    if (query->call_batch_update) {
      res += "Stopwatch propagation_timer;\n";
    }
    res += "long propagation_time = 0;\n";
    res += "long enumeration_time = 0;\n";
    res +=
        "std::ofstream output_file;\n if (print_result) output_file.open (\"output/" + query->query_name + ".csv\");\n";
    std::string update_type = "DELTA_";
    if (query->call_batch_update) {
      res += "    std::vector<" + update_type + query->query_name + "_entry> update = std::vector<" + update_type +
             query->query_name + "_entry>();";
    } else {
      res += "    std::vector<" + query->query_name + "_entry> update = std::vector<" +
             query->query_name + "_entry>();";
    }
    res += "    update.reserve(" + propagation_size + ");\n";
    int var_name_iter = 1;
    int tabbing_iter = 0;
    auto config = query->views;
    if (config->at(0)->access_vars != "") {
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
    auto splitted = split(config->at(0)->payload_vars);
    int i = 0;
    for (auto &var: *splitted) {
      res += tabbing(tabbing_iter) + "auto &" + var + " = std::get<" + std::to_string(i) + ">(t0.first);\n";
      i += 1;
    }
    res += tabbing(tabbing_iter) + "auto payload_0 = t0.second;\n";

    all_vars.insert(all_vars.end(), splitted->begin(), splitted->end());
    delete splitted;
    std::string combined_value = tabbing(tabbing_iter) + "auto combined_value = 1  * ";
    auto view = config->begin();
    std::advance(view, 1);
    for (; view != config->end(); ++view) {
      const std::string nr = std::to_string(var_name_iter);
      res += tabbing(tabbing_iter) + (*view)->getStructName() + " e" + nr + ";\n";
      res += tabbing(tabbing_iter) + "const auto& rel" + nr + " = data." + (*view)->getViewAccessFunctionName() +
             "().getValueOrDefault(e" + nr + ".modify(" + (*view)->access_vars + "));\n";

      if ((*view)->payload_vars == "") {
        res += tabbing(tabbing_iter) + "auto &payload_" + nr + " = rel" + nr + ";\n";
      } else {
        res += tabbing(tabbing_iter) + "for (auto &t" + nr + " : rel" + nr + ".store) {\n";
        tabbing_iter += 1;
        splitted = split((*view)->payload_vars);
        int i = 0;
        for (auto &var: *splitted) {
          res +=
              tabbing(tabbing_iter) + "auto &" + var + " = std::get<" + std::to_string(i) + ">(t" + nr + ".first);\n";
          i += 1;
        }
        res += tabbing(tabbing_iter) + "auto &payload_" + nr + " = t" + nr + ".second;\n";

        all_vars.insert(all_vars.end(), splitted->begin(), splitted->end());

        delete splitted;
      }
      if ((*view)->payload_view) {
        combined_value += "payload_" + nr + " * ";
      }
      ++var_name_iter;
    }

    combined_value.pop_back();
    combined_value.pop_back();
    combined_value.pop_back();
    res += combined_value + ";\n";
    res += tabbing(tabbing_iter) + "output_size++;\n";

    if (query->call_batch_update) {

      auto ordered_vars = enumerated_relations->find(query->query_name)->second;
      res +=
          tabbing(tabbing_iter) + "auto combined_entry = " + update_type + query->query_name + "_entry(" +
          join(&ordered_vars, ",") + ", combined_value);\n";
      res += tabbing(tabbing_iter) + "update.emplace_back(combined_entry);\n";

      res += tabbing(tabbing_iter) + "if (output_size % " + propagation_size + " == " + propagation_size +
             "-1) { enumeration_timer.stop(); enumeration_time += enumeration_timer.elapsedTimeInMilliSeconds();" +
             "propagation_timer.restart(); data.on_batch_update_"+query->query_name+"(update.begin(), update.end()); propagation_timer.stop();propagation_time += propagation_timer.elapsedTimeInMilliSeconds();\n";
      res += tabbing(tabbing_iter) + "enumeration_timer.restart();update.clear();}\n";
    } else {
      res +=
          tabbing(tabbing_iter) + "auto combined_entry = " + query->query_name + "_entry{" +
          join(&all_vars, ",") + ", combined_value};\n";
      res += tabbing(tabbing_iter) + "update.emplace_back(combined_entry);\n";
      res += tabbing(tabbing_iter) + "if (output_size % " + propagation_size + " == " + propagation_size +
             "-1) { update.clear();}\n";
    }

    res += tabbing(tabbing_iter) + "if (print_result) { output_file << " + join(&all_vars, " <<\"|\"<<") +
           " << combined_value << std::endl;}\n";

    std::string print_variable_order = "    std::cout << \"" + join(&all_vars, ",") + "\" << std::endl;\n";


    auto end_brackets = std::string(tabbing_iter, '}');
    res += end_brackets;
    res += "enumeration_timer.stop();\n";
    res += "enumeration_time += enumeration_timer.elapsedTimeInMilliSeconds();\n";
    res += "std::cout << \"enumeration time " + query->query_name +
           ": \" << enumeration_time << \"ms\"<< std::endl;\n";
    res += "measure->enumeration_time = enumeration_time;\n";
    if (query->call_batch_update) {
      res += "propagation_timer.restart();\n";
      res += "data.on_batch_update_" + query->query_name + "(update.begin(), update.end());\n";
      res += "propagation_timer.stop();\n";
      res += "propagation_time += propagation_timer.elapsedTimeInMilliSeconds();\n";
      res += "std::cout << \"propagation time " + query->query_name +
             ": \" << propagation_time << \"ms\" << std::endl;\n";
    } else {
      std::string entry = generate_struct(&all_vars, query->query_name);
      head = entry + head;
      res += "if (update.size() > 0) {\n";
      res += "std::uniform_int_distribution<int> uni(0,update.size()-1);\n";
      res += "volatile auto dummy = update.at(uni(rng));\n";
      res += "std::cout << \"dummy: \" << dummy.combined_value << std::endl;}\n";
    }
    res += "output_file.close();\n";
    res += "measure->size_output = output_size;\n";
    auto vars = join(&all_vars, ",");
    res += "measure->free_variables = \""+vars+"\";\n";

    res += "std::cout << \"" + query->query_name + ": \" << output_size << std::endl;\n";
    res += "return propagation_time;\n}\n";
    return head + print_variable_order + res;
  }


  std::string generate() {
    std::string res;
    std::string uppercase_filename = filename;
    std::transform(uppercase_filename.begin(), uppercase_filename.end(), uppercase_filename.begin(), toupper);
    std::string define_name = split(uppercase_filename, '-')->at(0);
    res += "#ifndef " + define_name + "_HPP\n";
    res += "#define " + define_name + "_HPP\n";
    res += "#include \"../src/basefiles/application.hpp\"\n";
    res += "#include <any>\n";
    res += "#include <random>\n";
    res += "std::random_device rd;\n";
    res += "std::mt19937 rng(rd());\n";
    res += generate_application();
    res += write_to_config;

    //iterate over queries and generate enumeration function for them
    for (auto query: *queries) {
      res += generate_function(query);
      res += "\n";
    }
    res += start;
    for (const auto &query: *query_atoms) {
      auto query_name = query.first;
      auto atoms = query.second;
      res += "    long updating_" + query_name + " = 0 + ";
      for (const auto &atom: atoms) {
        if(std::find(relations->begin(), relations->end(),atom) != relations->end()) {
          res += "dynamic_multiplexer.updating_times.find(\"" + atom + "\")->second + ";
        }
      }
      res.pop_back();
      res.pop_back();
      res += ";\n";
    }
    for (auto query: *queries) {
      res +=
          "    Measurement measure_" + query->query_name + " = Measurement(\"" + query->query_name + "\", updating_" +
          query->query_name + ", \"" + join(&(query_atoms->at(query->query_name)), ",") + "\");\n";
      res += "    cout << \"Enumerating " + query->query_name + "... \" << endl;\n";
      res +=
          "  long "+query->query_name+"_propagation = enumerate_" + query->query_name + "(data, print_result, count_view_size, &measure_" + query->query_name +
          ");\n";
      res += "    measurements.push_back(&measure_" + query->query_name + ");\n";
    }

    for (auto query_1: *queries) {
      int count = 0;
      for (auto query_2: *queries) {
        for (const auto& atom: query_atoms->at(query_2->query_name)) {
          if (atom == query_1->query_name){
            res += "    measure_" + query_2->query_name + ".update_time += " + query_1->query_name + "_propagation;\n";
            count++;
          }
        }
      }
      if(count > 1){
        res += "std::cout << \"WARNING: " + query_1->query_name + " is used in multiple queries but propagation time is indivisible. updating with "+query_1->query_name+" took\" << "+query_1->query_name + R"(_propagation" << "ms"<< std::endl;)";
      }
    }
    res += "std::ofstream measurements_file(\"output/output.txt\", std::ios::app | std::ios::out);\n";
    res += "write_to_config(&measurements, measurements_file, relation_list);\n";
    res += "measurements_file.close();\n";
    res += "}\n";
    res += " #endif";
    return res;
  }
};

int main(int argc, char **argv) {
  if (argc == 1) {
    std::cout << "No Config file given" << std::endl;
    return 1;
  }
  if (argc > 3) {
    std::cout << "Invalid Config file linked" << std::endl;
    return 1;
  }
  auto conf = new Config(argv[1]);
  auto res = conf->generate();
  if (argc == 3) {
    std::ofstream file(argv[2]);
    file << res;
    file.close();
  } else {
    std::cout << res;
  }
}