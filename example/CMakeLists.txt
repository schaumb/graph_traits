

add_executable(example
        common.h
        example1.cpp
        example2.cpp
        example3.cpp
        main.cpp
)

target_include_directories(example PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include)
