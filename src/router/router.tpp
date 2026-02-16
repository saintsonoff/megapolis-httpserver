namespace httpserver {


template<IsHandlerFabricsRange HandlerFabricsRangeType>
Router<HandlerFabricsRangeType>::Router(HandlerFabricsRangeType range) : handlers_fabric(std::move(range)) {};


template<IsHandlerFabricsRange HandlerFabricsRangeType>
Handler Router<HandlerFabricsRangeType>::route(const RequestContext& req) const {
    auto& msg = req.parser->get();
    auto it = std::ranges::find_if(handlers_fabric,
        [&msg](const auto& handle) {
            if (handle->endpoint() != msg.target()) {
                return false;
            }
            auto method = handle->method();
            if (method) {
                return *method == msg.method();
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