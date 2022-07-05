// =============================================================================
// Copyright 2022 National Technology & Engineering Solutions of Sandia, LLC
// (NTESS). Under the terms of Contract DE-NA0003525 with NTESS, the U.S.
// Government retains certain rights in this software.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// =============================================================================
// Maintained by Sandia National Laboratories <Netmeld@sandia.gov>
// =============================================================================

#ifndef PARSER_HPP
#define PARSER_HPP

#include <nlohmann/json.hpp>
#include <netmeld/datastore/objects/AbstractDatastoreObject.hpp>
#include <netmeld/datastore/objects/DeviceInformation.hpp>
#include <netmeld/datastore/objects/Interface.hpp>
#include <netmeld/datastore/objects/MacAddress.hpp>
#include <netmeld/datastore/objects/ToolObservations.hpp>

namespace nmdo = netmeld::datastore::objects;

using json = nlohmann::json;


// =============================================================================
// Data containers
// =============================================================================
struct Data {
  std::vector<nmdo::DeviceInformation> devices;
  std::map<std::string, std::vector<nmdo::Interface>> interfaces;

  nmdo::ToolObservations observations;
};
typedef std::vector<Data>    Result;


// =============================================================================
// Parser definition
// =============================================================================
class Parser
{
  // ===========================================================================
  // Variables
  // ===========================================================================
  private: // Variables are always private
    const std::string REASON {"aws ec2 describe-instances"};
    Data d;

  // ===========================================================================
  // Constructors
  // ===========================================================================
  public: // Constructor is only default and must be public
    Parser();

  // ===========================================================================
  // Methods
  // ===========================================================================
  private:
  protected:
    void processInstances(const json&);
    void processInterfaces(const json&, const std::string&);
    void processSecurityGroups(const json&);
    void processIps(const json&, nmdo::Interface&);

  public:
    void fromJson(const json&);
    Result getData();
};
#endif // PARSER_HPP
