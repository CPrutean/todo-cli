#include "arg_parser.hpp"
#include "query_client.hpp"
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace fs = std::filesystem;

void print_help() {
    std::cout
        << "Usage: todo <command> [options]\n\n"
        << "Commands:\n"
        << "  add <title> [options]   Create a new task\n"
        << "  list [options]          List tasks\n"
        << "  show <id>               Show task details\n"
        << "  done <id>               Mark task as done\n"
        << "  delete <id>             Delete a task\n"
        << "  help                    Show this help\n\n"
        << "Add options:\n"
        << "  --desc <text>           Task description\n"
        << "  --priority <0-5>        Priority level (default: 0)\n"
        << "  --tags <t1,t2,...>      Comma-separated tags\n\n"
        << "List options:\n"
        << "  --tag <name>            Filter by tag\n"
        << "  --priority <0-5>        Filter by priority\n"
        << "  --done                  Show only done tasks\n"
        << "  --pending               Show only pending tasks\n";
}

static std::vector<std::string> split_csv(const std::string& s) {
    std::vector<std::string> result;
    std::istringstream ss(s);
    std::string token;
    while (std::getline(ss, token, ','))
        if (!token.empty()) result.push_back(token);
    return result;
}

static std::string current_timestamp() {
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

ParsedArgs parse_args(int argc, char* argv[]) {
    ParsedArgs args;
    if (argc < 2) {
        args.command = Command::HELP;
        return args;
    }

    std::string cmd = argv[1];
    if      (cmd == "add")    args.command = Command::ADD;
    else if (cmd == "list")   args.command = Command::LIST;
    else if (cmd == "show")   args.command = Command::SHOW;
    else if (cmd == "done")   args.command = Command::DONE;
    else if (cmd == "delete") args.command = Command::DELETE;
    else if (cmd == "help")   args.command = Command::HELP;
    else                      args.command = Command::UNKNOWN;

    for (int i = 2; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--desc" && i + 1 < argc) {
            args.description = argv[++i];
        } else if (arg == "--priority" && i + 1 < argc) {
            try { args.priority = std::stoi(argv[++i]); } catch (...) {}
        } else if (arg == "--tags" && i + 1 < argc) {
            args.tags = split_csv(argv[++i]);
        } else if (arg == "--tag" && i + 1 < argc) {
            args.filter_tag = argv[++i];
        } else if (arg == "--done") {
            args.filter_status = TaskStatus::DONE;
        } else if (arg == "--pending") {
            args.filter_status = TaskStatus::PENDING;
        } else if (!arg.empty() && arg[0] != '-') {
            if (args.command == Command::ADD && args.title.empty()) {
                args.title = arg;
            } else if (args.command == Command::SHOW ||
                       args.command == Command::DONE ||
                       args.command == Command::DELETE) {
                try { args.task_id = std::stoi(arg); } catch (...) {}
            }
        }
    }
    return args;
}

void handle_args(int argc, char* argv[], const fs::path& data_dir) {
    ParsedArgs args = parse_args(argc, argv);
    StorageClient storage(data_dir);

    switch (args.command) {
        case Command::HELP:
        case Command::UNKNOWN:
            print_help();
            break;

        case Command::ADD: {
            if (args.title.empty()) {
                std::cerr << "Error: title is required\n"
                          << "Usage: todo add <title> [--desc ...] [--priority 0-5] [--tags t1,t2]\n";
                return;
            }
            Task t;
            t.title       = args.title;
            t.description = args.description;
            t.priority    = args.priority.value_or(0);
            t.created_at  = current_timestamp();
            for (auto& tag : args.tags) t.tags.push_back({tag});
            int id = storage.create_task(t);
            if (id > 0) std::cout << "Created task #" << id << ": " << t.title << "\n";
            break;
        }

        case Command::LIST: {
            ListFilter filter;
            if (args.filter_tag)    filter.tag      = args.filter_tag;
            if (args.priority)      filter.priority = args.priority;
            if (args.filter_status) filter.status   = args.filter_status;
            auto tasks = storage.list_tasks(filter);
            if (tasks.empty())
                std::cout << "No tasks found.\n";
            else
                for (auto& task : tasks) task.print_brief();
            break;
        }

        case Command::SHOW: {
            if (!args.task_id) { std::cerr << "Error: task ID required\n"; return; }
            auto task = storage.get_task(*args.task_id);
            if (!task) { std::cerr << "Error: task #" << *args.task_id << " not found\n"; return; }
            task->print_detail();
            break;
        }

        case Command::DONE: {
            if (!args.task_id) { std::cerr << "Error: task ID required\n"; return; }
            auto task = storage.get_task(*args.task_id);
            if (!task) { std::cerr << "Error: task #" << *args.task_id << " not found\n"; return; }
            task->status = TaskStatus::DONE;
            storage.update_task(*task);
            std::cout << "Marked task #" << *args.task_id << " as done\n";
            break;
        }

        case Command::DELETE: {
            if (!args.task_id) { std::cerr << "Error: task ID required\n"; return; }
            if (storage.delete_task(*args.task_id))
                std::cout << "Deleted task #" << *args.task_id << "\n";
            else
                std::cerr << "Error: task #" << *args.task_id << " not found\n";
            break;
        }
    }
}
