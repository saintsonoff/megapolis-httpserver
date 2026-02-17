#pragma once

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


class ParallelHandler : public IHandler {
public:
    ParallelHandler(Handler handler, asio::thread_pool& tpool)
            : m_handler{std::move(handler)}, m_tpool{tpool} {
    }

public:
    asio::awaitable<beast::http::message_generator> handle(RequestContext&& req) override {
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

private:
    Handler m_handler;
    asio::thread_pool& m_tpool;
};


class ParallelHandlerFabric : public IHandlerFabric {
public:

    template<std::convertible_to<asio::thread_pool>... TPoolArgs>
    ParallelHandlerFabric(HandlerFabric fabric, TPoolArgs&&... tpool_args)
            : m_fabric(std::move(fabric)), m_tpool{std::forward<TPoolArgs>(tpool_args)...} {
    }

public:
    Handler create() const override {
        return std::make_shared<ParallelHandler>(m_fabric->create(), m_tpool);
    }

    std::string_view endpoint() const override {
        return m_fabric->endpoint();
    }

    std::optional<beast::http::verb> method() const override {
        return m_fabric->method();
    }

private:
    HandlerFabric m_fabric;
    mutable asio::thread_pool m_tpool;
};


} // namespace httpserver