namespace httpserver {


template<IsHandlerFabricsRange HandlerFabricsRangeType>
Router<HandlerFabricsRangeType>::Router(HandlerFabricsRangeType range) : handlers_fabric(std::move(range)) {};


template<IsHandlerFabricsRange HandlerFabricsRangeType>
Handler Router<HandlerFabricsRangeType>::route(beast::http::request<boost::beast::http::empty_body> header) const {    
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


} // namespace httpserver