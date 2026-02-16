#pragma once


// stl
#include <string>
#include <string_view>
#include <memory>
#include <ranges>
#include <concepts>
#include <utility>
#include <optional>

// boost
#include <boost/beast.hpp>
#include <boost/asio.hpp>
#include <boost/log/trivial.hpp>

// self
#include <logging.hpp>


namespace httpserver {


namespace beast = boost::beast;
namespace asio = boost::asio;


struct RequestContext {
    beast::http::request_parser<beast::http::buffer_body>* parser;
    beast::tcp_stream* socket;
    beast::flat_buffer* buffer;
};

class IHandler;
using Handler = std::shared_ptr<IHandler>;

class IHandlerFabric;
using HandlerFabric = std::shared_ptr<IHandlerFabric>;


template<typename HandlerFabricsRangeType>
concept IsHandlerFabricsRange = std::ranges::range<HandlerFabricsRangeType>
    && std::same_as<std::ranges::range_value_t<HandlerFabricsRangeType>, HandlerFabric>;


class IHandler {
public:
    virtual asio::awaitable<beast::http::message_generator> handle(RequestContext&& req) = 0;
    virtual ~IHandler() {};
};


class IHandlerFabric {
public:
    virtual Handler create() const = 0;
    virtual std::string_view endpoint() const = 0;
    virtual std::optional<beast::http::verb> method() const = 0;
    virtual ~IHandlerFabric() {};
};


template<std::derived_from<IHandler> HandlerType>
class HandlerFabricImpl : public IHandlerFabric {
public:
    HandlerFabricImpl(beast::http::verb method, std::string endpoint);
    HandlerFabricImpl(std::string endpoint);

public:
    Handler create() const override;
    std::string_view endpoint() const override;
    std::optional<beast::http::verb> method() const override;
private:
    std::optional<beast::http::verb> m_method;
    std::string m_endpoint;
};


} // namespace httpserver


// header only implementation
#include "handler.tpp"