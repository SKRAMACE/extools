cmake_minimum_required(VERSION 3.10)
project(logex)

include_directories(include)

# Add the examples directory
add_subdirectory(examples)

file(GLOB LOGEX_HEADERS "include/logex*.h")

add_custom_target(install-header
    COMMAND ${CMAKE_COMMAND} -E make_directory /usr/local/include/extools
    COMMAND ${CMAKE_COMMAND} -E copy ${LOGEX_HEADERS} /usr/local/include/extools
    COMMENT "Installing logex-header files..."
)
