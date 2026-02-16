#include "notfoundhandler.hpp"

// stl
#include <memory>

// boost
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/log/trivial.hpp>

// self
#include <logging.hpp>


namespace httpserver {


namespace asio = boost::asio;
namespace beast = boost::beast;


asio::awaitable<beast::http::message_generator> NotFoundHandler::handle(RequestContext&& req) {
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
    beast::http::response<beast::http::string_body> res{beast::http::status::not_found, msg.version()};
    res.set(beast::http::field::content_type, "text/plain");
    res.keep_alive(msg.keep_alive());
    res.body() = "404 Not Found";
    res.prepare_payload();
    co_return res;
}


} // namespace httpserver