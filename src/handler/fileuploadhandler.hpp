#pragma once

// boost
#include <boost/asio.hpp>

// self
#include <handler.hpp>


namespace httpserver {


namespace asio = boost::asio;


class FileUploadHandler : public IHandler {
public:
    asio::awaitable<void> handle(RequestContext req) override;
};


} // namespace httpserver