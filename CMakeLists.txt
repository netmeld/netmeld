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
#cmake_minimum_required(VERSION 3.13)
set(TOOL_SUITE "netmeld")
set(MODULE_NAME "netmeld")
project(${TOOL_SUITE})

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
# Helper Functions
# ----------------------------------------------------------------------

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


function(nm_add_generic target)
  set(full_path "${CMAKE_CURRENT_SOURCE_DIR}/${target}")
  if(EXISTS "${full_path}" AND EXISTS "${full_path}/CMakeLists.txt")
    add_subdirectory("${target}")
  endif()
endfunction()
function(target_as_module target)
  if(NOT TGT_MODULE)
    set(prior_tgt_module "${TOOL_SUITE}")
    set(TGT_MODULE "${target}")
    set(prior_tgt_module_test "${TEST_ALL}")
    set(TGT_MODULE_TEST "Test.${target}")
  else()
    set(prior_tgt_module "${TGT_MODULE}")
    set(TGT_MODULE "${TGT_MODULE}-${target}")
    set(prior_tgt_module_test "${TGT_MODULE_TEST}")
    set(TGT_MODULE_TEST "${TGT_MODULE_TEST}.${target}")
  endif()
  add_custom_target("${TGT_MODULE}" DEPENDS ${prior_tgt_module})
  add_custom_target("${TGT_MODULE_TEST}" DEPENDS ${TGT_MODULE})
  add_dependencies(${prior_tgt_module_test} ${TGT_MODULE_TEST})
  nm_add_generic("${target}")
  #cpack_add_component(${TGT_MODULE})
endfunction()
function(target_as_tool target)
  set(TGT_TOOL "${target}")
  set(TGT_TOOL_TEST "${TGT_MODULE_TEST}.${target}")
  get_timestamp(${target} TGT_VERSION)
  add_dependencies("${TGT_MODULE}" "${TGT_TOOL}")
  nm_add_generic("${target}")
endfunction()
function(target_as_library target)
  set(TGT_LIBRARY "${TOOL_SUITE}-${TGT_MODULE}")
  set(TGT_LIBRARY_TEST "${TGT_MODULE_TEST}")
  nm_add_generic("${target}")
endfunction()

# TODO figure out how to add tool/library value
function(nm_add_test target)
  set(test_target "${TGT_MODULE_TEST}.${target}")
  set(TGT_TEST "${test_target}" PARENT_SCOPE)
  add_executable(${test_target}
      EXCLUDE_FROM_ALL
      ${target}.test.cpp
    )
  target_link_libraries(${test_target}
      ${Boost_LIBRARIES}
    )
  set(test_name "${test_target}")
  add_test(${test_name} ${test_target})
  set_tests_properties(${test_name}
      PROPERTIES LABELS "${TGT_MODULE_TEST}"
    )
  add_dependencies(${TGT_MODULE_TEST} ${test_target})
endfunction()


function(get_basename cwd_var result_var)
  execute_process(
      COMMAND
        basename "${cwd_var}"
      OUTPUT_VARIABLE RESULT_STRIPPED
      OUTPUT_STRIP_TRAILING_WHITESPACE
      ERROR_QUIET
  )
  set(${result_var} ${RESULT_STRIPPED} PARENT_SCOPE)
endfunction()

function(get_timestamp target_file result_var)
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


function(nm_install_bin target_file)
  target_compile_definitions(${target_file}
    PUBLIC
      -DPROGRAM_NAME="${target_file}"
      -DPROGRAM_VERSION="${MODULE_VERSION}.${TGT_VERSION}"
    )
  install(
    TARGETS ${target_file}
    OPTIONAL
    COMPONENT ${TGT_MODULE}
    DESTINATION bin
  )
  create_man_from_help(${target_file})
  nm_install_man(${target_file})
endfunction()
function(nm_install_program target_file)
  install(
    PROGRAMS ${target_file}
    OPTIONAL
    COMPONENT ${TGT_MODULE}
    DESTINATION bin
  )
  create_man_from_help(${target_file})
  nm_install_man(${target_file})
endfunction()
function(nm_install_lib target_file)
  install(
    TARGETS ${target_file}
    OPTIONAL
    COMPONENT ${TGT_MODULE}
    DESTINATION lib
  )
  set_target_properties(${target_file}
      PROPERTIES
        VERSION "${MODULE_VERSION}.${TGT_VERSION}"
    )
endfunction()
function(nm_install_include target_dir target_path)
  install(
    DIRECTORY ${target_dir}
    OPTIONAL
    COMPONENT ${TGT_MODULE}
    DESTINATION include/netmeld/${target_path}
    FILES_MATCHING PATTERN "*.[hi]pp"
  )
  add_subdirectory(${target_dir})
endfunction()
function(nm_install_man target_file)
  install(
    FILES "${CMAKE_CURRENT_BINARY_DIR}/${target_file}.1.gz"
    OPTIONAL
    COMPONENT ${TGT_MODULE}
    DESTINATION share/man/man1
  )
endfunction()
function(nm_install_conf target_file target_path)
  install(
      FILES ${target_file}
      OPTIONAL
      COMPONENT ${TGT_MODULE}
      DESTINATION ${NETMELD_CONF_DIR}/${target_path}
    )
endfunction()


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


# ----------------------------------------------------------------------
# Netmeld Configuration
# ----------------------------------------------------------------------
set(NETMELD_CONF_DIR "etc/netmeld")

add_definitions(
  -DNETMELD_CONF_DIR="${CMAKE_INSTALL_PREFIX}/${NETMELD_CONF_DIR}"
  -DDEFAULT_DB_NAME="site"
  )

set(TOOL_SUITE "netmeld")
add_custom_target(${TOOL_SUITE})
set(TEST_ALL "Test.${TOOL_SUITE}")
add_custom_target(${TEST_ALL})

#create_man_from_readme(${TOOL_SUITE})
#install_man(${TOOL_SUITE} ${TOOL_SUITE})

set(NETMELD_SOURCE_VERSION "2.0")

foreach(ITEM
    core
    datalake
    datastore
    fetchers
    playbook
  )
  target_as_module(${ITEM})
endforeach()


set(TGT_MODULE "fake-module")
add_custom_target("${TGT_MODULE}" DEPENDS "${TOOL_SUITE}")
foreach(ITEM
    clw
  )
  set(TGT_MODULE_TEST "Test.${ITEM}")
  add_custom_target("${TGT_MODULE_TEST}" DEPENDS ${TGT_MODULE})
  add_dependencies("${TEST_ALL}" ${TGT_MODULE_TEST})
  target_as_tool(${ITEM})
endforeach()


# ----------------------------------------------------------------------
# Deb Creation
# ----------------------------------------------------------------------
set(CPACK_GENERATOR "DEB")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Netmeld@sandia.gov")

set(CPACK_DEBIAN_ENABLE_COMPONENT_DEPENDS ON)
set(CPACK_DEB_COMPONENT_INSTALL ON)

set(CPACK_DEBIAN_CORE_PACKAGE_DEPENDS "something")
set(CPACK_DEBIAN_DATALAKE_PACKAGE_DEPENDS "something2")
set(CPACK_COMPONENT_DATALAKE_DEPENDS "core")
set(CPACK_DEBIAN_DATASTORE_PACKAGE_DEPENDS "something3")
set(CPACK_DEBIAN_FETCHERS_PACKAGE_DEPENDS "something4")
set(CPACK_DEBIAN_PLAYBOOK_PACKAGE_DEPENDS "something5")

#set(CPACK_DEBIAN_PACKAGE_DEPENDS "something")
include(CPack)
