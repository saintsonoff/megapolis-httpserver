#pragma once


// stl
#include <string>
#include <string_view>
#include <memory>
#include <ranges>
#include <concepts>
#include <utility>

// boost
#include <boost/beast.hpp>
#include <boost/asio.hpp>
#include <boost/log/trivial.hpp>

// self
#include <logging.hpp>


namespace httpserver {


namespace beast = boost::beast;
namespace asio = boost::asio;


using Request = beast::http::request<beast::http::string_body>;

class IHandler;
using Handler = std::shared_ptr<IHandler>;

class IHandlerFabric;
using HandlerFabric = std::shared_ptr<IHandlerFabric>;


template<typename HandlerFabricsRangeType>
concept IsHandlerFabricsRange = std::ranges::range<HandlerFabricsRangeType>
    && std::same_as<std::ranges::range_value_t<HandlerFabricsRangeType>, HandlerFabric>;


struct RequestContext {
public:
    asio::ip::tcp::socket socket;
    beast::flat_buffer buffer;
    // addition move, need be initialized
    std::unique_ptr<beast::http::request_parser<beast::http::empty_body>> parser;

public:
    const auto& header() const { return parser->get(); }
};


class IHandler {
public:
    virtual asio::awaitable<void> handle(RequestContext req) = 0;
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