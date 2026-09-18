//
// Created by avendel on 10.10.2019.
//

#include "script_holder.h"
#include "scancodes.h"

#include <functional>
#include <numeric>
#include <filesystem>
#include <fmt/printf.h>
//#include <range/v3/all.hpp>
#include <fmt/ostream.h>
#include <spdlog/spdlog.h>

namespace script {

namespace fs = std::filesystem;

holder::holder(std::string_view path, const input_sender &isender)
        : m_script_path(path),
          isender_(isender)
{
    spdlog::debug("holder ctor");
    m_state.open_libraries(sol::lib::base, sol::lib::coroutine, sol::lib::debug, sol::lib::io, sol::lib::math,
                           sol::lib::os, sol::lib::package, sol::lib::string, sol::lib::table);
    const std::string package_path = m_state["package"]["path"];
    m_state["package"]["path"] = package_path + ";" + (m_script_path.parent_path() / "?.lua").string();

    make_bindings();
    make_log_binding();

    load_script(m_script_path);

    m_modify_handler = [this](const std::filesystem::path &path, const filewatch::Event change_type) {
        try {
            if (change_type == filewatch::Event::modified) {
                spdlog::debug("Event {} on {} was triggered.",
                           static_cast<int>(change_type),
                           path);
                spdlog::info("reload script from {}", m_script_path);
                load_script(m_script_path);
            }
        }
        catch (std::exception& ex) {
            spdlog::error(ex.what());
        }
    };

    m_notifier = std::make_unique<filewatch::FileWatch<std::filesystem::path>>(
            path,
            m_modify_handler
            );
}

void holder::load_script(const std::filesystem::path &path) {
    std::unique_lock lock(m_script_mutex);
    auto res = m_state.script_file(path.string());
    if (!res.valid()) {
        sol::error err = res;
        spdlog::error(err.what());
        return;
    }
    m_on_event = m_state["OnEvent"];
    lock.unlock();
    if (!m_on_event.valid())
        spdlog::error("OnEvent is missing");

    sol::table loaded_packages = m_state["package"]["loaded"];
    std::string loaded_packages_str;
    for (const auto& [k, v] : loaded_packages) {
        loaded_packages_str += k.as<std::string>() + ", ";
    }
    if (!loaded_packages_str.empty())
        loaded_packages_str.erase(loaded_packages_str.end()-2);

//    spdlog::info("script {} loaded with uploaded packages:\n{}", path, loaded_packages_str);
}

std::string holder::to_string(const sol::stack_proxy& o) const {
    return m_state["tostring"](o);
}

std::string holder::fold_args(const sol::variadic_args& va) const {
    std::string args_str;
    for (const auto& a : va) {
        args_str += to_string(a) + ' ';
    }
    //remove last space char
    if (!args_str.empty())
        args_str.erase(std::prev(args_str.end()));
    return args_str;
}

template<class Fn, class... Args, std::size_t... Idxs>
decltype(auto) sol_call_impl(Fn&& fun, const sol::variadic_args& sol_args, std::index_sequence<Idxs...>)
{
//    return std::invoke(std::forward<Fn>(fun), (sol_args[Idxs].as<Args>())...);
    return [&fun](auto a, auto... b) { return fun(a, b...); }((sol_args[Idxs].as<Args>())...);
}

template<class Fn, class... Args>
decltype(auto) sol_call(Fn&& fun, const sol::variadic_args& sol_args)
{
    return sol_call_impl<Fn, Args...>(std::forward<Fn>(fun), sol_args, std::make_index_sequence<sizeof...(Args)>());
}

void holder::make_bindings() {
//    sol::table table(m_state, sol::create);

    m_state.new_enum("Event",
                 "KEY_PRESSED", Event::KEY_PRESSED,
                 "KEY_RELEASED", Event::KEY_RELEASED,
                 "MOUSE_BUTTON_PRESSED", Event::MOUSE_BUTTON_PRESSED,
                 "MOUSE_BUTTON_RELEASED", Event::MOUSE_BUTTON_RELEASED
    );

    m_state.set_function("Sleep", [](int ms) {
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
    });

    m_state.set_function("PressKey", sol::overload(
            [this](unsigned short code) {
                isender_.press_key(code);
//                INPUT input;
//                input.type = INPUT_KEYBOARD;
//                input.ki.wScan = code;
//                input.ki.wVk = 0;
//                input.ki.dwFlags = KEYEVENTF_SCANCODE;
//                input.ki.time = 0;
//                input.ki.dwExtraInfo = 0;
//                SendInput(1, &input, sizeof(INPUT));
//                SendMessage(HWND_BROADCAST, WM_KEYDOWN, MapVirtualKeyA(code, MAPVK_VSC_TO_VK_EX), 0);
            },
            [this](const char* c) {
                isender_.press_key(ascii_to_scan_code_table[c]);
//                assert(std::isalpha(c) || isdigit(c));
//                m_key_strokes.push_back({ascii_to_scan_code_table[c], INTERCEPTION_KEY_DOWN, 0});
            }
    ) );

    m_state.set_function("ReleaseKey", sol::overload(
            [this](unsigned short code) {
                isender_.release_key(code);
//                m_key_strokes.push_back({code, INTERCEPTION_KEY_UP, 0});
//                INPUT input;
//                input.type = INPUT_KEYBOARD;
//                input.ki.wScan = code;
//                input.ki.wVk = 0;
//                input.ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;
//                input.ki.time = 0;
//                input.ki.dwExtraInfo = 0;
//                SendInput(1, &input, sizeof(INPUT));
//                SendMessage(HWND_BROADCAST, WM_KEYUP, VK_ESCAPE, 0);
            },
            [this](const char* c) {
                isender_.release_key(ascii_to_scan_code_table[c]);
//                m_key_strokes.push_back({ascii_to_scan_code_table[c], INTERCEPTION_KEY_UP, 0});
            }
    ) );

    m_state.set_function("PressAndReleaseKey", sol::overload(
            [this](unsigned short code) {
                isender_.press_adn_release_key(code);
            },
            [this](const char* c) {
                isender_.press_adn_release_key(ascii_to_scan_code_table[c]);
            }
    ));

    const auto is_key_pressed = [this](const std::string& key){
        return m_kstrokes.contains(ascii_to_scan_code_table[key]);
    };

    m_state.set_function("IsKeyPressed", is_key_pressed);

    m_state.set_function("IsModifierPressed", [is_key_pressed](const std::string& modifer){
        if (modifer == "alt")
            return is_key_pressed("lalt") || is_key_pressed("ralt");
        if (modifer == "ctrl")
            return is_key_pressed("lctrl") || is_key_pressed("rctrl");
        if (modifer == "shift")
            return is_key_pressed("lshift") || is_key_pressed("rshift");

        return is_key_pressed(modifer);
    });

    m_state.set_function("Suppress", [this](unsigned short code){
        m_state["suppress"] = true;
    });

    m_state.set_function("OutputDebugMessage", [this](sol::variadic_args va) {
        std::string str = m_state["string"]["format"](va);
        spdlog::info(str);
    });
//    m_state["log"] = table;
}

InterceptionKeyStroke lctrl_down    = {0x1D, INTERCEPTION_KEY_DOWN};
InterceptionKeyStroke rctrl_down    = {0x1D, INTERCEPTION_KEY_DOWN | INTERCEPTION_KEY_E0};
InterceptionKeyStroke lalt_down     = {0x38, INTERCEPTION_KEY_DOWN};
InterceptionKeyStroke ralt_down     = {0x38, INTERCEPTION_KEY_DOWN | INTERCEPTION_KEY_E0};
InterceptionKeyStroke lshift_down   = {0x2a, INTERCEPTION_KEY_DOWN};
InterceptionKeyStroke rshift_down   = {0x36, INTERCEPTION_KEY_DOWN};
InterceptionKeyStroke lctrl_up  = {0x1D, INTERCEPTION_KEY_UP};
InterceptionKeyStroke rctrl_up  = {0x1D, INTERCEPTION_KEY_UP | INTERCEPTION_KEY_E0};
InterceptionKeyStroke lalt_up   = {0x38, INTERCEPTION_KEY_UP};
InterceptionKeyStroke ralt_up   = {0x38, INTERCEPTION_KEY_UP | INTERCEPTION_KEY_E0};
InterceptionKeyStroke lshift_up = {0x2a, INTERCEPTION_KEY_UP};
InterceptionKeyStroke rshift_up = {0x36, INTERCEPTION_KEY_UP};

bool operator == (const InterceptionKeyStroke &first, const InterceptionKeyStroke &second)
{
    return first.code == second.code && first.state == second.state;
}

bool operator != (const InterceptionKeyStroke &first, const InterceptionKeyStroke &second)
{
    return !(first == second);
}

bool holder::process_input(const InterceptionKeyStroke& stroke) try {
    spdlog::info("key {}", stroke.code);

    if (stroke.state == INTERCEPTION_KEY_DOWN)
        m_kstrokes.insert(stroke.code);
    else if (stroke.state == INTERCEPTION_KEY_UP)
        m_kstrokes.extract(stroke.code);

//    spdlog::info("process_input");
//    if (stroke.code == 0x1D || stroke.code == 0x11d
//        || stroke.code == 0x38 || stroke.code == 0x138
//        || stroke.code == 0x2a || stroke.code == 0x36) {
//        bool is_pressed = stroke.state == INTERCEPTION_KEY_DOWN;
//        spdlog::info("modifer {} is {}", stroke.code, is_pressed ? "pressed":"released");
//        switch (stroke.code) {
//            case 0x1D:
//                m_modifers["lctrl"] = is_pressed;
//                break;
//            case 0x11d:
//                m_modifers["rctrl"] = is_pressed;
//                break;
//            case 0x38:
//                m_modifers["lalt"] = is_pressed;
//                break;
//            case 0x138:
//                m_modifers["ralt"] = is_pressed;
//                break;
//            case 0x2a:
//                m_modifers["lshift"] = is_pressed;
//                break;
//            case 0x36:
//                m_modifers["rshift"] = is_pressed;
//                break;
//        }
//        return false;
//    }

    std::shared_lock lock(m_script_mutex);
    m_state["suppress"] = false;
    auto res = m_on_event(stroke.state ? "KEY_RELEASED" : "KEY_PRESSED",
                          stroke.code);

    return m_state["suppress"];
}
catch (std::exception& ex) {
    spdlog::error(ex.what());
    return false;
}

void holder::process_log_record(const structs::Record& record) {
    std::shared_lock lock(m_script_mutex);
    auto res = m_on_event("LOG", record);
    lock.unlock();
    if (!res.valid()) {
        sol::error err = res;
        spdlog::error(err.what());
    }
}

void holder::make_log_binding() {

    m_state.new_usertype<structs::Character>("Character",
                                          "is_player", &structs::Character::is_player,
                                          "name", &structs::Character::name
    );

    m_state.new_usertype<structs::Timestamp>("Timestamp",
                                             "time_of_day", &structs::Timestamp::time_of_day
    );

    m_state.new_usertype<structs::Ability>("Ability",
                                             "name", &structs::Ability::name,
                                             "id", &structs::Ability::id
    );

    m_state.new_usertype<structs::NameIdPair>("NameIdPair",
                                           "name", &structs::NameIdPair::name,
                                           "id", &structs::NameIdPair::id
    );

    m_state.new_usertype<structs::Amount>("Amount",
                                              "value", &structs::Amount::value,
                                              "is_critical", &structs::Amount::is_critical
    );

    m_state.new_usertype<structs::Record>("Record",
                        "time", sol::property(
                            [](const structs::Record &obj) {
//                                return system_clock::to_time_t(time::parse8601(obj.time_device));
                            },
                            [](structs::Record &obj, std::time_t time) {
//                                obj.time_device = time::to_json(system_clock::from_time_t(time));
                            }),
                        "who", &structs::Record::who,
                        "target", &structs::Record::target,
                        "ability", &structs::Record::ability,
                        "type", &structs::Record::type,
                        "action", &structs::Record::action,
                        "amount", &structs::Record::amount

    );
}

}
