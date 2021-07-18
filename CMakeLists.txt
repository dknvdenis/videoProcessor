cmake_minimum_required(VERSION 3.5)

project(videoProcessor LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(${PROJECT_NAME} main.cpp
    log.h
    httpServer.cpp httpServer.h
    app.cpp app.h
    httpLexer.cpp httpLexer.h
    charIter.h
    streamReader.cpp streamReader.h
    vpException.h
)
