set(TARGET_NAME router)

find_package(Boost REQUIRED CONFIG COMPONENTS system)

add_library(${TARGET_NAME} INTERFACE)

target_link_libraries(
${TARGET_NAME}
    INTERFACE
Boost::boost
Boost::system
    INTERFACE
handler
notfoundhandler
utility
)

target_include_directories(
${TARGET_NAME}
    INTERFACE
${CMAKE_CURRENT_SOURCE_DIR}
)
