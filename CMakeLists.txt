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

include(cmake/BuildEnvironment.cmake)
include(cmake/BuildHelpers.cmake)

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

find_package(nlohmann_json REQUIRED)

find_package(yaml-cpp REQUIRED)

# Appears to not be linked by default
link_libraries(stdc++fs)


# ----------------------------------------------------------------------
# Netmeld Configuration
# ----------------------------------------------------------------------
set(NETMELD_CONF_DIR "etc/netmeld")
set(NETMELD_DB_NAME "site")
set(NETMELD_SCHEMA_DIR "schemas")
set(NETMELD_IMAGE_DIR "images")

add_compile_definitions(
  NETMELD_CONF_DIR="${CMAKE_INSTALL_PREFIX}/${NETMELD_CONF_DIR}"
  NETMELD_DB_NAME="${NETMELD_DB_NAME}"
  NETMELD_SCHEMA_DIR="${NETMELD_SCHEMA_DIR}"
  NETMELD_IMAGE_DIR="${NETMELD_IMAGE_DIR}"
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
    tool-clw
  )
  target_as_module(${ITEM})
endforeach()


#set(TGT_MODULE "fake-module")
#add_custom_target("${TGT_MODULE}" DEPENDS "${TOOL_SUITE}")
#foreach(ITEM
#    clw
#  )
#  set(TGT_MODULE_TEST "Test.${ITEM}")
#  add_custom_target("${TGT_MODULE_TEST}" DEPENDS ${TGT_MODULE})
#  add_dependencies("${TEST_ALL}" ${TGT_MODULE_TEST})
#  target_as_tool(${ITEM})
#endforeach()

include(cmake/PackageCreation.cmake)
