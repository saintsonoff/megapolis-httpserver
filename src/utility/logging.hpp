/// @file    logging.hpp
/// @brief   Boost.Log initialisation and auto-init singleton for the HTTP server.
/// @author  saintson (pan.aleksandr.off@gmail.com)
/// @date    17.02.2026
/// @copyright Copyright (c) 2026 saintson. All rights reserved.
///            Licensed under the GNU General Public License v3.0 (GPLv3).

#pragma once

#include <boost/log/trivial.hpp>
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/attributes.hpp>

/// @brief Path to the persistent log file read by the /log endpoint.
inline constexpr const char* kServerLogPath = "/tmp/server_log.txt";

/// @brief Initialises Boost.Log with a console sink and a text-file sink,
///        timestamps, thread IDs, and severity-based filtering
///        (debug in Debug builds, info in Release).
inline void init_logging() {
    namespace log = boost::log;
    log::add_common_attributes();

    auto fmt = (
        log::expressions::stream
            << "[" << log::expressions::format_date_time<boost::posix_time::ptime>("TimeStamp", "%Y-%m-%d %H:%M:%S") << "] "
            << "[" << log::expressions::attr<log::attributes::current_thread_id::value_type>("ThreadID") << "] "
            << "[" << log::trivial::severity << "] "
            << log::expressions::smessage
    );

    // Console sink (unchanged).
    log::add_console_log(
        std::clog,
        log::keywords::format = fmt
    );

    // File sink – written to /tmp so the /log endpoint can serve it.
    log::add_file_log(
        log::keywords::file_name      = kServerLogPath,
        log::keywords::open_mode      = std::ios_base::app,   // append across restarts
        log::keywords::auto_flush     = true,                  // flush every record
        log::keywords::format         = fmt
    );

#ifdef NDEBUG
    log::core::get()->set_filter(log::trivial::severity >= log::trivial::info);
#else
    log::core::get()->set_filter(log::trivial::severity >= log::trivial::debug);
#endif
}


namespace httpserver {


/// @brief Internal detail namespace; holds the auto-init singleton.
namespace details{
    /// @brief Singleton whose constructor calls init_logging() at static-init time.
    inline const struct LogInit {
        LogInit() { init_logging(); }
    } log_init;
} // details


} // httpserver