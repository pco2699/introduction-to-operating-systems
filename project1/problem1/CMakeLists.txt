cmake_minimum_required(VERSION 3.0.0)
project(problem1 VERSION 0.1.0 LANGUAGES C)

include(CTest)
enable_testing()

add_executable(problem1 main.c)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(problem1 PRIVATE Threads::Threads)