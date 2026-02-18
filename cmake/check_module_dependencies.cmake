if(NOT DEFINED ROOT_DIR)
    message(FATAL_ERROR "ROOT_DIR is required.")
endif()

set(MODULE_DEP_VIOLATIONS "")

macro(check_module module_name allowed_deps)
    set(module_dirs ${ARGN})
    foreach(module_dir IN LISTS module_dirs)
        if(NOT EXISTS "${module_dir}")
            continue()
        endif()

        file(
            GLOB_RECURSE module_files
            LIST_DIRECTORIES false
            "${module_dir}/*.h"
            "${module_dir}/*.hpp"
            "${module_dir}/*.cpp"
        )

        foreach(module_file IN LISTS module_files)
            file(READ "${module_file}" module_content)
            string(REPLACE "\r\n" "\n" module_content "${module_content}")
            string(REPLACE "\n" ";" module_lines "${module_content}")
            set(module_line_no 0)

            foreach(module_line IN LISTS module_lines)
                math(EXPR module_line_no "${module_line_no} + 1")

                if(module_line MATCHES "^[ \t]*#[ \t]*include[ \t]*[<\"]([^\">]+)[\">]")
                    set(module_header "${CMAKE_MATCH_1}")
                    set(module_dep "")

                    if(module_header MATCHES "^core/")
                        set(module_dep "core")
                    elseif(module_header MATCHES "^tools/")
                        set(module_dep "tools")
                    elseif(module_header MATCHES "^ui/")
                        set(module_dep "ui")
                    endif()

                    if(NOT module_dep STREQUAL "")
                        set(module_allowed ${allowed_deps})
                        list(FIND module_allowed "${module_dep}" allowed_index)
                        if(allowed_index EQUAL -1)
                            file(RELATIVE_PATH rel_path "${ROOT_DIR}" "${module_file}")
                            string(
                                APPEND MODULE_DEP_VIOLATIONS
                                "${rel_path}:${module_line_no} includes '${module_header}' but module '${module_name}' may only include [${allowed_deps}]\n"
                            )
                        endif()
                    endif()
                endif()
            endforeach()
        endforeach()
    endforeach()
endmacro()

check_module("core" "core"
    "${ROOT_DIR}/include/core"
    "${ROOT_DIR}/src/core"
)
check_module("tools" "core;tools"
    "${ROOT_DIR}/include/tools"
    "${ROOT_DIR}/src/tools"
)
check_module("ui" "core;tools;ui"
    "${ROOT_DIR}/include/ui"
    "${ROOT_DIR}/src/ui"
)
check_module("adapters" "core"
    "${ROOT_DIR}/adapters/sdk/include"
    "${ROOT_DIR}/adapters/sdk/src"
)

if(NOT MODULE_DEP_VIOLATIONS STREQUAL "")
    message(FATAL_ERROR "Module dependency check failed:\n${MODULE_DEP_VIOLATIONS}")
endif()

message(STATUS "Module dependency boundaries are valid.")
