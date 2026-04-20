#pragma once
#include "types.hpp"
#include <filesystem>
#include <optional>
#include <vector>

struct ListFilter {
    std::optional<std::string> tag;
    std::optional<int> priority;
    std::optional<TaskStatus> status;
};

class StorageClient {
public:
    explicit StorageClient(std::filesystem::path data_dir);

    int create_task(Task& task);
    std::optional<Task> get_task(int id);
    std::vector<Task> list_tasks(const ListFilter& filter = {});
    bool update_task(const Task& task);
    bool delete_task(int id);

private:
    std::filesystem::path data_dir_;
    int next_id();
    std::filesystem::path task_dir(int id) const;
    std::filesystem::path task_file(int id) const;
};
