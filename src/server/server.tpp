namespace httpserver {


template<IsRouter RouterType>
Server<RouterType>::Server(beast::net::ip::port_type port, RouterType handlers)
        : m_router{std::move(handlers)}, m_ctx{1}, m_signals{m_ctx, SIGINT, SIGTERM} {
    m_signals.async_wait(
        [&](const boost::system::error_code& error, int signal_number) {
            if (!error) {
                BOOST_LOG_TRIVIAL(info) << std::format("received signal {}", signal_number);
                m_ctx.stop();
            } else if (error == boost::asio::error::operation_aborted) {
                BOOST_LOG_TRIVIAL(debug) << "signal wait cancelled";
            } else {
                BOOST_LOG_TRIVIAL(error) << std::format("error waiting for signal: {} (code: {})", 
                                                error.message(), error.value());
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

            co_spawn(executor, [&executor, socket = std::move(socket), this]() mutable
                    -> asio::awaitable<void> {
                RequestContext request{.socket = std::move(socket), .buffer = {},
                    .parser = std::make_unique<beast::http::request_parser<beast::http::empty_body>>()};
                co_await beast::http::async_read_header(request.socket, request.buffer, *request.parser, asio::use_awaitable);

                auto handler = m_router.route(request.parser->get());
                co_spawn(executor, handler->handle(std::move(request)), asio::detached);
            }, asio::detached);
        }
    }, asio::detached);
}


template<IsRouter RouterType>
Server<RouterType>::~Server() {
    m_ctx.stop();
    BOOST_LOG_TRIVIAL(info) << "server shutdown";
}


template<IsRouter RouterType>
std::expected<void, std::string> Server<RouterType>::run() {
    try {
        m_ctx.run();
    } catch (std::exception& ex) {
        return std::unexpected(ex.what());
    }

    return {};
}


} // namespace httpserver