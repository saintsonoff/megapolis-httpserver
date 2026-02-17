/// @file    root_checker.hpp
/// @brief   Utility to verify that the process is running with root privileges.
/// @author  saintson (pan.aleksandr.off@gmail.com)
/// @date    17.02.2026
/// @copyright Copyright (c) 2026 saintson. All rights reserved.
///            Licensed under the GNU General Public License v3.0 (GPLv3).

#pragma once

namespace httpserver {


/// @brief Checks whether the current process has root (UID 0) privileges.
/// @return true if running as root, false otherwise.
/// @note  On non-Unix platforms this function triggers a static_assert failure.
bool is_root_execution();


} // namespace httpserver
