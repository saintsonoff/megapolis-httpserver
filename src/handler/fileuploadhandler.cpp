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

asio::awaitable<beast::http::message_generator> FileUploadHandler::handle(Request&& req) {
    std::ofstream file("/tmp/data.bin", std::ios::binary);

    if (!file) {
        beast::http::response<beast::http::string_body> res{
            beast::http::status::internal_server_error, req.version()
        };
        res.set(beast::http::field::content_type, "text/plain");
        res.keep_alive(req.keep_alive());
        res.body() = "error opening file";
        res.prepare_payload();
        co_return res;
    }

    file.write(req.body().data(), static_cast<std::streamsize>(req.body().size()));
    file.close();

    beast::http::response<beast::http::string_body> res{beast::http::status::ok, req.version()};
    res.set(beast::http::field::content_type, "text/plain");
    res.keep_alive(req.keep_alive());
    res.body() = "Success";
    res.prepare_payload();
    co_return res;
}


} // namespace httpserver