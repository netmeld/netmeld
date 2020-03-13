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

get_tool_name(${CMAKE_CURRENT_SOURCE_DIR} TOOL_NAME)
get_git_timestamp(${TOOL_NAME}.cpp CODE_VERSION)

add_definitions(
  -DPROGRAM_NAME="${TOOL_NAME}"
  -DPROGRAM_VERSION="${TOOL_VERSION}.${CODE_VERSION}"
  )

add_executable(${TOOL_NAME}
  NseResult.cpp
  SshAlgorithm.cpp
  SshPublicKey.cpp
  TracerouteHop.cpp
  ${TOOL_NAME}.cpp
  )

target_link_libraries(${TOOL_NAME}
  netmeld-core
  ${Boost_LIBRARIES}
  pugixml
  pqxx
  )

create_man_from_help(${TOOL_NAME})

install_bin(${TOOL_NAME} ${PROJECT_NAME})
install_man(${TOOL_NAME} ${PROJECT_NAME})
