#pragma once

// stl
#include <string_view>

// boost
#include <boost/asio.hpp>
#include <boost/beast.hpp>

// self
#include <handler.hpp>


namespace httpserver {


namespace asio = boost::asio;
namespace beast = boost::beast;


class InfoHandler : public IHandler {
private:
    static constexpr std::string_view kMessage = "all ok";
public:
    asio::awaitable<beast::http::message_generator> handle(Request&& req) override;
};


} // namespace httpserver