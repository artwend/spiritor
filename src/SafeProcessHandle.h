//
// Created by Arthur on 16/12/2020.
//

#pragma once

#include <vector>

#include <windows.h>

class CSafeProcessHandle
{
public:
    CSafeProcessHandle(DWORD dwProcessId);
    ~CSafeProcessHandle();

    HANDLE	CreateProcessHandle();
    void	ClearVeriables();

protected:
    void	CloseUselessHandles();

private:
    std::vector<HANDLE> m_vHandleList;

    DWORD				m_dwTargetProcessId;
    HANDLE				m_hTargetProcessHandle;
};

