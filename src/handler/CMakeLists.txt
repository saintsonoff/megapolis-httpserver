# handler library

set(TARGET_NAME handler)

find_package(Boost REQUIRED CONFIG COMPONENTS system)

add_library(
    ${TARGET_NAME}
INTERFACE
)

target_link_libraries(
${TARGET_NAME}
    INTERFACE
Boost::boost
Boost::system
    INTERFACE
utility
)

target_include_directories(
${TARGET_NAME}
    INTERFACE
${CMAKE_CURRENT_SOURCE_DIR}
)


# info handler
set(TARGET_NAME infohandler)

add_library(
    ${TARGET_NAME}
STATIC
    infohandler.cpp
)

target_link_libraries(
${TARGET_NAME}
    PUBLIC
Boost::boost
Boost::system
    PUBLIC
utility
handler
)

target_include_directories(
${TARGET_NAME}
    PUBLIC
${CMAKE_CURRENT_SOURCE_DIR}
)

# notfound handler
set(TARGET_NAME notfoundhandler)

add_library(
    ${TARGET_NAME}
STATIC
    notfoundhandler.cpp
)

target_link_libraries(
${TARGET_NAME}
    PUBLIC
Boost::boost
Boost::system
    PUBLIC
utility
handler
)

target_include_directories(
${TARGET_NAME}
    PUBLIC
${CMAKE_CURRENT_SOURCE_DIR}
)

# file upload handler
set(TARGET_NAME fileuploadhandler)

add_library(
    ${TARGET_NAME}
STATIC
    fileuploadhandler.cpp
)

target_link_libraries(
${TARGET_NAME}
    PUBLIC
Boost::boost
Boost::system
    PUBLIC
utility
handler
)

target_include_directories(
${TARGET_NAME}
    PUBLIC
${CMAKE_CURRENT_SOURCE_DIR}
)
