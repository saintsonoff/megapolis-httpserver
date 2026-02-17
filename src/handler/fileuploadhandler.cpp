/// @file    fileuploadhandler.cpp
/// @brief   Implementation of FileUploadHandler – incremental body-to-file writer.
/// @author  saintson (pan.aleksandr.off@gmail.com)
/// @date    17.02.2026
/// @copyright Copyright (c) 2026 saintson. All rights reserved.
///            Licensed under the GNU General Public License v3.0 (GPLv3).

#include "fileuploadhandler.hpp"


// stl
#include <fstream>
#include <filesystem>
#include <string>
#include <format>
#include <chrono>

// boost
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/log/trivial.hpp>

// self
#include <logging.hpp>


namespace {


/// @brief Generates a nanosecond-precision timestamp prefix string.
/// @return A string formatted as "YYYYMMDD_HHMMSS_NNNNNNNNN".
std::string get_timestamp_prefix() {
    auto now = std::chrono::system_clock::now();    
    auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
        now.time_since_epoch()) % 1'000'000'000;

    return std::format("{:%Y%m%d_%H%M%S}_{:09d}", 
                       std::chrono::floor<std::chrono::seconds>(now), 
                       ns.count());
}

/// @brief Builds a safe filesystem path by combining an upload directory,
///        a timestamp prefix, and the sanitised base name of @p raw_filename.
/// @param upload_dir    The target directory (e.g. "/tmp").
/// @param raw_filename  An arbitrary filename; only the base name is kept.
/// @return The full path string.
std::string construct_path(const std::string& upload_dir, std::string raw_filename) {
    static 
    std::filesystem::path p(raw_filename);
    std::string safe_name = p.filename().string();
    
    return (std::filesystem::path(upload_dir) / (get_timestamp_prefix() + "_" + safe_name)).string();
}


} // namespace


namespace httpserver {


namespace asio = boost::asio;
namespace beast = boost::beast;

asio::awaitable<beast::http::message_generator> FileUploadHandler::handle(RequestContext&& req) {
    auto& msg = req.parser->get();
    auto version = msg.version();
    bool keep_alive = msg.keep_alive();

    std::string full_path = construct_path("/tmp", "data.bin");
    
    BOOST_LOG_TRIVIAL(debug) << std::format("Saving to: {}", full_path);

    std::ofstream file(full_path, std::ios::binary);

    if (!file) {
        BOOST_LOG_TRIVIAL(error) << "failed to open /tmp/data.bin for writing";
        beast::http::response<beast::http::string_body> res{
            beast::http::status::internal_server_error, version
        };
        res.set(beast::http::field::content_type, "text/plain");
        res.keep_alive(keep_alive);
        res.body() = "error opening file";
        res.prepare_payload();
        co_return res;
    }

    std::size_t total = 0;

    while (!req.parser->is_done()) {
        char chunk[1 << 16];
        req.parser->get().body().data = chunk;
        req.parser->get().body().size = sizeof(chunk);

        co_await beast::http::async_read_some(*req.socket, *req.buffer, *req.parser, asio::use_awaitable);

        std::size_t read_size = sizeof(chunk) - req.parser->get().body().size;
        file.write(chunk, static_cast<std::streamsize>(read_size));

        if (!file) {
            BOOST_LOG_TRIVIAL(error) << "write error to /tmp/data.bin";
            beast::http::response<beast::http::string_body> res{
                beast::http::status::internal_server_error, version
            };
            res.set(beast::http::field::content_type, "text/plain");
            res.keep_alive(keep_alive);
            res.body() = "error writing file";
            res.prepare_payload();
            co_return res;
        }

        total += read_size;
    }

    file.close();
    BOOST_LOG_TRIVIAL(debug) << std::format("uploaded {} bytes to {}", total, full_path);

    beast::http::response<beast::http::string_body> res{beast::http::status::ok, version};
    res.set(beast::http::field::content_type, "text/plain");
    res.keep_alive(keep_alive);
    res.body() = "Success";
    res.prepare_payload();
    co_return res;
}


} // namespace httpserver