#include "notfoundhandler.hpp"

// boost
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/log/trivial.hpp>

// self
#include <logging.hpp>


namespace httpserver {


namespace asio = boost::asio;
namespace beast = boost::beast;


asio::awaitable<void> NotFoundHandler::handle(RequestContext req) {
    BOOST_LOG_TRIVIAL(debug) << "accepted 'not found' handler";

    beast::http::request_parser<beast::http::buffer_body> parser{std::move(*req.parser)};
    parser.body_limit(boost::none);

    char dump_buf[1<<11];
    beast::error_code error_code;

    while (!parser.is_done()) {
        parser.get().body().data = dump_buf;
        parser.get().body().size = sizeof(dump_buf);

        co_await beast::http::async_read_some(req.socket, req.buffer, parser, 
            beast::net::redirect_error(beast::net::use_awaitable, error_code));

        if (error_code == beast::http::error::need_buffer) {
            error_code = {};
        } 
        if (error_code) {
            break;
        }
    }


    beast::http::response<beast::http::string_body> res{beast::http::status::not_found, req.header().version()};
    res.set(beast::http::field::content_type, "text/plain");
    res.body() = "404 Not Found";
    res.prepare_payload();
    co_await beast::http::async_write(req.socket, res, beast::net::use_awaitable);

    {
    beast::error_code error_code;
    req.socket.shutdown(asio::ip::tcp::socket::shutdown_send, error_code);
    if (error_code) {
        BOOST_LOG_TRIVIAL(debug) << std::format("socket shutdown error {}", error_code.message());
        co_return;
    }
    }
}


} // namespace httpserver