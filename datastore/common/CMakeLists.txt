# =============================================================================
# Copyright 2022 National Technology & Engineering Solutions of Sandia, LLC
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

add_library(${TGT_LIBRARY} SHARED
    ./objects/AbstractDatastoreObject.cpp
    ./objects/AcNetworkBook.cpp
    ./objects/AcServiceBook.cpp
    ./objects/AcRule.cpp
    ./objects/AclIpNetSet.cpp
    ./objects/AclPortSet.cpp
    ./objects/AclService.cpp
    ./objects/AclZone.cpp
    ./objects/AclRule.cpp
    ./objects/AclRulePort.cpp
    ./objects/AclRuleService.cpp
    ./objects/Cve.cpp
    ./objects/DeviceInformation.cpp
    ./objects/DnsLookup.cpp
    ./objects/DnsResolver.cpp
    ./objects/Interface.cpp
    ./objects/InterfaceNetwork.cpp
    ./objects/IpAddress.cpp
    ./objects/IpNetwork.cpp
    ./objects/MacAddress.cpp
    ./objects/OperatingSystem.cpp
    ./objects/Port.cpp
    ./objects/PortRange.cpp
    ./objects/Route.cpp
    ./objects/Service.cpp
    ./objects/ToolObservations.cpp
    ./objects/TracerouteHop.cpp
    ./objects/Vlan.cpp
    ./objects/Vrf.cpp

    ./objects/aws/Attachment.cpp
    ./objects/aws/CidrBlock.cpp
    ./objects/aws/Instance.cpp
    ./objects/aws/NetworkAcl.cpp
    ./objects/aws/NetworkInterface.cpp
    ./objects/aws/Route.cpp
    ./objects/aws/RouteTable.cpp
    ./objects/aws/SecurityGroup.cpp
    ./objects/aws/Subnet.cpp
    ./objects/aws/Vpc.cpp

    ./tools/AbstractDatastoreTool.cpp
    ./tools/AbstractExportTool.cpp
    ./tools/AbstractGraphTool.cpp
    ./tools/AbstractInsertTool.cpp

    ./utils/QueriesCommon.cpp
    ./utils/ServiceFactory.cpp
  )
target_include_directories(${TGT_LIBRARY}
  PUBLIC
    ${Boost_INCLUDE_DIRS}
    include
  )
target_link_libraries(${TGT_LIBRARY}
  PUBLIC
    netmeld-core
    pqxx
  )

nm_install_lib(${TGT_LIBRARY})

foreach(ITEM
      objects
      parsers
      tools
      utils
      schemas
    )
  nm_install_include(${ITEM} "datastore")
endforeach()
