/// @file    main.cpp
/// @brief   Application entry point – configures routes and starts the HTTP server.
/// @author  saintson (pan.aleksandr.off@gmail.com)
/// @date    17.02.2026
/// @copyright Copyright (c) 2026 saintson. All rights reserved.
///            Licensed under the GNU General Public License v3.0 (GPLv3).

// stl
#include <vector>
#include <memory>
#include <thread>
#include <cstddef>

// boost
#include <boost/beast.hpp>
#include <boost/log/trivial.hpp>

// self
#include <logging.hpp>
#include <root_checker.hpp>
#include <router.hpp>
#include <server.hpp>
#include <handler.hpp>

#include <infohandler.hpp>
#include <fileuploadhandler.hpp>
#include <parallelhandler.hpp>
#include <loghandler.hpp>


int main() {
    using namespace httpserver;
    enum { SUCCESSFULL, SERVER_START_ERROR, NOT_ROOT_EXECUTION };

    if (!is_root_execution()) {
        BOOST_LOG_TRIVIAL(error) << "execution is not root";
        return NOT_ROOT_EXECUTION;
    }

    const std::size_t kUploadConcurrencySize = std::thread::hardware_concurrency() - 1;

    Server server{1616,
        Router{std::vector<HandlerFabric>{
            std::make_shared<HandlerFabricImpl<InfoHandler>>("/info"),
            std::make_shared<HandlerFabricImpl<LogHandler>>(beast::http::verb::get, "/log"),
            std::make_shared<ParallelHandlerFabric>(
                std::make_shared<HandlerFabricImpl<FileUploadHandler>>(beast::http::verb::post, "/upload"),
                std::size_t{kUploadConcurrencySize}
            )
        }}
    };

    if (auto result = server.run(); !result) {
        BOOST_LOG_TRIVIAL(error) << result.error();
        return SERVER_START_ERROR;
    }

    return SUCCESSFULL;
}