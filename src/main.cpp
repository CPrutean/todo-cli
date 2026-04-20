#include <cstdlib>
#include <filesystem>
#include <fstream>

#include "arg_parser.hpp"

namespace fs = std::filesystem;

static fs::path get_home_dir() {
#ifdef _WIN32
    const char* home = std::getenv("USERPROFILE");
#else
    const char* home = std::getenv("HOME");
#endif
    return home ? fs::path(home) : fs::current_path();
}

static fs::path bootstrap_config() {
    fs::path home_dir   = get_home_dir();
    fs::path config_dir = home_dir / ".config" / "todo";
    fs::path data_dir   = home_dir / "todo-data";
    fs::path config_file = config_dir / "config.json";

    if (!fs::exists(config_dir)) fs::create_directories(config_dir);
    if (!fs::exists(data_dir))   fs::create_directories(data_dir);

    if (!fs::exists(config_file)) {
        std::ofstream out(config_file);
        out << "{\n  \"data_dir\": \"" << data_dir.generic_string() << "\"\n}\n";
    } else {
        std::ifstream f(config_file);
        std::string line;
        while (std::getline(f, line)) {
            if (line.find("\"data_dir\"") == std::string::npos) continue;
            size_t first = line.find('"', line.find(':'));
            size_t last  = line.rfind('"');
            if (first != std::string::npos && last != first) {
                std::string path = line.substr(first + 1, last - first - 1);
                if (!path.empty()) data_dir = fs::path(path);
            }
            break;
        }
    }

    return data_dir;
}

int main(int argc, char* argv[]) {
    fs::path data_dir = bootstrap_config();
    handle_args(argc, argv, data_dir);
    return 0;
}
