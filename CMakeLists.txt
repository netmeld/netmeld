# =============================================================================
# Copyright 2017 National Technology & Engineering Solutions of Sandia, LLC
# (NTESS). Under the terms of Contract DE-NA0003525 with NTESS, the U.S.
# Government retains certain rights in this software.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
# =============================================================================
# Maintained by Sandia National Laboratories <Netmeld@sandia.gov>
# =============================================================================

# ----------------------------------------------------------------------
# CMake Settings
# ----------------------------------------------------------------------

cmake_minimum_required(VERSION 3.0)
project(Netmeld)

enable_testing()


# ----------------------------------------------------------------------
# Build Environment Settings
# ----------------------------------------------------------------------

set(PROJECT_GNU_COMMON_FLAGS
  "-D_FORTIFY_SOURCE=2"
  "-Os"
  "-fdiagnostics-show-option"
  "-pedantic-errors"
  "-Wall"
  "-Wextra"
  "-Wformat=2"
  "-Werror=format-security"
  "-Wno-long-long"
  "-Wfatal-errors"
  "-Wcast-align"
  "-Wcast-qual"
  #"-Wconversion"
  "-Wsign-conversion"
  "-Wsign-compare"
  "-Wfloat-equal"
  "-Wlogical-op"
  "-Wmissing-include-dirs"
  "-Wmissing-declarations"
  "-Wredundant-decls"
  #"-Wshadow"
  "-Wswitch-default"
  #"-Wswitch-enum"
  "-Wundef"
  "-Wuninitialized"
  "-Winit-self"
  "-Wunreachable-code"
  "-Wwrite-strings"
  #"-Winline"
  "-Wpacked"
  #"-Wpadded"
  "-fstack-protector-all"
  "-Wstack-protector"
  "-ftrapv"
  "-Wl,-z,relro"
  "-Wl,-z,now"
  )

set(PROJECT_GNU_C_FLAGS
  "-std=c99"
  "-Wbad-function-cast"
  "-Wc++-compat"
  "-Wmissing-prototypes"
  "-Wstrict-prototypes"
  "-Wnested-externs"
  "-Wold-style-declaration"
  "-Wold-style-definition"
  "-Wtraditional-conversion"
  )

set(PROJECT_GNU_CXX_FLAGS
    # Following line needed to use boost libraries > 1.62 with cmake
    #"-DBOOST_PHOENIX_NO_VARIADIC_EXPRESSION"
  "-std=c++17"
  #"-Weffc++"
  "-Wctor-dtor-privacy"
  "-Wold-style-cast"
  "-Woverloaded-virtual"
  "-Wsign-promo"
  "-Wstrict-null-sentinel"
  #"-Wabi"
  )

set(CMAKE_SKIP_INSTALL_ALL_DEPENDENCY true)

include(CheckCCompilerFlag)
include(CheckCXXCompilerFlag)

function(encode_compiler_flag_as_token string_value result_var)
  string(STRIP "${string_value}" RESULT)
  string(REPLACE " " "_" RESULT "${RESULT}")
  string(REPLACE "," "_" RESULT "${RESULT}")
  string(REPLACE "." "_" RESULT "${RESULT}")
  string(REPLACE "=" "_" RESULT "${RESULT}")
  string(REPLACE "/" "_" RESULT "${RESULT}")
  string(REPLACE "-" "_" RESULT "${RESULT}")
  string(REPLACE "+" "x" RESULT "${RESULT}")
  set(${result_var} ${RESULT} PARENT_SCOPE)
endfunction()

function(set_pie_flags)
  if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)  # GNU C/C++
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIE -pie -fPIC" PARENT_SCOPE)
  endif()
endfunction()

function(add_project project_dir)
  set(full_path "${CMAKE_CURRENT_SOURCE_DIR}/${project_dir}")
  if(EXISTS "${full_path}" AND EXISTS "${full_path}/CMakeLists.txt")
    add_subdirectory("${project_dir}")
    add_dependencies(Test.${TEST_ALL}
        Test.${TEST_ALL}-${project_dir}
      )
  endif()
endfunction()

function(get_tool_name cwd_var result_var)
  execute_process(
      COMMAND
        basename "${cwd_var}"
      OUTPUT_VARIABLE RESULT_STRIPPED
      OUTPUT_STRIP_TRAILING_WHITESPACE
      ERROR_QUIET
  )
  set(${result_var} ${RESULT_STRIPPED} PARENT_SCOPE)
endfunction()

