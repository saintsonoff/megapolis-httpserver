#include "infohandler.hpp"

// boost
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/log/trivial.hpp>

// self
#include <logging.hpp>


namespace httpserver {


namespace beast = boost::beast;
namespace asio = boost::asio;


asio::awaitable<beast::http::message_generator> InfoHandler::handle(Request&& req) {
    beast::http::response<beast::http::string_body> res{beast::http::status::ok, req.version()};
    res.set(beast::http::field::content_type, "text/plain");
    res.keep_alive(req.keep_alive());
    res.body() = std::string{kMessage};
    res.prepare_payload();
    co_return res;
}


} // namespace httpserver