
add_executable(graph_test
        femto_test.hpp
        test_graphs.hpp
        test.cpp
        classification.cpp
        state_machine.cpp
        graph_traits.cpp
        graph_traits_retrieval.cpp
        graph_traits_queries.cpp
        connected.cpp
        topology.cpp
        )

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR ${CMAKE_CXX_COMPILER_ID} STREQUAL "AppleClang")
    target_compile_options(graph_test INTERFACE
            $<BUILD_INTERFACE:
            "-Wfatal-errors" # stops on first error
            "-Wall" # enables all the warnings about constructions
            "-Wextra" # Print extra warning messages"
            "-Werror" # Make all warnings into errors.
            "-pedantic" # Issue all the warnings demanded by strict ISO C and ISO C++
            "-pedantic-errors" # Like -pedantic, except that errors are produced rather than warnings
            >
            )

    if (NOT ${BXLX_GRAPH_USE_EXCEPTIONS})
        target_compile_options(graph_test INTERFACE
                $<BUILD_INTERFACE:"-fno-exceptions"> # compiles without exception support
                )
    endif()
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W3 /EHsc /Zc:__cplusplus")
else()
    message(WARNING "Unsupported compiler!")
endif()

# TODO make work add_dependency
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ftemplate-backtrace-limit=0 -ferror-limit=1")
get_target_property(graph_include bxlx.graph INTERFACE_INCLUDE_DIRECTORIES)
target_include_directories(graph_test PRIVATE ${graph_include})

add_test(graph_test graph_test)