function(get_git_timestamp target_file result_var)
  execute_process(
      COMMAND
          date +%Y%m%d
      OUTPUT_VARIABLE RESULT_DATE
      OUTPUT_STRIP_TRAILING_WHITESPACE
      ERROR_QUIET
  )
  execute_process(
      COMMAND
          git log -1 --format=%ad --date=format:%Y%m%d
            -- ${CMAKE_CURRENT_SOURCE_DIR}/${target_file}
      OUTPUT_VARIABLE RESULT_GIT
      OUTPUT_STRIP_TRAILING_WHITESPACE
      ERROR_QUIET
  )
  if(RESULT_GIT)
    set(RESULT ${RESULT_GIT})
  elseif(RESULT_DATE)
    set(RESULT ${RESULT_DATE})
  else()
    set(RESULT 0)
  endif()
  set(${result_var} ${RESULT} PARENT_SCOPE)
endfunction()

function(create_man_from_help tool_name)
  if(HELP2MAN AND GZIP)
    set(man_file "${CMAKE_CURRENT_BINARY_DIR}/${tool_name}.1")
    set(readme_file "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
    set(h2m_file "${CMAKE_CURRENT_BINARY_DIR}/${tool_name}.h2m")
    if(PANDOC AND SED AND EXISTS "${readme_file}")
      add_custom_command(
        TARGET ${tool_name}
        COMMAND
          ${PANDOC} -f gfm -t man
            --output ${h2m_file} ${readme_file}
        COMMAND
          bash -c "sed -i -e 's/.SH \\(.*\\)/[=\\1]/g' ${h2m_file}"
        VERBATIM
      )
    endif()
    add_custom_command(
      TARGET ${tool_name}
      DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${tool_name}
      COMMAND
        ${HELP2MAN} ${CMAKE_CURRENT_BINARY_DIR}/${tool_name}
          -I ${h2m_file}
          --no-discard-stderr -N
          -o ${man_file}
      COMMAND
        ${GZIP} -f ${man_file}
    )
  endif()
endfunction()

function(add_tool_suite_man target_comp)
  if(PANDOC AND GZIP)
    set(man_file "${CMAKE_CURRENT_BINARY_DIR}/${TOOL_SUITE}.7")
    add_custom_command(
      TARGET ${target_comp}
      COMMAND
        ${PANDOC} --standalone -f gfm -t man
          --variable=title:${TOOL_SUITE}
          --variable=header:"Netmeld Tool Suite"
          --variable=section:7
          --variable=footer:${TOOL_SUITE}
          --output ${man_file}
          ${CMAKE_SOURCE_DIR}/README.md
          ${CMAKE_SOURCE_DIR}/LICENSE
      COMMAND
        ${GZIP} -f ${man_file}
    )
    install(
      FILES "${man_file}.gz"
      OPTIONAL
      COMPONENT ${target_comp}
      DESTINATION share/man/man7
    )
  endif()
endfunction()

function(install_lib target_file component)
  install(
    TARGETS ${target_file}
    OPTIONAL
    COMPONENT ${component}
    DESTINATION lib
  )
endfunction()

function(install_bin target_file component)
  install(
    TARGETS ${target_file}
    OPTIONAL
    COMPONENT ${component}
    DESTINATION bin
  )
endfunction()

function(install_man target_file component)
  install(
    FILES "${CMAKE_CURRENT_BINARY_DIR}/${target_file}.1.gz"
    OPTIONAL
    COMPONENT ${component}
    DESTINATION share/man/man1
  )
endfunction()

if(CMAKE_COMPILER_IS_GNUCC)  # GNU C compiler (gcc)
  foreach(LINE ${PROJECT_GNU_ASM_FLAGS})
    set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} ${LINE}")
  endforeach(LINE)
endif()

if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)  # GNU C/C++
  set(CMAKE_EXE_LINKER_FLAGS_INIT "${CMAKE_EXE_LINKER_FLAGS_INIT} -fPIE -fPIC")
endif()

if(CMAKE_COMPILER_IS_GNUCC)  # GNU C compiler (gcc)
  foreach(LINE ${PROJECT_GNU_MACHINE_FLAGS})
    # Without checking, add each machine flag to CMAKE_C_FLAGS.
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${LINE}")
  endforeach(LINE)
  foreach(LINE ${PROJECT_GNU_COMMON_FLAGS} ${PROJECT_GNU_C_FLAGS})
    # Check whether each requested flag is supported by this gcc.
    encode_compiler_flag_as_token("${LINE}" FLAG)
    check_c_compiler_flag("-Werror ${LINE}" GNUCC_SUPPORTS_FLAG_${FLAG})
    if(GNUCC_SUPPORTS_FLAG_${FLAG})
      # Add each supported flag to CMAKE_C_FLAGS.
      set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${LINE}")
    endif()
  endforeach(LINE)
endif()

if(CMAKE_COMPILER_IS_GNUCXX)  # GNU C++ compiler (g++)
  foreach(LINE ${PROJECT_GNU_MACHINE_FLAGS})
    # Without checking, add each machine flag to CMAKE_CXX_FLAGS.
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${LINE}")
  endforeach(LINE)
  foreach(LINE ${PROJECT_GNU_COMMON_FLAGS} ${PROJECT_GNU_CXX_FLAGS})
    # Check whether each requested flag is supported by this g++.
    encode_compiler_flag_as_token("${LINE}" FLAG)
    check_cxx_compiler_flag("-Werror ${LINE}" GNUCXX_SUPPORTS_FLAG_${FLAG})
    if(GNUCXX_SUPPORTS_FLAG_${FLAG})
      # Add each supported flag to CMAKE_CXX_FLAGS.
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${LINE}")
    endif()
  endforeach(LINE)
