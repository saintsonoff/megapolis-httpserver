/// @file    fileuploadhandler.hpp
/// @brief   Handler for streaming file uploads to disk with timestamped filenames.
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


/// @brief Streams an HTTP POST body to a file on disk in 64 KiB chunks.
/// @note  Reads the body incrementally via async_read_some so that
///        arbitrarily large payloads can be received without exceeding RAM.
class FileUploadHandler : public IHandler {
public:
    /// @brief Handles the upload request by writing body data to /tmp.
    /// @param req  The request context (parser, socket, buffer).
    /// @return An awaitable yielding an HTTP 200 on success or 500 on I/O error.
    asio::awaitable<beast::http::message_generator> handle(RequestContext&& req) override;
};


} // namespace httpserver