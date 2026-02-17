set(TARGET_NAME httpserver)

find_package(Boost REQUIRED CONFIG COMPONENTS log log_setup system)


add_executable(${TARGET_NAME} main.cpp)

target_link_libraries(
    ${TARGET_NAME}
PRIVATE
    root_checker
    utility
    server
    parallelhandler
    fileuploadhandler
    infohandler
    loghandler
)
