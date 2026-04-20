#pragma once
#include "types.hpp"
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

enum class Command { ADD, LIST, SHOW, DONE, DELETE, HELP, UNKNOWN };

struct ParsedArgs {
    Command command{Command::UNKNOWN};
    std::string title;
    std::string description;
    std::optional<int> priority;
    std::vector<std::string> tags;
    std::optional<int> task_id;
    std::optional<std::string> filter_tag;
    std::optional<TaskStatus> filter_status;
};

ParsedArgs parse_args(int argc, char* argv[]);
void handle_args(int argc, char* argv[], const std::filesystem::path& data_dir);
void print_help();
