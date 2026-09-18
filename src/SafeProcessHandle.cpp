//
// Created by Arthur on 16/12/2020.
//

#include "SafeProcessHandle.h"

#include <ntstatus.h>

#define IS_VALID_HANDLE(handle) (handle && handle != INVALID_HANDLE_VALUE)

CSafeProcessHandle::CSafeProcessHandle(DWORD dwProcessId) :
        m_dwTargetProcessId(dwProcessId)
{
    ClearVeriables();
}
CSafeProcessHandle::~CSafeProcessHandle()
{
    m_dwTargetProcessId = 0;

    ClearVeriables();
}

void CSafeProcessHandle::ClearVeriables()
{
    CloseUselessHandles();

    if (IS_VALID_HANDLE(m_hTargetProcessHandle))
        CloseHandle(m_hTargetProcessHandle);
    m_hTargetProcessHandle = INVALID_HANDLE_VALUE;
}

typedef NTSTATUS(NTAPI* lpNtGetNextProcess)(HANDLE ProcessHandle, ACCESS_MASK DesiredAccess, ULONG HandleAttributes, ULONG Flags, PHANDLE NewProcessHandle);

HANDLE CSafeProcessHandle::CreateProcessHandle()
{
    //DWORD dwExitCode = 0;

    auto hNtdll = LoadLibraryA("ntdll.dll");
    if (!hNtdll) {
//        DEBUG_LOG(LL_CRI, "WinModuleTable->hNtdll bind fail!");
        return false;
    }

    HANDLE hCurr = nullptr;
    auto NtGetNextProcess = (lpNtGetNextProcess)GetProcAddress(hNtdll, "NtGetNextProcess");
    if (!NtGetNextProcess) {
//        DEBUG_LOG(LL_CRI, "WinAPITable->NtGetNextProcess bind fail!");
        return false;
    }

    while (NtGetNextProcess(hCurr, MAXIMUM_ALLOWED, 0, 0, &hCurr) == STATUS_SUCCESS)
    {
        //if (!GetExitCodeProcess(hCurr, &dwExitCode) || dwExitCode != STILL_ACTIVE)
        //	continue;

        if (GetProcessId(hCurr) == m_dwTargetProcessId)
            m_hTargetProcessHandle = hCurr;
        else
            m_vHandleList.push_back(hCurr);
    }

    CloseUselessHandles();
    return m_hTargetProcessHandle;
}

void CSafeProcessHandle::CloseUselessHandles()
{
    for (size_t i = 0; i < m_vHandleList.size(); i++)
    {
        __try { CloseHandle(m_vHandleList[i]); }
        __except (1) { }
    }
    m_vHandleList.clear();
}
