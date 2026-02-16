#include "root_checker.hpp"

// stl
#include <utility>

#ifdef __unix__
// unix
#include <unistd.h>
#endif


namespace httpserver {


bool is_root_execution() {
#ifdef __unix__
    return geteuid() == 0;
#else
    static_assert(false);
#endif

}


} // namespace httpserver