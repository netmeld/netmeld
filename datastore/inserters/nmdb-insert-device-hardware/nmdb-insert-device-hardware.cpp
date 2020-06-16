// =============================================================================
// Copyright 2017 National Technology & Engineering Solutions of Sandia, LLC
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

#include <netmeld/datastore/objects/DeviceInformation.hpp>
#include <netmeld/datastore/tools/AbstractInsertTool.hpp>

namespace nmdt = netmeld::datastore::tools;
namespace nmdo = netmeld::datastore::objects;


class Tool : public nmdt::AbstractInsertTool
{
  public:
    Tool() : nmdt::AbstractInsertTool
      ("device hardware", PROGRAM_NAME, PROGRAM_VERSION)
    {}

    void
    addToolOptions() override
    {
      addRequiredDeviceId();

      opts.addOptionalOption("vendor", std::make_tuple(
            "vendor",
            po::value<std::string>(),
            "Name of device hardware vendor.")
          );

      opts.addOptionalOption("model", std::make_tuple(
            "model",
            po::value<std::string>(),
            "Model of device hardware.")
          );

      opts.addOptionalOption("hardware-revision", std::make_tuple(
            "hardware-revision",
            po::value<std::string>(),
            "Hardware revision of device hardware.")
          );

      opts.addOptionalOption("serial-number", std::make_tuple(
            "serial-number",
            po::value<std::string>(),
            "Serial number of device hardware.")
          );

      opts.addOptionalOption("description", std::make_tuple(
            "description",
            po::value<std::string>(),
            "Description of the device hardware.")
          );
    }

    void
    specificInserts(pqxx::transaction_base& t) override
    {
      const auto& toolRunId {getToolRunId()};
      //auto& devInfo         {this->devInfo};

      // NOTE: RequiredDeviceId data is currently handled in parent tool

      if (opts.exists("vendor")) {
        devInfo.setVendor(opts.getValue("vendor"));
      }

      if (opts.exists("model")) {
        devInfo.setModel(opts.getValue("model"));
      }

      if (opts.exists("hardware-revision")) {
        devInfo.setHardwareRevision(opts.getValue("hardware-revision"));
      }

      if (opts.exists("serial-number")) {
        devInfo.setSerialNumber(opts.getValue("serial-number"));
      }

      if (opts.exists("description")) {
        devInfo.setDescription(opts.getValue("description"));
      }

      devInfo.save(t, toolRunId);
      LOG_DEBUG << devInfo.toDebugString() << std::endl;
    }
};


int
main(int argc, char** argv)
{
  Tool tool;
  return tool.start(argc, argv);
}
