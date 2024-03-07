cmake_minimum_required(VERSION 3.20)

project(vdf_parser)

include(cmake/CPM.cmake)

CPMAddPackage("gh:boost-ext/ut#v2.0.1")
CPMAddPackage("gh:taocpp/PEGTL#3.2.7")

add_library(vdf_parser INTERFACE)
add_library(cursey::vdf_parser ALIAS vdf_parser)
target_include_directories(vdf_parser INTERFACE .)
target_link_libraries(vdf_parser INTERFACE taocpp::pegtl)
target_compile_features(vdf_parser INTERFACE cxx_std_23)

add_executable(test test/main.cpp)
target_link_libraries(test PRIVATE vdf_parser Boost::ut)
target_compile_definitions(test PRIVATE BOOST_UT_DISABLE_MODULE)
