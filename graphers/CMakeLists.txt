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

get_tool_name(${CMAKE_CURRENT_SOURCE_DIR} PROJECT_NAME_SUFFIX)
set(PROJECT_NAME ${TOOL_SUITE}-${PROJECT_NAME_SUFFIX})
add_custom_target(${PROJECT_NAME})

###############################################################################
# Add template
###############################################################################
add_subdirectory(graph-tool-template)


###############################################################################
# Add binary data
###############################################################################
add_custom_target(Test.${PROJECT_NAME}
    DEPENDS ${PROJECT_NAME}
  )
function(create_test TEST_NAME)
  add_executable(Test.${TEST_NAME}
      EXCLUDE_FROM_ALL
      ${TEST_NAME}.test.cpp
    )

  target_link_libraries(Test.${TEST_NAME}
      netmeld-core
      ${Boost_LIBRARIES}
    )

  add_test(${TEST_NAME}
      Test.${TEST_NAME}
    )

  set_tests_properties(${TEST_NAME}
      PROPERTIES LABELS "Test.${PROJECT_NAME}"
    )

  add_dependencies(Test.${PROJECT_NAME}
      Test.${TEST_NAME}
    )
endfunction()

foreach(TOOL_NAME
    nmdb-graph-ac
    nmdb-graph-network
  )

  add_subdirectory(${TOOL_NAME})

  add_dependencies(${PROJECT_NAME}
      ${TOOL_NAME}
    )
endforeach(TOOL_NAME)
