//
// Created by avendel on 10.10.2019.
//

#pragma once

#include <shared_mutex>
#include <filesystem>
#include <set>

#define SOL_ALL_SAFETIES_ON 1
#define SOL_NO_CHECK_NUMBER_PRECISION 1
#include <sol/sol.hpp>

#include "FileWatch.h"
#include "interception.h"
#include "record.h"
#include "input_sender.h"

namespace script {

namespace fs = std::filesystem;

enum Event {
    KEY_PRESSED,
    KEY_RELEASED,
    MOUSE_BUTTON_PRESSED,
    MOUSE_BUTTON_RELEASED
};

class holder {
    fs::path m_script_path;
    const input_sender& isender_;
    sol::state m_state;
    sol::function m_on_event;
    std::unique_ptr<filewatch::FileWatch<std::filesystem::path>> m_notifier;
    std::function<void(const std::filesystem::path &path, const filewatch::Event change_type)> m_modify_handler;
    std::set<unsigned short> m_kstrokes;
    mutable std::shared_mutex m_script_mutex;
public:
    explicit holder(std::string_view , const input_sender& isender);

    void load_script(const std::filesystem::path &path);

    bool process_input(const InterceptionKeyStroke&);
    void process_log_record(const structs::Record& recort);

    template<typename T>
    sol::function_result apply(T&& v) const {
        std::shared_lock lock(m_script_mutex);
        return m_apply(std::forward<T>(v));
    }

private:
    void make_bindings();
    void make_log_binding();
    std::string to_string(const sol::stack_proxy& o) const;
    std::string fold_args(const sol::variadic_args& va) const;
};

}
