#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

namespace fs = std::filesystem;
using std::cout, std::cin, std::cerr;

namespace config {
const char *data_dir{nullptr};
}

fs::path get_home_dir() {
#ifdef _WIN32
#else
  const char *home = std::getenv("HOME");
#endif
  return home ? fs::path(home) : fs::current_path();
}

void parse_config(fs::path &config) {
  std::fstream file(config);
  if (!file.is_open()) {
    cerr << "Failed to open config file\n";
    exit(1);
  }
  std::stringstream buff;
  buff << file.rdbuf();
  std::string str = buff.str();
  size_t mid{str.find(":")};

  if (mid == std::string::npos) {
    cerr << "Failed to find valid json \n";
    exit(1);
  }

  int t1_left, t1_right;
  int t2_left, t2_right;

  size_t s{str.find_first_of("\"")};
  int count{0};
  while (s < str.find_last_of("\"")) {
    if (str.at(s) == '\"' && count == 0) {
      t1_left = str.find_first_of("\"");
      t1_right = s;
      count++;
    } else if (str.at(s) == '\"') {
      t2_left = s;
      t2_right = str.find_last_of("\"");
      break;
    }
  }

  char *t2 = new char[t2_right - t2_left + 1];
  snprintf(t2, t2_right - t2_left, "%s", (str.c_str() + t2_left + 1));
  config::data_dir = t2;
  cout << t2 << "\n";

  file.close();
}

void bootstrap_config() {
  fs::path home_dir = get_home_dir();
  fs::path config_dir = home_dir / ".config" / "todo";
  fs::path data_dir = home_dir / "todo-data";
  fs::path config_file = config_dir / "config.json";

  if (!fs::exists(config_dir)) {
    fs::create_directories(config_dir);
  }
  if (!fs::exists(data_dir)) {
    fs::create_directories(data_dir);
  }

  if (!fs::exists(config_file)) {
    std::cout << "Config not found. Creating default at: " << config_file
              << '\n';
    std::ofstream out(config_file);
    out << "{\n  \"data_dir\": \"" << data_dir.generic_string() << "\"\n}\n";
  } else {
    std::cout << "Found config file, parsing...\n";
    parse_config(config_file);
  }
}

int main() {
  bootstrap_config();
  return 0;
}
