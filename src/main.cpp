#include <boost/chrono/chrono.hpp>
#include <iomanip>
#include <fstream>
#include <filesystem>
#include <ctime>
#include <tchar.h>
#include <thread>
#include <queue>

#include <spdlog/spdlog.h>

#include "date.h"
#include "KeyEvents.h"
#include "FileWatch.h"
#include "log_reader.h"
#include "script_holder.h"
#include "input_sender.h"
#include "interception.h"
#include "record.h"
#include "parser.h"

enum ScanCode
{
    SCANCODE_X   = 0x2D,
    SCANCODE_Y   = 0x15,
    SCANCODE_ESC = 0x01
};

using namespace std::string_literals;

const std::string input = R"([01:09:17.454] [@Vhaerlein] [@Vhaerlein] [] [Event {836045448945472}: EnterCombat {836045448945489}] ()
[01:09:25.388] [@Vhaerlein] [@Vhaerlein] [Force Leap {812105301229568}] [Event {836045448945472}: AbilityActivate {836045448945479}] ()
[01:09:26.022] [@Vhaerlein] [Eternal Warden {1784013450641408}:13604008676603] [Force Leap {812105301229568}] [ApplyEffect {836045448945477}: Damage {836045448945501}] (2833* energy {836045448940874}) <2125>
[01:09:26.828] [@Vhaerlein] [@Vhaerlein] [Zealous Strike {996200484438016}] [Event {836045448945472}: AbilityActivate {836045448945479}] ()
[01:09:27.334] [@Vhaerlein] [Eternal Warden {1784013450641408}:13604008676603] [Zealous Strike {996200484438016}] [ApplyEffect {836045448945477}: Damage {836045448945501}] (1558* energy {836045448940874}) <1168>
[01:09:27.462] [@Vhaerlein] [Eternal Warden {1784013450641408}:13604008676603] [Zealous Strike {996200484438016}] [ApplyEffect {836045448945477}: Damage {836045448945501}] (375* energy {836045448940874}) <281>
[01:09:29.007] [@Vhaerlein] [Eternal Warden {1784013450641408}:13604008676603] [Burning Slices {3630205142827008}] [ApplyEffect {836045448945477}: Burning Slices {3630205142827008}] ()
[01:09:29.008] [@Vhaerlein] [Eternal Warden {1784013450641408}:13604008676603] [Blade Dance {3619557918900224}] [ApplyEffect {836045448945477}: Damage {836045448945501}] (0 -miss {836045448945502}) <1>
[01:09:29.008] [@Vhaerlein] [Eternal Warden {1784013450641408}:13604008676603] [Blade Dance {3619557918900224}] [ApplyEffect {836045448945477}: Damage {836045448945501}] (2692 energy {836045448940874} -shield {836045448945509}) <2019>
[01:09:30.737] [@Vhaerlein] [Eternal Warden {1784013450641408}:13604008676603] [Blade Dance {3619557918900224}] [ApplyEffect {836045448945477}: Damage {836045448945501}] (8228 energy {836045448940874}) <6171>
[01:09:30.737] [@Vhaerlein] [Eternal Warden {1784013450641408}:13604008676603] [Blade Dance {3619557918900224}] [ApplyEffect {836045448945477}: Damage {836045448945501}] (0 -miss {836045448945502}) <1>
[01:09:30.647] [@H?ly] [@Vhaerlein] [Successive Treatment {3393470840438784}] [ApplyEffect {836045448945477}: Heal {836045448945500}] (6068*) <1900>
)";
//[01:09:31.679] [@Vhaerlein] [@Vhaerlein] [Precision {2209167968305152}] [RemoveEffect {836045448945478}: Precision {2209167968305152}] ()
//[01:09:32.742] [@Vhaerlein] [Eternal Warden {1784013450641408}:13604008676603] [Dispatch {3461554662014976}] [ApplyEffect {836045448945477}: Damage {836045448945501}] (6320 energy {836045448940874}) <4740>
//[01:09:59.676] [@Vhaerlein] [@Vhaerlein] [] [Event {836045448945472}: ExitCombat {836045448945490}] ()
//[01:12:11.209] [@Vhaerlein] [Escaped Infernal Thrall {2018269556899840}:13604009668170] [] [Event {836045448945472}: Death {836045448945493}] ()
//[00:56:54.579] [Annihilation Droid XRR-3 {2034573252755456}:13604008912435] [@Vhaerlein] [Missile Salvo {2508441289490432}] [ApplyEffect {836045448945477}: Damage {836045448945501}] (34085 energy {836045448940874}) <34085>
//[00:56:54.579] [Annihilation Droid XRR-3 {2034573252755456}:13604008912435] [@Vhaerlein] [] [Event {836045448945472}: Death {836045448945493}] ()
//[00:57:10.666] [@Vhaerlein] [@Vhaerlein] [] [Event {836045448945472}: Revived {836045448945494}] ()
//[01:31:27.209] [@Vhaerlein] [Proto Acklay {2021018335969280}:13604008684537] [Focused Slash {3478012976693248}] [ApplyEffect {836045448945477}: Beat Down (Physical) {3478012976693516}] ()
//[01:38:18.469] [@Vhaerlein] [@Vhaerlein] [Rebuke {2480816059842560}] [Event {836045448945472}: AbilityActivate {836045448945479}] ()
//[01:38:18.470] [@Vhaerlein] [@Vhaerlein] [Rebuke {2480816059842560}] [ApplyEffect {836045448945477}: Rebuke {2480816059842560}] ()
//[00:51:27.032] [@Vhaerlein] [@Vhaerlein] [Strike {947362411315200}] [Event {836045448945472}: AbilityActivate {836045448945479}] ()
//[00:51:27.033] [@Vhaerlein] [Annihilation Droid XRR-3 {2034573252755456}:13604008912435] [Strike {947362411315200}] [ApplyEffect {836045448945477}: Damage {836045448945501}] (709 energy {836045448940874}) <709>
//)";