endif()

# Print the compiler and linker flags for verification purposes.
message("CMAKE_C_FLAGS   = ${CMAKE_C_FLAGS}")
message("CMAKE_CXX_FLAGS = ${CMAKE_CXX_FLAGS}")

message("CMAKE_EXE_LINKER_FLAGS = ${CMAKE_EXE_LINKER_FLAGS}")
message("CMAKE_C_COMPILE_OPTIONS_PIE = ${CMAKE_C_COMPILE_OPTIONS_PIE}")
message("CMAKE_CXX_COMPILE_OPTIONS_PIE = ${CMAKE_CXX_COMPILE_OPTIONS_PIE}")

# ----------------------------------------------------------------------

set(NETMELD_CONF_DIR "etc/netmeld")

add_definitions(
  -DNETMELD_CONF_DIR="${CMAKE_INSTALL_PREFIX}/${NETMELD_CONF_DIR}"
  -DDEFAULT_DB_NAME="site"
  )

# ----------------------------------------------------------------------
# Libraries that many things need to link against for various reasons
# ----------------------------------------------------------------------

find_package(Boost REQUIRED
  date_time
  iostreams
  program_options
  system
  unit_test_framework
  )

# Appears to not be linked by default
link_libraries(stdc++fs)

find_program(HELP2MAN help2man)
if (NOT HELP2MAN)
  message(WARNING "help2man not found, docs will not generate.")
endif()

find_program(PANDOC pandoc)
if (NOT PANDOC)
  message(WARNING "pandoc not found, docs will not fully generate.")
endif()

find_program(SED sed)
if (NOT SED)
  message(WARNING "sed not found, docs will not fully generate.")
endif()

find_program(GZIP gzip)
if (NOT GZIP)
  message(WARNING "gzip not found, docs will not generate.")
endif()

find_program(GIT git)
if (NOT GIT)
  message(WARNING "git not found, versioning will be wrong.")
endif()
find_program(DATE date)
if (NOT DATE)
  message(WARNING "date not found, versioning will be wrong.")
endif()

# ----------------------------------------------------------------------

get_tool_name(${CMAKE_CURRENT_SOURCE_DIR} TOOL_SUITE)
set(TEST_ALL ${TOOL_SUITE})
add_custom_target(Test.${TEST_ALL})

#create_man_from_readme(${TOOL_SUITE})
#install_man(${TOOL_SUITE} ${TOOL_SUITE})

set(NETMELD_SOURCE_VERSION "2.0")

set(NETMELD_CORE_VERSION "2.0")
set(TOOL_VERSION ${NETMELD_CORE_VERSION})
add_project(core)

set(NETMELD_CORE_BIN_VERSION "2.0")
set(TOOL_VERSION ${NETMELD_CORE_VERSION})
add_project(core-bin)

set(NETMELD_INSERTERS_VERSION "2.0")
set(TOOL_VERSION ${NETMELD_INSERTERS_VERSION})
add_project(inserters)

set(NETMELD_EXPORTERS_VERSION "2.0")
set(TOOL_VERSION ${NETMELD_EXPORTERS_VERSION})
add_project(exporters)

set(NETMELD_IMPORTERS_VERSION "2.0")
set(TOOL_VERSION ${NETMELD_IMPORTERS_VERSION})
add_project(importers)

set(NETMELD_GRAPHERS_VERSION "2.0")
set(TOOL_VERSION ${NETMELD_GRAPHERS_VERSION})
add_project(graphers)

set(NETMELD_FETCHERS_VERSION "2.0")
set(TOOL_VERSION ${NETMELD_FETCHERS_VERSION})
add_project(fetchers)

set(NETMELD_CLW_VERSION "2.0")
set(TOOL_VERSION ${NETMELD_CLW_VERSION})
add_project(clw)

set(NETMELD_PLAYBOOK_CORE_VERSION "2.0")
set(TOOL_VERSION ${NETMELD_PLAYBOOK_CORE_VERSION})
add_project(playbook-core)

set(NETMELD_PLAYBOOK_VERSION "2.0")
set(TOOL_VERSION ${NETMELD_PLAYBOOK_VERSION})
add_project(playbook)

set(NETMELD_DATALAKE_CORE_VERSION "2.0")
set(TOOL_VERSION ${NETMELD_DATALAKE_CORE_VERSION})
add_project(datalake-core)

set(NETMELD_DATALAKE_VERSION "2.0")
set(TOOL_VERSION ${NETMELD_DATALAKE_VERSION})
add_project(datalake)
