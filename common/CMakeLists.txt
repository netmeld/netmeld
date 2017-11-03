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

find_package(Boost REQUIRED
  date_time
  filesystem
  system
  )


include_directories(
  ${Boost_INCLUDE_DIRS}
  .
  )

add_library(netmeld-common SHARED
  acls.cpp
  cve.cpp
  fork_exec.cpp
  networking.cpp
  parser_networking.cpp
  piped_input.cpp
  queries_common.cpp
  )

set_target_properties(netmeld-common
  PROPERTIES
  VERSION 1.0.0
  )

target_link_libraries(netmeld-common
  ${Boost_LIBRARIES}
  pqxx
  )

install(
  TARGETS netmeld-common
  DESTINATION lib
  )

install(
  FILES
  ../include/netmeld/common/acls.hpp
  ../include/netmeld/common/cve.hpp
  ../include/netmeld/common/fork_exec.hpp
  ../include/netmeld/common/networking.hpp
  ../include/netmeld/common/parser_networking.hpp
  ../include/netmeld/common/piped_input.hpp
  ../include/netmeld/common/queries_common.hpp
  DESTINATION include/netmeld/common
  )

install(
  FILES
  ../include/netmeld/common/impl/networking.ipp
  DESTINATION include/netmeld/common/impl
  )
