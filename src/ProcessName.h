//
// Created by Arthur on 16/12/2020.
//

#pragma once

#include <string>

#include <windows.h>

class CProcessName
{
public:
    static std::string DosDevicePath2LogicalPath(LPCTSTR lpszDosPath);
    static std::string GetProcessFullName(HANDLE hProcess);
    static std::string GetProcessName(HANDLE hProcess);
};