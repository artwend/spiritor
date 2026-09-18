//
// Created by Arthur on 20/12/2020.
//

#pragma once

#include <filesystem>
#include <fstream>
#include <functional>
#include <shared_mutex>
#include <stop_token>

namespace structs {
    class Record;
}

class log_reader {
public:
    using callback_t = std::function<void(const structs::Record&)>;

    explicit log_reader(std::filesystem::path combat_log_path, callback_t&&);
    void open(std::filesystem::path combat_log_path);
    void run(std::stop_token stop_token);

private:
    std::ifstream combat_log_;
    std::shared_mutex mutex_;
    callback_t callback_;
};
