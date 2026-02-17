set(TARGET_NAME root_checker)

add_library(${TARGET_NAME} root_checker.cpp)

target_include_directories(
${TARGET_NAME}
    INTERFACE
${CMAKE_CURRENT_SOURCE_DIR}
)
