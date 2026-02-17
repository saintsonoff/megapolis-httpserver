/// @file    parallelhandler.cpp
/// @brief   Implementation of ParallelHandler and ParallelHandlerFabric.
/// @author  saintson (pan.aleksandr.off@gmail.com)
/// @date    17.02.2026
/// @copyright Copyright (c) 2026 saintson. All rights reserved.
///            Licensed under the GNU General Public License v3.0 (GPLv3).

#include "parallelhandler.hpp"

// stl
#include <concepts>
#include <memory>
#include <optional>

// boost
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/log/trivial.hpp>

// self
#include <handler.hpp>
#include <logging.hpp>


namespace httpserver {


namespace asio = boost::asio;
namespace beast = boost::beast;


ParallelHandler::ParallelHandler(Handler handler, asio::thread_pool& tpool)
        : m_handler{std::move(handler)}, m_tpool{tpool} {
}

asio::awaitable<beast::http::message_generator> ParallelHandler::handle(RequestContext&& req) {
    auto res = co_await asio::co_spawn(
        m_tpool,
        [handler = m_handler, req = std::move(req)]() mutable
                -> asio::awaitable<std::optional<beast::http::message_generator>> {
            co_return co_await handler->handle(std::move(req));
        },
        asio::use_awaitable
    );
    co_return std::move(*res);
}


Handler ParallelHandlerFabric::create() const {
    return std::make_shared<ParallelHandler>(m_fabric->create(), m_tpool);
}

std::string_view ParallelHandlerFabric::endpoint() const {
    return m_fabric->endpoint();
}

std::optional<beast::http::verb> ParallelHandlerFabric::method() const {
    return m_fabric->method();
}


} // namespace httpserver