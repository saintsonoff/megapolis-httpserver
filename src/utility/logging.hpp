#pragma once

#include <boost/log/trivial.hpp>
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/attributes.hpp>

inline void init_logging() {
    namespace log = boost::log;
    log::add_common_attributes();

    log::add_console_log(
        std::clog,
        log::keywords::format = (
            log::expressions::stream
                << "[" << log::expressions::format_date_time<boost::posix_time::ptime>("TimeStamp", "%Y-%m-%d %H:%M:%S") << "] "
                << "[" << log::expressions::attr<log::attributes::current_thread_id::value_type>("ThreadID") << "] "
                << "[" << log::trivial::severity << "] "
                << log::expressions::smessage
        )
    );
#ifdef NDEBUG
    log::core::get()->set_filter(log::trivial::severity >= log::trivial::info);
#else
    log::core::get()->set_filter(log::trivial::severity >= log::trivial::debug);
#endif
}


namespace httpserver {


namespace details{
    inline const struct LogInit {
        LogInit() { init_logging(); }
    } log_init;
} // details


} // httpserver