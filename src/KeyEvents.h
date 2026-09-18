//
// Created by Arthur on 16/12/2020.
//

#pragma once
#pragma warning(disable:4334)
#pragma warning(disable:4244)
#pragma warning(disable:4800)

#include <Windows.h>
#include <stdint.h>
#include <functional>
#include <spdlog/spdlog.h>

class CKeyEvents
{
public:
    using callback_t = std::function<bool(bool, int)>;
private:
    HHOOK hhookObject;

    uint64_t Segments [ 4 ];

    callback_t callback_;

    void SetBitToZero( uint64_t &Segment, uint8_t Shifting ) const { Segment &= ~( 1 << Shifting ); }

    void SetBitToOne( uint64_t &Segment, uint8_t Shifting ) const { Segment |= 1 << Shifting; }

    uint64_t GetBitFromSegment( uint64_t &Segment, uint8_t Shifting ) const { return Segment & 1 << Shifting; }

    void SetKeyState( uint8_t Key, uint8_t State )
    {
        uint8_t SegmentID = Key / 64;

        uint8_t Shifting = Key - SegmentID * 64;

        !State ? SetBitToZero( Segments[ SegmentID ], Shifting ) : SetBitToOne( Segments [ SegmentID ], Shifting );
    }

    static LRESULT CALLBACK KeyboardHook( int nCode, WPARAM wParam, LPARAM lParam )
    {
        if ( nCode < HC_ACTION ) { return CallNextHookEx( nullptr, nCode, wParam, lParam ); }

        PKBDLLHOOKSTRUCT Events = reinterpret_cast< PKBDLLHOOKSTRUCT >( lParam );

//        INPUT input;
//        input.type = INPUT_KEYBOARD;
//        input.ki.wScan = 0;
////        input.ki.wVk = 0;
//        input.ki.dwFlags = 0;KEYEVENTF_SCANCODE;
//        input.ki.time = 0;
//        input.ki.dwExtraInfo = 0;

        if ( wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN )
        {
            GetSingleton( )->SetKeyState( Events->vkCode, 1 );
            if ((Events->flags & LLKHF_INJECTED) == 0)
                if (GetSingleton()->callback_(true, Events->scanCode)) {
                    spdlog::info("key {} press blocked", Events->scanCode);
                    return 1;
                }
//            spdlog::info("key {} pressed", Events->vkCode);
        }

        if ( wParam == WM_KEYUP || wParam == WM_SYSKEYUP )
        {
//            input.ki.dwFlags |= KEYEVENTF_KEYUP;
            GetSingleton( )->SetKeyState( Events->vkCode, 0 );
            if ((Events->flags & LLKHF_INJECTED) == 0) {
                if (GetSingleton()->callback_(false, Events->scanCode)) {
                    spdlog::info("key {} release blocked", Events->scanCode);
                    return 1;
                }
            }
        }

//        if ((Events->flags & LLKHF_INJECTED) == 0) {
//            if (Events->vkCode == 'A') {
//                input.ki.wVk = 'B';
////                input.ki.wScan = 48;
//                SendInput(1, &input, sizeof(INPUT));
//                return 1;
//            } else if (Events->vkCode == 'D') {
//                input.ki.wVk = 'A';
////                input.ki.wScan = 30;
//                SendInput(1, &input, sizeof(INPUT));
//                return 1;
//            }
//        }

        return CallNextHookEx( nullptr, nCode, wParam, lParam );
    }

    CKeyEvents( )
    {
        hhookObject = nullptr;

        memset( Segments, 0, sizeof( Segments ) );
    }

    ~CKeyEvents( )
    {

    }

public:
    static CKeyEvents* GetSingleton( )
    {
        static CKeyEvents Singleton;

        return &Singleton;
    }

    bool GetKeyState( uint8_t Key )
    {
        uint8_t SegmentID = Key / 64;

        uint8_t Shifting = Key - SegmentID * 64;

        return GetBitFromSegment( Segments[ SegmentID ], Shifting );
    }

    bool Hook(callback_t&& callback)
    {
        callback_ = std::move(callback);

        hhookObject = SetWindowsHookEx( WH_KEYBOARD_LL, KeyboardHook, nullptr, 0);

        return hhookObject != nullptr;
    }

    bool Unhook( )
    {
        memset( Segments, 0, sizeof( Segments ) );

        return UnhookWindowsHookEx( hhookObject );
    }
};
