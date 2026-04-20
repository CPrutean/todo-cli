#include "types.hpp"
#include <iostream>
#include <sstream>

static std::string escape_json_str(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    for (char c : s) {
        switch (c) {
            case '"':  out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n";  break;
            case '\r': out += "\\r";  break;
            case '\t': out += "\\t";  break;
            default:   out += c;
        }
    }
    return out;
}

static std::string unescape_json_str(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    for (size_t i = 0; i < s.size(); i++) {
        if (s[i] == '\\' && i + 1 < s.size()) {
            switch (s[i + 1]) {
                case '"':  out += '"';  i++; break;
                case '\\': out += '\\'; i++; break;
                case 'n':  out += '\n'; i++; break;
                case 'r':  out += '\r'; i++; break;
                case 't':  out += '\t'; i++; break;
                default:   out += s[i];
            }
        } else {
            out += s[i];
        }
    }
    return out;
}

std::string Task::to_json() const {
    std::ostringstream ss;
    ss << "{\n";
    ss << "  \"id\": " << id << ",\n";
    ss << "  \"title\": \"" << escape_json_str(title) << "\",\n";
    ss << "  \"description\": \"" << escape_json_str(description) << "\",\n";
    ss << "  \"priority\": " << priority << ",\n";
    ss << "  \"status\": \"" << (status == TaskStatus::DONE ? "done" : "pending") << "\",\n";
    ss << "  \"created_at\": \"" << escape_json_str(created_at) << "\",\n";
    ss << "  \"tags\": [";
    for (size_t i = 0; i < tags.size(); ++i) {
        if (i > 0) ss << ", ";
        ss << "\"" << escape_json_str(tags[i].name) << "\"";
    }
    ss << "],\n";
    ss << "  \"resources\": [";
    for (size_t i = 0; i < resources.size(); ++i) {
        if (i > 0) ss << ", ";
        ss << "\"" << escape_json_str(resources[i]) << "\"";
    }
    ss << "]\n}";
    return ss.str();
}

static std::string get_string_value(const std::string& line) {
    size_t colon = line.find(':');
    if (colon == std::string::npos) return "";
    std::string after = line.substr(colon + 1);
    size_t start = after.find('"');
    if (start == std::string::npos) return "";
    size_t end = after.rfind('"');
    if (end == start) return "";
    return unescape_json_str(after.substr(start + 1, end - start - 1));
}

static int get_int_value(const std::string& line) {
    size_t colon = line.find(':');
    if (colon == std::string::npos) return 0;
    std::string after = line.substr(colon + 1);
    size_t i = after.find_first_of("0123456789");
    if (i == std::string::npos) return 0;
    try { return std::stoi(after.substr(i)); }
    catch (...) { return 0; }
}

static std::vector<std::string> get_string_array(const std::string& line) {
    std::vector<std::string> result;
    size_t start = line.find('[');
    size_t end = line.rfind(']');
    if (start == std::string::npos || end == std::string::npos || start >= end) return result;
    std::string inside = line.substr(start + 1, end - start - 1);
    size_t pos = 0;
    while ((pos = inside.find('"', pos)) != std::string::npos) {
        size_t close = inside.find('"', pos + 1);
        while (close != std::string::npos && inside[close - 1] == '\\')
            close = inside.find('"', close + 1);
        if (close == std::string::npos) break;
        result.push_back(unescape_json_str(inside.substr(pos + 1, close - pos - 1)));
        pos = close + 1;
    }
    return result;
}

Task Task::from_json(const std::string& data) {
    Task t;
    std::istringstream ss(data);
    std::string line;
    while (std::getline(ss, line)) {
        if (line.find("\"id\"") != std::string::npos)
            t.id = get_int_value(line);
        else if (line.find("\"title\"") != std::string::npos)
            t.title = get_string_value(line);
        else if (line.find("\"description\"") != std::string::npos)
            t.description = get_string_value(line);
        else if (line.find("\"priority\"") != std::string::npos)
            t.priority = get_int_value(line);
        else if (line.find("\"status\"") != std::string::npos)
            t.status = (get_string_value(line) == "done") ? TaskStatus::DONE : TaskStatus::PENDING;
        else if (line.find("\"created_at\"") != std::string::npos)
            t.created_at = get_string_value(line);
        else if (line.find("\"tags\"") != std::string::npos) {
            for (auto& name : get_string_array(line)) t.tags.push_back({name});
        } else if (line.find("\"resources\"") != std::string::npos) {
            t.resources = get_string_array(line);
        }
    }
    return t;
}

void Task::print_brief() const {
    std::string mark = (status == TaskStatus::DONE) ? "[x]" : "[ ]";
    std::cout << mark << " #" << id << " [P" << priority << "] " << title;
    if (!tags.empty()) {
        std::cout << " (";
        for (size_t i = 0; i < tags.size(); ++i) {
            if (i > 0) std::cout << ", ";
            std::cout << tags[i].name;
        }
        std::cout << ")";
    }
    std::cout << "\n";
}

void Task::print_detail() const {
    std::cout << "ID:          " << id << "\n"
              << "Title:       " << title << "\n"
              << "Description: " << description << "\n"
              << "Priority:    " << priority << "\n"
              << "Status:      " << (status == TaskStatus::DONE ? "Done" : "Pending") << "\n"
              << "Created:     " << created_at << "\n";
    if (!tags.empty()) {
        std::cout << "Tags:        ";
        for (size_t i = 0; i < tags.size(); ++i) {
            if (i > 0) std::cout << ", ";
            std::cout << tags[i].name;
        }
        std::cout << "\n";
    }
    if (!resources.empty()) {
        std::cout << "Resources:\n";
        for (auto& r : resources) std::cout << "  - " << r << "\n";
    }
}
