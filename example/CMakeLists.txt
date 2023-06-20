

add_executable(example
        common.h
        main.cpp
)

target_include_directories(example PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include)
