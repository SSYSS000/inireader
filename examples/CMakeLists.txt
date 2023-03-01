cmake_minimum_required(VERSION 3.0)

file(GLOB files "*.c")

foreach(file ${files})
    get_filename_component(target ${file} NAME_WLE)
    add_executable(${target} ${file})
    target_link_libraries(${target} ${PROJECT_NAME})
endforeach()
