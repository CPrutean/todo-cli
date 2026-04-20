#include "query_client.hpp"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>

namespace fs = std::filesystem;

StorageClient::StorageClient(fs::path data_dir) : data_dir_(std::move(data_dir)) {
    if (!fs::exists(data_dir_))
        fs::create_directories(data_dir_);
}

fs::path StorageClient::task_dir(int id) const {
    return data_dir_ / std::to_string(id);
}

fs::path StorageClient::task_file(int id) const {
    return task_dir(id) / "task.json";
}

int StorageClient::next_id() {
    fs::path id_file = data_dir_ / "next_id.txt";
    int id = 1;
    if (fs::exists(id_file)) {
        std::ifstream f(id_file);
        f >> id;
    }
    std::ofstream f(id_file);
    f << (id + 1);
    return id;
}

int StorageClient::create_task(Task& task) {
    task.id = next_id();
    fs::create_directories(task_dir(task.id));
    std::ofstream f(task_file(task.id));
    if (!f.is_open()) {
        std::cerr << "Error: could not create task file\n";
        return -1;
    }
    f << task.to_json();
    return task.id;
}

std::optional<Task> StorageClient::get_task(int id) {
    fs::path file = task_file(id);
    if (!fs::exists(file)) return std::nullopt;
    std::ifstream f(file);
    if (!f.is_open()) return std::nullopt;
    std::ostringstream ss;
    ss << f.rdbuf();
    return Task::from_json(ss.str());
}

std::vector<Task> StorageClient::list_tasks(const ListFilter& filter) {
    std::vector<Task> tasks;
    if (!fs::exists(data_dir_)) return tasks;

    for (auto& entry : fs::directory_iterator(data_dir_)) {
        if (!entry.is_directory()) continue;
        fs::path file = entry.path() / "task.json";
        if (!fs::exists(file)) continue;
        std::ifstream f(file);
        if (!f.is_open()) continue;
        std::ostringstream ss;
        ss << f.rdbuf();
        Task t = Task::from_json(ss.str());

        if (filter.priority && t.priority != *filter.priority) continue;
        if (filter.status && t.status != *filter.status) continue;
        if (filter.tag) {
            bool found = false;
            for (auto& tg : t.tags)
                if (tg.name == *filter.tag) { found = true; break; }
            if (!found) continue;
        }
        tasks.push_back(std::move(t));
    }

    std::sort(tasks.begin(), tasks.end(),
              [](const Task& a, const Task& b) { return a.id < b.id; });
    return tasks;
}

bool StorageClient::update_task(const Task& task) {
    if (!fs::exists(task_file(task.id))) return false;
    std::ofstream f(task_file(task.id));
    if (!f.is_open()) return false;
    f << task.to_json();
    return true;
}

bool StorageClient::delete_task(int id) {
    fs::path dir = task_dir(id);
    if (!fs::exists(dir)) return false;
    fs::remove_all(dir);
    return true;
}
