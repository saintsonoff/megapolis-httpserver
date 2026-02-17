/// @file    root_checker.cpp
/// @brief   Implementation of is_root_execution() using POSIX geteuid().
/// @author  saintson (pan.aleksandr.off@gmail.com)
/// @date    17.02.2026
/// @copyright Copyright (c) 2026 saintson. All rights reserved.
///            Licensed under the GNU General Public License v3.0 (GPLv3).

#include "root_checker.hpp"

// stl
#include <utility>

#ifdef __unix__
// unix
#include <unistd.h>
#endif


namespace httpserver {


bool is_root_execution() {
#ifdef __unix__
    return geteuid() == 0;
#else
    static_assert(false);
#endif

}


} // namespace httpserver