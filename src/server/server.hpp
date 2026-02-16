#pragma once

// stl
#include <string>
#include <expected>
#include <chrono>

// boost
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/log/trivial.hpp>

// self
#include <logging.hpp>
#include <handler.hpp>
#include <router.hpp>


namespace httpserver {


namespace beast = boost::beast;
namespace asio = boost::asio;


template<IsRouter RouterType>
class Server {
public:
    Server(beast::net::ip::port_type port, RouterType handlers);
    ~Server();
public:
    std::expected<void, std::string> run();

private:
    asio::awaitable<void> listen(beast::net::ip::port_type port);
    asio::awaitable<void> session(beast::tcp_stream stream);

private:
    RouterType m_router;
    asio::io_context m_ctx;
    asio::signal_set m_signals;
};


} // namespace httpserver



// header only implemetation
#include "server.tpp"