HWND g_HWND;

struct FindWindowParam {
    std::string partial_name;
    HWND windows_handler;
};

BOOL CALLBACK FindWindowPartial(HWND hwnd, LPARAM partial_string) {
    const size_t title_size = 1024;
    TCHAR window_title[title_size];
    if(GetWindowText(hwnd, window_title, title_size)) {
        if(_tcsstr(window_title, reinterpret_cast<const char *>(partial_string)) != nullptr) {
            g_HWND = hwnd;
            return false;
        }
    }
    return true;
}

namespace fs = std::filesystem;

fs::directory_entry get_last_combat_log(fs::path& path) {
    std::priority_queue<fs::directory_entry> q;
    for(auto& p: fs::directory_iterator(path)) {
        q.push(p);
//        std::cout << p.path() << '\n';
    }
    return q.top();
}

void raise_process_priority(void)
{
    SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
}

volatile bool isRunnung = true;

BOOL WINAPI HandlerRoutine(_In_ DWORD dwCtrlType) {
    switch (dwCtrlType)
    {
        case CTRL_C_EVENT:
            printf("[Ctrl]+C\n");
            isRunnung = false;
            // Signal is handled - don't pass it on to the next handler
            return TRUE;
        default:
            // Pass signal on to the next handler
            return FALSE;
    }
}

