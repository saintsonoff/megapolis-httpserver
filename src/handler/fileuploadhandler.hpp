#pragma once

// boost
#include <boost/asio.hpp>
#include <boost/beast.hpp>

// self
#include <handler.hpp>


namespace httpserver {


namespace asio = boost::asio;
namespace beast = boost::beast;


class FileUploadHandler : public IHandler {
public:
    asio::awaitable<beast::http::message_generator> handle(Request&& req) override;
};


} // namespace httpserver