/// @file    handler.tpp
/// @brief   Template implementation for HandlerFabricImpl.
/// @author  saintson (pan.aleksandr.off@gmail.com)
/// @date    17.02.2026
/// @copyright Copyright (c) 2026 saintson. All rights reserved.
///            Licensed under the GNU General Public License v3.0 (GPLv3).

namespace httpserver {


template<std::derived_from<IHandler> HandlerType>
HandlerFabricImpl<HandlerType>::HandlerFabricImpl(beast::http::verb method, std::string endpoint)
        : m_method{std::move(method)}, m_endpoint{std::move(endpoint)} {};

template<std::derived_from<IHandler> HandlerType>
HandlerFabricImpl<HandlerType>::HandlerFabricImpl(std::string endpoint)
        : m_method{std::nullopt}, m_endpoint{std::move(endpoint)} {};

template<std::derived_from<IHandler> HandlerType>
Handler HandlerFabricImpl<HandlerType>::create() const {
    return std::make_shared<HandlerType>();
};

template<std::derived_from<IHandler> HandlerType>
std::string_view HandlerFabricImpl<HandlerType>::endpoint() const {
    return m_endpoint;
};

template<std::derived_from<IHandler> HandlerType>
std::optional<beast::http::verb> HandlerFabricImpl<HandlerType>::method() const {
    return m_method;
};


} // namespace httpserver
