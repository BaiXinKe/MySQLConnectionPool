cmake_minimum_required(VERSION 3.0.0)
project(zero VERSION 0.1.0)

include(CTest)
enable_testing()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED 17)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

find_library(mysqlclient libmysqlclient.a)
find_library(mysqlcppconn libmysqlcppconn8.a)

find_package(Threads REQUIRED)

find_package(ZeroMQ REQUIRED)
include_directories(${ZeroMQ_INCLUDE_DIRS})


add_executable(zero main.cpp Connector.cpp mysqlConnectionPool.cpp)

target_link_libraries(zero ${ZeroMQ_Librar} ${Threads_Librar} mysqlclient mysqlcppconn )

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
