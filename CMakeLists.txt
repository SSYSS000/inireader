cmake_minimum_required(VERSION 3.0)

project(inireader VERSION 0.0.1 LANGUAGES C)

include(CTest)

add_compile_definitions(
    VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
    VERSION_MINOR=${PROJECT_VERSION_MINOR}
    VERSION_PATCH=${PROJECT_VERSION_PATCH}
    VERSION_STRING="${PROJECT_VERSION}")

add_library(${PROJECT_NAME} STATIC
    inireader.c)

set_target_properties(${PROJECT_NAME} PROPERTIES PUBLIC_HEADER "inireader.h")

install(TARGETS ${PROJECT_NAME})

add_subdirectory(examples)
add_subdirectory(tests)
