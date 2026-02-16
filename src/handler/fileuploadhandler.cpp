#include "fileuploadhandler.hpp"


// stl
#include <fstream>
#include <string>
#include <format>

// boost
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/log/trivial.hpp>

// self
#include <logging.hpp>


namespace httpserver {


namespace asio = boost::asio;
namespace beast = boost::beast;

asio::awaitable<void> FileUploadHandler::handle(RequestContext req) {
    BOOST_LOG_TRIVIAL(debug) << "accepted 'upload file' handler";
    auto http_version = req.header().version();

    beast::error_code error_code;
    beast::http::request_parser<beast::http::file_body> parser{std::move(*req.parser)};
    
    parser.get().body().open("/tmp/data.bin", beast::file_mode::write, error_code);

    if (error_code) {
        beast::http::response<beast::http::string_body> res{beast::http::status::internal_server_error, http_version};
        res.body() = "error opening file: " + error_code.message();
        res.prepare_payload();
        co_await beast::http::async_write(req.socket, res, beast::net::use_awaitable);
        co_return;
    }

    co_await beast::http::async_read(
        req.socket, 
        req.buffer, 
        parser, 
        beast::net::redirect_error(beast::net::use_awaitable, error_code)
    );

    if (error_code) {
        beast::http::response<beast::http::string_body> res{beast::http::status::internal_server_error, http_version};
        res.body() = "error read file: " + error_code.message();
        res.prepare_payload();
        co_await beast::http::async_write(req.socket, res, beast::net::use_awaitable);
        co_return;
    }

    beast::http::response<beast::http::string_body> res{beast::http::status::ok, http_version};
    res.body() = "Success";
    res.prepare_payload();
    co_await beast::http::async_write(req.socket, res, beast::net::use_awaitable);
    
    {
    req.socket.shutdown(asio::ip::tcp::socket::shutdown_send, error_code);
    if (error_code) {
        BOOST_LOG_TRIVIAL(debug) << std::format("socket shutdown error {}", error_code.message());
        co_return;
    }
    }
}


} // namespace httpserver