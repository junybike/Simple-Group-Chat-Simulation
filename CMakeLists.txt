cmake_minimum_required(VERSION 3.14)

project(GroupChat VERSION 1.0 DESCRIPTION "Simple group chat" LANGUAGES C)

set(THREAD_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

file(GLOB SERVER "src/server.c")
file(GLOB CLIENT "src/client.c")

add_executable(server ${SERVER})
add_executable(client ${CLIENT})

target_link_libraries(client PRIVATE Threads::Threads rt)
