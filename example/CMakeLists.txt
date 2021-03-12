cmake_minimum_required(VERSION 3.16)
project(parcae_example)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(SOURCE_PARCAE_EXAMPLE
    main.cpp
    )

add_executable(parcae_example ${SOURCE_PARCAE_EXAMPLE})

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(parcae_example PRIVATE Threads::Threads parcae)


