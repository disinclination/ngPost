/*
 * Copyright (c) 2020 Matthieu Bruel <Matthieu.Bruel@gmail.com>
 * Copyright (c) 2025 disinclination
 * Licensed under the GNU General Public License v3.0
 */

#ifdef WIN32
#include "WindowsUtils.h"
#include <windows.h>
#include <filesystem>

std::string GetWinRARPathFromRegistry() {
    HKEY hKey;
    const char* subKey = "SOFTWARE\\WinRAR";
    char installPath[MAX_PATH];
    DWORD bufSize = sizeof(installPath);

    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, subKey, 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        if (RegQueryValueExA(hKey, "InstallPath", nullptr, nullptr, (LPBYTE)installPath, &bufSize) == ERROR_SUCCESS) {
            RegCloseKey(hKey);
            return std::string(installPath) + "\\";
        }
        RegCloseKey(hKey);
    }
    return "";
}

std::string CheckCommonPaths() {
    std::string paths[] = {
        "C:\\Program Files\\WinRAR\\",
        "C:\\Program Files (x86)\\WinRAR\\"
    };

    for (const std::string& path : paths) {
        if (std::filesystem::exists(path + "rar.exe")) {
            return path;
        }
    }
    return "";
}

std::string WindowsUtils::GetWinRARPath() {
    std::string path = GetWinRARPathFromRegistry();
    if (!path.empty()) {
        return path;
    }
    return CheckCommonPaths();
}

#endif
