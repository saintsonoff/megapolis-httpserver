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


std::string get_timestamp_prefix() {
    auto now = std::chrono::system_clock::now();    
    auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
        now.time_since_epoch()) % 1'000'000'000;

    return std::format("{:%Y%m%d_%H%M%S}_{:09d}", 
                       std::chrono::floor<std::chrono::seconds>(now), 
                       ns.count());
}

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

        try {
            co_await beast::http::async_read_some(*req.socket, *req.buffer, *req.parser, asio::use_awaitable);
        } catch (const boost::system::system_error& e) {
            if (e.code() != beast::http::error::need_buffer)
                throw;
        }

        std::size_t n = sizeof(chunk) - req.parser->get().body().size;
        file.write(chunk, static_cast<std::streamsize>(n));

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

        total += n;
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