int main() try {
    SetConsoleCtrlHandler(HandlerRoutine, TRUE);

    EnumWindows(FindWindowPartial, reinterpret_cast<LPARAM>(TEXT("Star Wars")));

//    if(!SetForegroundWindow(g_HWND)) {
//        std::cout << "windows not found" << std::endl;
//        return 0;
//    }
//
//    SetActiveWindow(g_HWND);
//
//    Sleep(2000);

//    SendMessage(g_HWND, WM_SYSKEYDOWN, VK_MENU/*VkKeyScanA('w')*/, 0);
//    Sleep(10);
//    SendMessage(g_HWND, WM_SYSKEYUP, VK_MENU/*VkKeyScanA('w')*/, 0);
//    Sleep(500);

//    for (int i = 0; i > 50; i++)
//    {
//        SendMessage(g_HWND, WM_KEYDOWN, VK_SPACE/*VkKeyScanA('w')*/, 0);
//        Sleep(10);
//        SendMessage(g_HWND, WM_KEYUP, VK_SPACE/*VkKeyScanA('w')*/, 0);
//        Sleep(500);
//    }

//    PostMessage(g_HWND, WM_KEYDOWN, VK_ESCAPE, 0);
//    Sleep(15);
//    PostMessage(g_HWND, WM_KEYUP, VK_ESCAPE, 0);
//    Sleep(15);
//    PostMessage(g_HWND, WM_KEYDOWN, VK_RETURN, 0);
//    Sleep(15);
//    PostMessage(g_HWND, WM_KEYUP, VK_RETURN, 0);

    raise_process_priority();

    InterceptionContext context = interception_create_context();
    InterceptionDevice device{};

    input_sender isender(context, device);

    auto path_to_script = R"(C:\Users\Arthur\projects\spiritor\script\swtor.lua)";
    script::holder script_holder(path_to_script, isender);

    namespace fs = std::filesystem;
    fs::path combat_log_dir(R"(C:\Users\Arthur\Documents\Star Wars - The Old Republic\CombatLogs)");

    auto last_log = get_last_combat_log(combat_log_dir);
    std::cout << "last_log: " << last_log.path() << std::endl;
    log_reader reader(last_log.path(), [&script_holder](const auto& record){
        script_holder.process_log_record(record);
    });
    std::jthread log_read_thread([&reader](std::stop_token stop_token){
        reader.run(stop_token);
    });

    filewatch::FileWatch<std::filesystem::path> watch(
            combat_log_dir,
            [&reader, &combat_log_dir](const std::filesystem::path &path, const filewatch::Event change_type) {
                std::cout << "[FileWatch callback]" << std::filesystem::absolute(path) << std::endl;
                if (change_type == filewatch::Event::added) {
                    std::cout << "The file was added to the directory." << std::endl;
                    reader.open(combat_log_dir / path);
                    spdlog::info("Log file {} opened", path);
                }
            }
    );

//    log_read_thread.join();
//
//    return 0;

    InterceptionKeyStroke stroke;

    interception_set_filter(context, interception_is_keyboard,
                            INTERCEPTION_FILTER_KEY_DOWN | INTERCEPTION_FILTER_KEY_UP | INTERCEPTION_FILTER_KEY_E0);

    while(isRunnung && interception_receive(context, device = interception_wait(context), (InterceptionStroke *)&stroke, 1) > 0)
    {
//        if(stroke.code == SCANCODE_X) stroke.code = SCANCODE_Y;

        if (script_holder.process_input(stroke))
            continue;

        interception_send(context, device, (InterceptionStroke *)&stroke, 1);

//        interception_send(context, device,
//                          (InterceptionStroke *)&kstrokes[0],
//                          kstrokes.size());

//        if(stroke.code == SCANCODE_ESC) break;
    }

    interception_destroy_context(context);

//    CKeyEvents::GetSingleton()->Hook([&script_holder](bool pressed, int code){
//        if (pressed) {
//            return script_holder.process_input(pressed, code);
//        }
//        else {
//            return script_holder.process_input(pressed, code);
//        }
//    });
//
//    MSG msg{};
//    while (GetMessage(&msg, nullptr, 0, 0) != 0);
//
//    CKeyEvents::GetSingleton()->Unhook();

    spdlog::info("exit");

    return 0;
}
catch (std::exception& ex) {
    spdlog::error(ex.what());
}
