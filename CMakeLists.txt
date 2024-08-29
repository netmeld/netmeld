# =============================================================================
# Copyright 2023 National Technology & Engineering Solutions of Sandia, LLC
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

cmake_minimum_required(VERSION 3.18)
set(TOOL_SUITE "netmeld")
set(MODULE_NAME "netmeld")
project(${TOOL_SUITE})

enable_testing()

include(cmake/BuildEnvironment.cmake)
include(cmake/BuildHelpers.cmake)

# ----------------------------------------------------------------------
# Libraries that many things need to link against for various reasons
# ----------------------------------------------------------------------

set(BOOST_PKG_NAME_VERSION "1.83.0")
find_package(Boost REQUIRED
  date_time
  iostreams
  program_options
  system
  unit_test_framework
  )

find_package(nlohmann_json REQUIRED)

find_package(yaml-cpp REQUIRED)

# ----------------------------------------------------------------------
# 20230929 -- See: https://github.com/jtv/libpqxx/issues/732
#             Stop-gap until external library/compiler issues resolved
set(CMAKE_CXX_STANDARD 20)
set(SKIP_BUILD_TEST ON)
include(FetchContent)
FetchContent_Declare(
  libpqxx
  GIT_REPOSITORY https://github.com/jtv/libpqxx.git
#  GIT_TAG 21afb0d6ec3d31424df6b7aac04c6b20c55c0a65 # 7.8.1 tag
  GIT_TAG 1ca80b0e638f6182426c5b11255069cae4fbd542 # 7.9.2 tag
  GIT_PROGRESS YES
)
set(PQXX_LIBRARIES pqxx)
FetchContent_MakeAvailable(libpqxx)
# Silence warnings for this library we do not control
target_compile_options(pqxx PRIVATE "-w")
target_compile_options(pqxx INTERFACE "-w")

# ----------------------------------------------------------------------


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
    tool-cisco-type7-decode
    tool-clw
    tool-junos-type9-decode
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
