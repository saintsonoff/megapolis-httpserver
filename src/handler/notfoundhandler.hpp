/// @file    notfoundhandler.hpp
/// @brief   Default handler returning HTTP 404 for unmatched routes.
/// @author  saintson (pan.aleksandr.off@gmail.com)
/// @date    17.02.2026
/// @copyright Copyright (c) 2026 saintson. All rights reserved.
///            Licensed under the GNU General Public License v3.0 (GPLv3).

#pragma once

// stl
#include <memory>

// boost
#include <boost/asio.hpp>
#include <boost/beast.hpp>

// self
#include <handler.hpp>


namespace httpserver {


namespace asio = boost::asio;


/// @brief Produces an HTTP 404 response for any unmatched route.
/// @note  Any request body is discarded before responding.
class NotFoundHandler : public IHandler {
public:
    /// @brief Drains the request body and responds with HTTP 404 "Not Found".
    /// @param req  The request context.
    /// @return An awaitable yielding the 404 response.
    asio::awaitable<beast::http::message_generator> handle(RequestContext&& req) override;
};


} // namespace httpserver