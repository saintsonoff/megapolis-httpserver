#include <boost/beast.hpp>
#include <boost/log/trivial.hpp>

#include <vector>
#include <memory>

// self
#include <root_checker.hpp>
#include <router.hpp>
#include <server.hpp>
#include <handler.hpp>

#include <infohandler.hpp>
#include <fileuploadhandler.hpp>


int main() {
    using namespace httpserver;
    enum { SUCCESSFULL, SERVER_START_ERROR, NOT_ROOT_EXECUTION };

    if (!is_root_execution()) {
        BOOST_LOG_TRIVIAL(error) << "execution is not root";
        return NOT_ROOT_EXECUTION;
    }

    Server server{1616,
        Router{std::vector<HandlerFabric>{
            std::make_shared<HandlerFabricImpl<InfoHandler>>("/info"),
            std::make_shared<HandlerFabricImpl<FileUploadHandler>>(beast::http::verb::post, "/upload")
        }}
    };

    if (auto result = server.run(); !result) {
        BOOST_LOG_TRIVIAL(error) << result.error();
        return SERVER_START_ERROR;
    }

    return SUCCESSFULL;
}