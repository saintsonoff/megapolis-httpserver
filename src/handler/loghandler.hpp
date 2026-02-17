/// @file    loghandler.hpp
/// @brief   Handler that serves the server log file via GET /log.
/// @author  saintson (pan.aleksandr.off@gmail.com)
/// @date    17.02.2026
/// @copyright Copyright (c) 2026 saintson. All rights reserved.
///            Licensed under the GNU General Public License v3.0 (GPLv3).

#pragma once

// boost
#include <boost/asio.hpp>
#include <boost/beast.hpp>

// self
#include <handler.hpp>


namespace httpserver {


namespace asio = boost::asio;
namespace beast = boost::beast;


/// @brief Serves the contents of the server log file (/tmp/server_log.txt)
///        as a plain-text HTTP response using http::file_body for zero-copy
///        efficiency and bounded memory usage regardless of file size.
class LogHandler : public IHandler {
public:
    /// @brief Drains the request body, opens the log file and returns it.
    /// @param req  The request context.
    /// @return An awaitable yielding the log file response (200), or an error
    ///         response (404 / 500) if the file cannot be opened.
    asio::awaitable<beast::http::message_generator> handle(RequestContext&& req) override;
};


} // namespace httpserver
