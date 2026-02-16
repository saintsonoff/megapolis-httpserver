namespace httpserver {


template<IsHandlerFabricsRange HandlerFabricsRangeType>
Router<HandlerFabricsRangeType>::Router(HandlerFabricsRangeType range) : handlers_fabric(std::move(range)) {};


template<IsHandlerFabricsRange HandlerFabricsRangeType>
Handler Router<HandlerFabricsRangeType>::route(const Request& req) const {
    auto it = std::ranges::find_if(handlers_fabric,
        [&req](const auto& handle) {
            if (handle->endpoint() != req.target()) {
                return false;
            }
            auto method = handle->method();
            if (method) {
                return *method == req.method();
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