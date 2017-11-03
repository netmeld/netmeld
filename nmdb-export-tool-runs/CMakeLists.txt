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

set(PROJECT_NAME nmdb-export-tool-runs)


add_definitions(
  -DPROGRAM_NAME="${PROJECT_NAME}"
  )

find_package(Boost REQUIRED
  filesystem
  program_options
  system
  )

include_directories(
  ${Boost_INCLUDE_DIRS}
  .
  )


set_pie_flags()

add_executable(${PROJECT_NAME}
  ${PROJECT_NAME}.cpp
  )

target_link_libraries(${PROJECT_NAME}
  netmeld-common
  ${Boost_LIBRARIES}
  pqxx
  )

add_custom_command(
  TARGET ${PROJECT_NAME} POST_BUILD
  COMMAND 
    ${HELP2MAN} ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}
      -I ${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}.h2m
      -I ${CMAKE_SOURCE_DIR}/man/Template.h2m
      --no-discard-stderr -N
      -o ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.1
  )

install(
  FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.1
  DESTINATION share/man/man1
  )

install(
  TARGETS ${PROJECT_NAME}
  DESTINATION bin
  )
