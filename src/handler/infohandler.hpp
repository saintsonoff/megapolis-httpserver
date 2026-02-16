#pragma once

// stl
#include <string_view>

// boost
#include <boost/asio.hpp>

// self
#include <handler.hpp>


namespace httpserver {


namespace asio = boost::asio;


class InfoHandler : public IHandler {
private:
    static constexpr std::string_view kMessage = "all ok";
public:
    asio::awaitable<void> handle(RequestContext req) override;
};


} // namespace httpserver