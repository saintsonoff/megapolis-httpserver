set(TARGET_NAME handler)

find_package(Boost REQUIRED CONFIG COMPONENTS system)

add_library(
    ${TARGET_NAME}
STATIC
    notfoundhandler.cpp
    infohandler.cpp
    fileuploadhandler.cpp
)

target_link_libraries(
${TARGET_NAME}
    PUBLIC
Boost::boost
Boost::system
    PUBLIC
utility
)

target_include_directories(
${TARGET_NAME}
    PUBLIC
${CMAKE_CURRENT_SOURCE_DIR}
)
