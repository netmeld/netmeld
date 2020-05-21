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
#add_custom_target(${PROJECT_NAME})


###############################################################################
# Test hooks
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


###############################################################################
# Add library data
###############################################################################
add_library(${PROJECT_NAME} SHARED
    ./objects/AbstractObject.cpp
    ./objects/AbstractDatastoreObject.cpp
    ./objects/AcNetworkBook.cpp
    ./objects/AcServiceBook.cpp
    ./objects/AcRule.cpp
    ./objects/Cve.cpp
    ./objects/DeviceInformation.cpp
    ./objects/Interface.cpp
    ./objects/InterfaceNetwork.cpp
    ./objects/IpAddress.cpp
    ./objects/IpNetwork.cpp
    ./objects/MacAddress.cpp
    ./objects/OperatingSystem.cpp
    ./objects/Port.cpp
    ./objects/Route.cpp
    ./objects/Service.cpp
    ./objects/Time.cpp
    ./objects/ToolObservations.cpp
    ./objects/Uuid.cpp
    ./objects/Vlan.cpp

    ./tools/AbstractExportTool.cpp
    ./tools/AbstractGraphTool.cpp
    ./tools/AbstractInsertTool.cpp
    ./tools/AbstractTool.cpp

    ./utils/CmdExec.cpp
    ./utils/FileManager.cpp
    ./utils/ForkExec.cpp
    ./utils/Logger.cpp
    ./utils/LoggerSingleton.cpp
    ./utils/ProgramOptions.cpp
    ./utils/QueriesCommon.cpp
    ./utils/Severity.cpp
    ./utils/StreamUtilities.cpp
    ./utils/StringUtilities.cpp
  )

target_include_directories(${PROJECT_NAME} PUBLIC
    ${Boost_INCLUDE_DIRS}
    include
  )

set_target_properties(${PROJECT_NAME}
    PROPERTIES VERSION ${TOOL_VERSION}
  )

target_link_libraries(${PROJECT_NAME}
    ${Boost_LIBRARIES}
    pqxx
  )

install_lib(${PROJECT_NAME} ${PROJECT_NAME})
install_man(${PROJECT_NAME} ${PROJECT_NAME})
add_tool_suite_man(${PROJECT_NAME})

install (
    DIRECTORY
      objects
      parsers
      tools
      utils
    OPTIONAL
    COMPONENT ${PROJECT_NAME}
    DESTINATION include/netmeld/core
    FILES_MATCHING PATTERN "*.[hi]pp"
  )

add_subdirectory(objects)
add_subdirectory(parsers)
add_subdirectory(schemas)
add_subdirectory(tools)
add_subdirectory(utils)
