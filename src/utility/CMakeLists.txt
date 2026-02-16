set(TARGET_NAME utility)

find_package(Boost REQUIRED CONFIG COMPONENTS log log_setup system)

add_library(${TARGET_NAME} INTERFACE)

target_link_libraries(
${TARGET_NAME}
    INTERFACE
Boost::log Boost::log_setup
)

target_include_directories(
${TARGET_NAME}
    INTERFACE
${CMAKE_CURRENT_SOURCE_DIR}
)