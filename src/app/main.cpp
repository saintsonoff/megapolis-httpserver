#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/asio/thread_pool.hpp>

#include <concepts>
#include <print>
#include <expected>
#include <string>
#include <format>
#include <exception>
#include <memory>
#include <ranges>
#include <concepts>
#include <type_traits>


namespace beast = boost::beast;
namespace asio = boost::asio;

#include <boost/log/trivial.hpp>
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/attributes.hpp>


void init_logging() {
    boost::log::add_common_attributes();

    boost::log::add_console_log(
        std::clog,
        boost::log::keywords::format = (
            boost::log::expressions::stream
                << "[" << boost::log::expressions::format_date_time<boost::posix_time::ptime>("TimeStamp", "%Y-%m-%d %H:%M:%S") << "] "
                << "[" << boost::log::expressions::attr<boost::log::attributes::current_thread_id::value_type>("ThreadID") << "] "
                << "[" << boost::log::trivial::severity << "] "
                << boost::log::expressions::smessage
        )
    );
#ifdef NDEBUG
    boost::log::core::get()->set_filter(boost::log::trivial::severity >= boost::log::trivial::info);
#else
    boost::log::core::get()->set_filter(boost::log::trivial::severity >= boost::log::trivial::debug);
#endif
}


class IHandler {
public:
    virtual void handle(asio::ip::tcp::socket socket) = 0;
    virtual ~IHandler() {};
};

using Request = beast::http::request<beast::http::string_body>;
using Handler = std::shared_ptr<IHandler>;

class IHandlerFabric {
public:
    virtual Handler create() const = 0;
    virtual std::string_view endpoint() const = 0;
    virtual std::optional<beast::http::verb> method() const = 0;
    virtual ~IHandlerFabric() {};
};

class EmptyHandler : public IHandler {
public:
    void handle(asio::ip::tcp::socket socket) override {
        beast::http::response<beast::http::string_body> res{beast::http::status::ok, 11};
        res.set(beast::http::field::content_type, "text/plain");
        res.body() = "empty action handler called";
        res.prepare_payload();
        beast::http::write(socket, res);

        std::println("request handling");
    }
};

class NotFoundHandler : public IHandler {
public:
    void handle(asio::ip::tcp::socket socket) override {
        beast::http::response<beast::http::string_body> res{beast::http::status::not_found, 11};
        res.set(beast::http::field::content_type, "text/plain");
        res.body() = "404 Not Found";
        res.prepare_payload();
        beast::http::write(socket, res);
    }
};

template<std::derived_from<IHandler> HandlerType>
class HandlerFabricImpl : public IHandlerFabric {
public:
    HandlerFabricImpl(beast::http::verb method, std::string endpoint)
      : m_method{std::move(method)}, m_endpoint{std::move(endpoint)} {};

    HandlerFabricImpl(std::string endpoint)
      : m_method{std::nullopt}, m_endpoint{std::move(endpoint)} {};

    Handler create() const override {
        return std::make_shared<HandlerType>();
    };
    std::string_view endpoint() const override {
        return m_endpoint;
    };
    std::optional<beast::http::verb> method() const override {
        return m_method;
    };

private:
    std::optional<beast::http::verb> m_method;
    std::string m_endpoint;
};

using HandlerFabric = std::shared_ptr<IHandlerFabric>;

template<typename HandlerFabricsRangeType>
concept IsHandlerFabricsRange = std::ranges::range<HandlerFabricsRangeType>
    && std::same_as<std::ranges::range_value_t<HandlerFabricsRangeType>, HandlerFabric>;

template<IsHandlerFabricsRange HandlerFabricsRangeType>
class Router {
public:
    Router(HandlerFabricsRangeType range) : handlers_fabric(std::move(range)) {};

public:
    Handler route(beast::http::request<boost::beast::http::empty_body> header) const {
        
        auto it = std::ranges::find_if(handlers_fabric,
            [&header](const auto& handle) {
                if (handle->endpoint() != header.target()) {
                    return false;
                }
                auto method = handle->method();
                if (method) {
                    return *method == header.method();
                }
                return true;
            }
        );

        if (it != std::ranges::end(handlers_fabric)) {
            return (*it)->create();
        }

        return std::make_shared<NotFoundHandler>();
    }

private:
    HandlerFabricsRangeType handlers_fabric;
};

template<typename RouterType>
concept IsRouter = requires(RouterType router, beast::http::request<boost::beast::http::empty_body> header) {
    {router.route(header)} -> std::same_as<Handler>;
};

template<IsRouter RouterType>
class Server {
public:
    Server(beast::net::ip::port_type port, RouterType handlers)
      : m_router{std::move(handlers)}, m_ctx{1}, m_signals{m_ctx, SIGINT, SIGTERM} {
        m_signals.async_wait(
            [&](const boost::system::error_code& error, int signal_number) {
            if (!error) {
                    std::println("Received signal {}", signal_number);
                    m_ctx.stop();
                }
            }
        );

        co_spawn(m_ctx,
        [&, port]() -> asio::awaitable<void> {
            auto executor = co_await asio::this_coro::executor;
            asio::ip::tcp::acceptor acceptor{executor, {asio::ip::tcp::v4(), port}};
            BOOST_LOG_TRIVIAL(info)
                << std::format("server started on {}:{}",
                    acceptor.local_endpoint().address().to_string(), acceptor.local_endpoint().port());

            while (true) {
                auto socket = co_await acceptor.async_accept(asio::use_awaitable);

                co_spawn(executor, [socket = std::move(socket), this]() mutable
                  -> asio::awaitable<void> {
                    beast::flat_buffer buffer;
                    beast::http::request_parser<beast::http::empty_body> header_parser;
                    co_await beast::http::async_read(socket, buffer, header_parser, asio::use_awaitable);

                    auto handler = m_router.route(header_parser.get());
                    handler->handle(std::move(socket));
                }, asio::detached);
            }
        }, asio::detached);
    }

    ~Server() {
        m_ctx.stop();
        BOOST_LOG_TRIVIAL(debug) << "server shutdown";
    }
public:
    std::expected<void, std::string> run() {
        try {
            m_ctx.run();
        } catch (std::exception& ex) {
            return std::unexpected(ex.what());
        }

        return {};
    }

private:
    RouterType m_router;
    asio::io_context m_ctx;
    asio::signal_set m_signals;
};

int main() {
    enum { SUCCESSFULL, SERVER_START_ERROR };

    init_logging();

    Server server{1616,
        Router{std::vector<HandlerFabric>{
            std::make_shared<HandlerFabricImpl<EmptyHandler>>(beast::http::verb::get, "/1"),
            std::make_shared<HandlerFabricImpl<EmptyHandler>>("/2")
        }}};

    if (auto result = server.run(); !result) {
        BOOST_LOG_TRIVIAL(error) << result.error();
        return SERVER_START_ERROR;
    }

    return SUCCESSFULL;
}