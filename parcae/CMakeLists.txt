cmake_minimum_required(VERSION 3.16)

project(parcae VERSION 0.0.1)

add_library(parcae INTERFACE)
target_sources(parcae INTERFACE types.h node.h parcae.h)

target_include_directories(parcae INTERFACE
    "${PROJECT_SOURCE_DIR}"
)

