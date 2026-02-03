cmake_minimum_required(VERSION 3.10)

project(AirTraceAdapterTemplate VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Template adapter target. Copy and rename for official adapters.
add_library(airtrace_adapter_template
    src/adapter_sdk.cpp
)

target_include_directories(airtrace_adapter_template PUBLIC include)
target_include_directories(airtrace_adapter_template PRIVATE ../../include)

# Link to core contracts only. Tools/UI are not required in adapters.
target_link_libraries(airtrace_adapter_template PRIVATE airtrace_core)
