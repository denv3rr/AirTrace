cmake_minimum_required(VERSION 3.10)

# Project name
project(AirTrace VERSION 1.0)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Define the executable target and source files
add_executable(AirTrace
        src/main.cpp
        src/menu.cpp
        src/simulation.cpp
        src/inputValidation.cpp
        src/Object.cpp
        src/Tracker.cpp
        src/PathCalculator.cpp
        src/PredictionAlgorithm.cpp
        src/KalmanFilter.cpp
        src/HeatSignature.cpp
        src/DeadReckoning.cpp
)

# Include directories for headers
target_include_directories(AirTrace PUBLIC include)
