/*
 * Copyright (c) 2020 Matthieu Bruel <Matthieu.Bruel@gmail.com>
 * Copyright (c) 2025 disinclination
 * Licensed under the GNU General Public License v3.0
 */

#ifndef WINDOWSUTILS_H
#define WINDOWSUTILS_H

#ifdef WIN32
#include <string>

class WindowsUtils {
public:
    static std::string GetWinRARPath();
};

#endif

#endif
