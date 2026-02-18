# SDK target for adapter development and independent module verification.
add_library(airtrace_adapters_sdk
    src/adapter_sdk.cpp
)

add_library(airtrace_adapter_template ALIAS airtrace_adapters_sdk)

target_include_directories(airtrace_adapters_sdk
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE ${PROJECT_SOURCE_DIR}/include
)

# Link to core contracts only. Tools/UI are not required in adapters.
target_link_libraries(airtrace_adapters_sdk PUBLIC airtrace_core)

set_target_properties(airtrace_adapters_sdk PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
)
