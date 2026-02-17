/// @file    infohandler.hpp
/// @brief   Simple health-check handler returning a fixed status message.
/// @author  saintson (pan.aleksandr.off@gmail.com)
/// @date    17.02.2026
/// @copyright Copyright (c) 2026 saintson. All rights reserved.
///            Licensed under the GNU General Public License v3.0 (GPLv3).

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


/// @brief Returns a fixed "all ok" plain-text response for health checks.
/// @note  Any request body is discarded before responding.
class InfoHandler : public IHandler {
private:
    static constexpr std::string_view kMessage = "all ok";  ///< Fixed health-check response body.
public:
    /// @brief Drains the request body and responds with HTTP 200 "all ok".
    /// @param req  The request context.
    /// @return An awaitable yielding the plain-text response.
    asio::awaitable<beast::http::message_generator> handle(RequestContext&& req) override;
};


} // namespace httpserver