#pragma once

// stl
#include <type_traits>
#include <concepts>
#include <ranges>
#include <memory>

// boost 
#include <boost/beast.hpp>
#include <boost/asio.hpp>

// self
#include <handler.hpp>
#include <notfoundhandler.hpp>


namespace httpserver {


namespace beast = boost::beast;
namespace asio = boost::asio;

template<typename RouterType>
concept IsRouter = requires(RouterType router, beast::http::request<boost::beast::http::empty_body> header) {
    {router.route(header)} -> std::same_as<Handler>;
};

template<IsHandlerFabricsRange HandlerFabricsRangeType>
class Router {
public:
    Router(HandlerFabricsRangeType range);

public:
    Handler route(beast::http::request<boost::beast::http::empty_body> header) const;

private:
    HandlerFabricsRangeType handlers_fabric;
};


} // namespace httpserver


// header only implemetation
#include <router.tpp>