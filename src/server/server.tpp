/// @file    server.tpp
/// @brief   Template implementation of Server – listener, session, and error logging.
/// @author  saintson (pan.aleksandr.off@gmail.com)
/// @date    17.02.2026
/// @copyright Copyright (c) 2026 saintson. All rights reserved.
///            Licensed under the GNU General Public License v3.0 (GPLv3).

namespace httpserver {


namespace beast = boost::beast;
namespace asio = boost::asio;


template<IsRouter RouterType>
Server<RouterType>::Server(beast::net::ip::port_type port, RouterType handlers)
        : m_router{std::move(handlers)}, m_ctx{1}, m_signals{m_ctx, SIGINT, SIGTERM} {
    BOOST_LOG_TRIVIAL(debug) << std::format("server initializing on 0.0.0.0:{}", port);

    m_signals.async_wait(
        [&](const boost::system::error_code& error, int signal_number) {
            if (!error) {
                BOOST_LOG_TRIVIAL(info) << std::format("caught signal {}, shutting down", signal_number);
                m_ctx.stop();
            } else if (error == asio::error::operation_aborted) {
                BOOST_LOG_TRIVIAL(error) << "signal wait cancelled";
            } else {
                BOOST_LOG_TRIVIAL(error) << std::format("signal wait error: {} (code: {})", 
                                                error.message(), error.value());
            }
        }
    );
    
    asio::co_spawn(m_ctx, listen(port), asio::detached);
}


template<IsRouter RouterType>
Server<RouterType>::~Server() {
    m_ctx.stop();
    BOOST_LOG_TRIVIAL(info) << "server shutdown complete";
}


template<IsRouter RouterType>
std::expected<void, std::string> Server<RouterType>::run() {
    BOOST_LOG_TRIVIAL(debug) << "server entering io_context run loop";
    try {
        m_ctx.run();
    } catch (std::exception& ex) {
        BOOST_LOG_TRIVIAL(error) << std::format("server run exception: {}", ex.what());
        return std::unexpected(ex.what());
    }

    return {};
}


template<IsRouter RouterType>
asio::awaitable<void> Server<RouterType>::listen(beast::net::ip::port_type port) {
    auto executor = co_await asio::this_coro::executor;
    beast::net::ip::tcp::acceptor acceptor{executor, {beast::net::ip::tcp::v4(), port}};
    acceptor.set_option(asio::socket_base::reuse_address(true));

    BOOST_LOG_TRIVIAL(info) << std::format("listener started on 0.0.0.0:{}", port);

    while (!m_ctx.stopped()) {
        auto socket = co_await acceptor.async_accept(asio::make_strand(m_ctx.get_executor()), asio::use_awaitable);
        auto remote = socket.remote_endpoint();
        BOOST_LOG_TRIVIAL(debug) << std::format("new connection from {}:{}", 
                                        remote.address().to_string(), remote.port());
        asio::co_spawn(executor, session(beast::tcp_stream{std::move(socket)}), asio::detached);
    }
}


template<IsRouter RouterType>
asio::awaitable<void> Server<RouterType>::session(beast::tcp_stream stream) {
    beast::flat_buffer buffer;
    auto ep = stream.socket().remote_endpoint();
    auto peer = std::format("{}:{}", ep.address().to_string(), ep.port());

    try {
        while (true) {
            stream.expires_after(std::chrono::seconds(5));

            beast::http::request_parser<beast::http::buffer_body> parser;
            parser.body_limit(std::numeric_limits<std::uint64_t>::max());

            {
            auto [error_code, _] = co_await beast::http::async_read_header(stream, buffer, parser, asio::as_tuple(asio::use_awaitable));
            if (error_logging(error_code, peer)) {
                break;
            }
            }

            stream.expires_never();

            auto& msg = parser.get();
            bool keep_alive = msg.keep_alive();

            BOOST_LOG_TRIVIAL(debug) << std::format("[{}] {} {} keep_alive={}", 
                                            peer, std::string{msg.method_string()}, 
                                            std::string{msg.target()}, keep_alive);

            RequestContext ctx{&parser, &stream, &buffer};
            auto handler = m_router.route(ctx);
            beast::http::message_generator response = co_await handler->handle(std::move(ctx));

            {
            auto [error_code, _] = co_await beast::async_write(stream, std::move(response), asio::as_tuple(asio::use_awaitable));
            if (error_logging(error_code, peer)) {
                break;
            }
            }

            if (!keep_alive) {
                BOOST_LOG_TRIVIAL(debug) << std::format("[{}] closing, keep-alive not requested", peer);
                break;
            }
        }
    } catch (const boost::system::system_error& se) {
        error_logging(se.code(), peer);
    }

    beast::error_code ec;
    stream.socket().shutdown(asio::ip::tcp::socket::shutdown_send, ec);
}


template<IsRouter RouterType>
bool Server<RouterType>::error_logging(const boost::system::error_code& error_code, const std::string& peer) {
    if  (error_code) {
        if (error_code == beast::http::error::end_of_stream) {
            BOOST_LOG_TRIVIAL(debug) << std::format("[{}] peer closed connection", peer);
        } 
        else if (error_code == beast::error::timeout) {
            BOOST_LOG_TRIVIAL(debug) << std::format("[{}] session idle timeout", peer);
        }
        else if (error_code == asio::error::operation_aborted) {
            BOOST_LOG_TRIVIAL(debug) << std::format("[{}] operation cancelled", peer);
        }
        else {
            BOOST_LOG_TRIVIAL(error) << std::format("[{}] session error: {} {} (code: {})",
                                                        peer, error_code.category().name(),
                                                        error_code.message(), error_code.value());
        }
        return true;
    }
    return false;
}


} // namespace httpserver