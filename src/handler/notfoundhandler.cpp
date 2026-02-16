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


asio::awaitable<beast::http::message_generator> NotFoundHandler::handle(Request&& req) {
    beast::http::response<beast::http::string_body> res{beast::http::status::not_found, req.version()};
    res.set(beast::http::field::content_type, "text/plain");
    res.keep_alive(req.keep_alive());
    res.body() = "404 Not Found";
    res.prepare_payload();
    co_return res;
}


} // namespace httpserver