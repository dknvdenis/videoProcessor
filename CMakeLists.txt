cmake_minimum_required(VERSION 3.5)

project(videoProcessor LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCV 2.4 REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

find_package(OpenMP)
if(OPENMP_FOUND)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

add_executable(${PROJECT_NAME} main.cpp
    log.h
    httpServer.cpp httpServer.h
    app.cpp app.h
    httpLexer.cpp httpLexer.h
    charIter.h
    streamReader.cpp streamReader.h
    vpException.h
    parserUtils.cpp parserUtils.h
    token.cpp token.h
    tcpServer.cpp tcpServer.h
    iProcess.h
    copyProcess.cpp copyProcess.h
    drawHistogramProcess.cpp drawHistogramProcess.h
    brightnessGainProcess.cpp brightnessGainProcess.h
)

target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS})
