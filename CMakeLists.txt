#
# Copyright (c) 2022-2022 Bela Schaum (schaumb at gmail dot com)
#
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#

cmake_minimum_required(VERSION 3.0)

project(BXLX.graph_traits LANGUAGES CXX VERSION 1.0.0)

option(BXLX_GRAPH_TRAITS_USE_EXCEPTIONS   "Do not disable exceptions" ON)

set(CXX_STANDARD 17 CACHE STRING "C++ Standard Version. [17|20|23]")
set(CMAKE_CXX_STANDARD ${CXX_STANDARD})

add_library(${PROJECT_NAME} INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include/bxlx/graph_traits.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/bxlx/graph_traits/graph_traits_impl.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/bxlx/graph_traits/iterable_alg.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/bxlx/graph_traits/algs/breadth_first_search.hpp
)

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR ${CMAKE_CXX_COMPILER_ID} STREQUAL "AppleClang")
    target_compile_options(${PROJECT_NAME} INTERFACE
            $<BUILD_INTERFACE:
            "-Wfatal-errors" # stops on first error
            "-Wall" # enables all the warnings about constructions
            "-Wextra" # Print extra warning messages"
            "-Werror" # Make all warnings into errors.
            "-pedantic" # Issue all the warnings demanded by strict ISO C and ISO C++
            "-pedantic-errors" # Like -pedantic, except that errors are produced rather than warnings
            >
            )

    if (NOT ${BXLX_GRAPH_TRAITS_USE_EXCEPTIONS})
        target_compile_options(${PROJECT_NAME} INTERFACE
                $<BUILD_INTERFACE:"-fno-exceptions"> # compiles without exception support
                )
    endif()
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W3 /EHsc /Zc:__cplusplus")

    target_compile_definitions(${PROJECT_NAME}
            INTERFACE NOMINMAX) # avoid Win macro definition of min/max, use std one
else()
    message(WARNING "Unsupported compiler!")
endif()


add_library(bxlx::graph_traits ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        )


# install support
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

configure_package_config_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/bxlxConfig.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/bxlxConfig.cmake
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/bxlx
)

write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/bxlxConfigVersion.cmake
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion
)

install(TARGETS ${PROJECT_NAME}
        EXPORT ${PROJECT_NAME}Targets
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        )

install(EXPORT ${PROJECT_NAME}Targets
        NAMESPACE bxlx::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/bxlx
        )

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/bxlx
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        )

install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/bxlxConfigVersion.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/bxlxConfig.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/bxlx
        )
