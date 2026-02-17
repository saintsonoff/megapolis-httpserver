/// @file    router.hpp
/// @brief   URL/method-based request router mapping endpoints to handler factories.
/// @author  saintson (pan.aleksandr.off@gmail.com)
/// @date    17.02.2026
/// @copyright Copyright (c) 2026 saintson. All rights reserved.
///            Licensed under the GNU General Public License v3.0 (GPLv3).

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


/// @brief Concept requiring a type to provide a route(const RequestContext&) -> Handler method.
/// @tparam RouterType The candidate router type.
template<typename RouterType>
concept IsRouter = requires(RouterType router, const RequestContext& req) {
    {router.route(req)} -> std::same_as<Handler>;
};

/// @brief Routes incoming requests to the appropriate handler factory.
/// @tparam HandlerFabricsRangeType A range of HandlerFabric entries to search.
/// @note  Performs a linear scan; the first match on endpoint and (optionally) method wins.
///        Returns a NotFoundHandler if no factory matches.
template<IsHandlerFabricsRange HandlerFabricsRangeType>
class Router {
public:
    /// @brief Constructs a Router from a range of handler factories.
    /// @param range The handler factories to register.
    Router(HandlerFabricsRangeType range);

public:
    /// @brief Finds the first matching handler factory and creates a handler.
    /// @param req The request context (only the parsed headers are inspected).
    /// @return A new handler instance, or a NotFoundHandler if nothing matches.
    Handler route(const RequestContext& req) const;

private:
    HandlerFabricsRangeType handlers_fabric;  ///< The registered handler factories.
};


} // namespace httpserver


// header only implemetation
#include <router.tpp>