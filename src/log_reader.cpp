//
// Created by Arthur on 20/12/2020.
//

#include "log_reader.h"
#include "record.h"
#include "parser.h"

#include <iostream>
#include <thread>

namespace fs = std::filesystem;

log_reader::log_reader(std::filesystem::path combat_log_path, callback_t&& callback) : callback_(callback) {
    open(combat_log_path);
}

void log_reader::open(std::filesystem::path combat_log_path) {
    std::unique_lock lock(mutex_);
    combat_log_.open(combat_log_path, std::ios::in/* | std::ios::ate*/);
}

using namespace std::chrono_literals;

void log_reader::run(std::stop_token stop_token) {
    std::string line;
    int line_number{1};
    std::streamoff p = combat_log_.tellg();
    while (!stop_token.stop_requested()) {
        std::shared_lock lock(mutex_);
        if(combat_log_.seekg(p)) {
            while (getline(combat_log_, line)) {
                lock.unlock();
//                std::cout << "line number: " << line_number << std::endl;
//                std::cout << "line: " << line << std::endl;
                if (combat_log_.eof()) {
//                    std::cout << "line: " << line << std::endl;
                    spdlog::info("EOF");
                    break;
                }
                auto record = parse_ignoring<structs::Record>(line.begin(), line.end(), line_number);
    //            std::cout << record << std::endl;
                callback_(record);
                lock.lock();
                line_number++;
                p = combat_log_.tellg();
            }
            if (!combat_log_.eof()) {
                spdlog::info("not EOF");
                break;
            }
        }
        combat_log_.clear();
        std::this_thread::sleep_for(10ms);
        combat_log_.sync();
    }
}
