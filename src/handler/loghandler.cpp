/// @file    loghandler.cpp
/// @brief   Implementation of LogHandler – serves the server log file.
/// @author  saintson (pan.aleksandr.off@gmail.com)
/// @date    17.02.2026
/// @copyright Copyright (c) 2026 saintson. All rights reserved.
///            Licensed under the GNU General Public License v3.0 (GPLv3).

#include "loghandler.hpp"

// stl
#include <cstdint>

// boost
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/log/trivial.hpp>

// self
#include <logging.hpp>
#include <handler.hpp>


namespace httpserver {


namespace beast = boost::beast;
namespace asio = boost::asio;


asio::awaitable<beast::http::message_generator> LogHandler::handle(RequestContext&& req) {
    connection_logging(req);

    while (!req.parser->is_done()) {
        char discard[1 << 16];
        req.parser->get().body().data = discard;
        req.parser->get().body().size = sizeof(discard);
        try {
            co_await beast::http::async_read_some(*req.socket, *req.buffer, *req.parser, asio::use_awaitable);
        } catch (const boost::system::system_error& e) {
            if (e.code() != beast::http::error::need_buffer)
                throw;
        }
    }

    auto& msg = req.parser->get();

    beast::error_code error_code;
    beast::http::file_body::value_type body;
    body.open(kServerLogPath, beast::file_mode::scan, error_code);

    if (error_code == boost::system::errc::no_such_file_or_directory) {
        beast::http::response<beast::http::string_body> res{beast::http::status::not_found, msg.version()};
        res.set(beast::http::field::content_type, "text/plain; charset=utf-8");
        res.keep_alive(msg.keep_alive());
        res.body() = "Log file not found.";
        res.prepare_payload();
        co_return res;
    }
    if (error_code) {
        BOOST_LOG_TRIVIAL(error) << "Failed to open log file: " << error_code.message();
        beast::http::response<beast::http::string_body> res{beast::http::status::internal_server_error, msg.version()};
        res.set(beast::http::field::content_type, "text/plain; charset=utf-8");
        res.keep_alive(msg.keep_alive());
        res.body() = "Failed to read log file: " + error_code.message();
        res.prepare_payload();
        co_return res;
    }

    constexpr std::uint64_t kMaxLogTailSize = 5 * (1 << 11);
    const auto file_size = body.size();
    const std::uint64_t offset = (file_size > kMaxLogTailSize) ? (file_size - kMaxLogTailSize) : 0;
    const std::uint64_t content_size = file_size - offset;

    if (offset > 0) {
        body.seek(offset, error_code);
        if (error_code) {
            BOOST_LOG_TRIVIAL(error) << "Failed to seek log file: " << error_code.message();
            beast::http::response<beast::http::string_body> res{beast::http::status::internal_server_error, msg.version()};
            res.set(beast::http::field::content_type, "text/plain; charset=utf-8");
            res.keep_alive(msg.keep_alive());
            res.body() = "Failed to seek log file: " + error_code.message();
            res.prepare_payload();
            co_return res;
        }
    }

    beast::http::response<beast::http::file_body> res{
        std::piecewise_construct,
        std::make_tuple(std::move(body)),
        std::make_tuple(beast::http::status::ok, msg.version())
    };
    res.set(beast::http::field::content_type, "text/plain; charset=utf-8");
    res.content_length(content_size);
    res.keep_alive(msg.keep_alive());
    co_return res;
}


} // namespace httpserver
