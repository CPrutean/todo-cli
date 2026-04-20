#pragma once
#include <string>
#include <vector>

enum class TaskStatus { PENDING, DONE };

struct Tag {
    std::string name;
};

struct Task {
    int id{0};
    std::string title;
    std::string description;
    int priority{0};  // 0 = lowest, 5 = highest
    std::vector<Tag> tags;
    TaskStatus status{TaskStatus::PENDING};
    std::string created_at;
    std::vector<std::string> resources;

    std::string to_json() const;
    static Task from_json(const std::string& data);
    void print_detail() const;
    void print_brief() const;
};
