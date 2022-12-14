#
# Copyright (c) 2022-2022 Bela Schaum (schaumb at gmail dot com)
#
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#

cmake_minimum_required(VERSION 3.0)

project(bxlx.graph LANGUAGES CXX VERSION 1.0.0)

option(BXLX_GRAPH_USE_EXCEPTIONS    "Do not disable exceptions" ON)
option(BXLX_GRAPH_BUILD_TESTING     "Enable test compilation"   ON)

set(CXX_STANDARD 17 CACHE STRING "C++ Standard Version. [17|20|23]")
set(CMAKE_CXX_STANDARD ${CXX_STANDARD})

add_library(${PROJECT_NAME} INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include/bxlx/graph/traits.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/bxlx/graph/storage.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/bxlx/graph/bitset_helper.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/bxlx/graph/iterable_alg.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/bxlx/graph/algorithms/breadth_first_search.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/bxlx/graph/execution.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/bxlx/graph/traits/type_classification.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/bxlx/graph/traits/graph_traits.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/bxlx/graph/traits/node.hpp
)

add_library(bxlx::graph ALIAS ${PROJECT_NAME})

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

if(BXLX_GRAPH_BUILD_TESTING AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/test/CMakeLists.txt")
    add_subdirectory(test)
endif()