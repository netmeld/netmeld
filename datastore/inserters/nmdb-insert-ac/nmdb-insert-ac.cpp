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

#include <netmeld/datastore/tools/AbstractInsertTool.hpp>
#include <netmeld/datastore/objects/AcRule.hpp>
#include <netmeld/datastore/objects/AcNetworkBook.hpp>
#include <netmeld/datastore/objects/AcServiceBook.hpp>

namespace nmco = netmeld::core::objects;
namespace nmdo = netmeld::datastore::objects;
namespace nmdt = netmeld::datastore::tools;


// =============================================================================
// Insert tool definition
// =============================================================================
class Tool : public nmdt::AbstractInsertTool
{
  // ===========================================================================
  // Variables
  // ===========================================================================
  private: // Variables should generally be private
  protected: // Variables intended for internal/subclass API
  public: // Variables should rarely appear at this scope


  // ===========================================================================
  // Constructors
  // ===========================================================================
  private: // Constructors should rarely appear at this scope
  protected: // Constructors intended for internal/subclass API
  public: // Constructors should generally be public
    Tool() : nmdt::AbstractInsertTool
      (
       "access control rule or data book",    // help message, prefixed with:
                        //   "Insert a manually specified "
       PROGRAM_NAME,    // program name (set in CMakeLists.txt)
       PROGRAM_VERSION  // program version (set in CMakeLists.txt)
      )
    {}


  // ===========================================================================
  // Methods
  // ===========================================================================
  private: // Methods part of internal API
    void
    modifyToolOptions() override
    {
      opts.addRequiredOption("device-id", std::make_tuple(
            "device-id",
            po::value<std::string>()->required(),
            "Name of device.")
          );

      const std::string RULE {"Rule"};
      opts.createCustomOptionsMap(RULE);
      opts.addOption(RULE, "rule-id", std::make_tuple(
            "id",
            po::value<size_t>()->default_value(0U),
            "Identificaiton number")
          );
      opts.addOption(RULE, "rule-src-id", std::make_tuple(
            "src-id",
            po::value<std::string>()->default_value("global"),
            "Source identifier")
          );
      opts.addOption(RULE, "rule-src", std::make_tuple(
            "src",
            po::value<std::string>(),
            "Source network name")
          );
      opts.addOption(RULE, "rule-src-iface", std::make_tuple(
            "src-iface",
            po::value<std::string>()->default_value("any"),
            "Source interface")
          );
      opts.addOption(RULE, "rule-dst-id", std::make_tuple(
            "dst-id",
            po::value<std::string>()->default_value("global"),
            "Destination identifier")
          );
      opts.addOption(RULE, "rule-dst", std::make_tuple(
            "dst",
            po::value<std::string>(),
            "Destination network name")
          );
      opts.addOption(RULE, "rule-dst-iface", std::make_tuple(
            "dst-iface",
            po::value<std::string>()->default_value("any"),
            "Destination interface")
          );
      opts.addOption(RULE, "rule-action", std::make_tuple(
            "action",
            po::value<std::string>(),
            "Action")
          );
      opts.addOption(RULE, "rule-service", std::make_tuple(
            "service",
            po::value<std::string>()->default_value("any"),
            "Service")
          );
      opts.addOption(RULE, "rule-description", std::make_tuple(
            "description",
            po::value<std::string>()->default_value(""),
            "Rule description/name/alias")
          );

      const std::string NBOOK {"Network Book"};
      opts.createCustomOptionsMap(NBOOK);
      opts.addOption(NBOOK, "nb-set-id", std::make_tuple(
            "nb-id",
            po::value<std::string>()->default_value("global"),
            "Source/Destination identifier")
          );
      opts.addOption(NBOOK, "nb-set-name", std::make_tuple(
            "nb-name",
            po::value<std::string>(),
            "Source/Destination network name")
          );
      opts.addOption(NBOOK, "nb-set-data", std::make_tuple(
            "nb-data",
            po::value<std::string>(),
            "Data contained in the set (e.g., IP)")
          );


      const std::string SBOOK {"Service Book"};
      opts.createCustomOptionsMap(SBOOK);
      opts.addOption(SBOOK, "sb-set-name", std::make_tuple(
            "sb-name",
            po::value<std::string>(),
            "Service name")
          );
      opts.addOption(SBOOK, "sb-set-data", std::make_tuple(
            "sb-data",
            po::value<std::string>(),
            "Data contained in the set (e.g., protocol:src-port:dst-port)")
          );
    }

    void
    specificInserts(pqxx::transaction_base& t) override
    {
      const auto& toolRunId {getToolRunId()};
      const auto& deviceId  {getDeviceId()};

      if (!trySaveAcRule(t, toolRunId, deviceId) &&
          !trySaveAcNetworkBook(t, toolRunId, deviceId) &&
          !trySaveAcServiceBook(t, toolRunId, deviceId)) {
        LOG_INFO << "Failed to save any access control data" << std::endl;
      }
    }

    bool
    trySaveAcRule(pqxx::transaction_base& t, const nmco::Uuid& toolRunId,
                  const std::string& deviceId) const
    {
      bool status {false};

      if (opts.exists("src") && opts.exists("dst")) {
        nmdo::AcRule acr;
        acr.setRuleId(opts.getValueAs<size_t>("id"));
        acr.setSrcId(opts.getValue("src-id"));
        acr.addSrc(opts.getValue("src"));
        acr.addSrcIface(opts.getValue("src-iface"));
        acr.setDstId(opts.getValue("dst-id"));
        acr.addDst(opts.getValue("dst"));
        acr.addDstIface(opts.getValue("dst-iface"));
        acr.addService(opts.getValue("service"));
        acr.addAction(opts.getValue("action"));
        acr.setRuleDescription(opts.getValue("description"));

        LOG_DEBUG << acr.toDebugString() << std::endl;
        acr.save(t, toolRunId, deviceId);
        status = acr.isValid();

        if (status) {
          LOG_INFO << "Saved AcRule" << std::endl;
        }
      }

      return status;
    }

    bool
    trySaveAcNetworkBook(pqxx::transaction_base& t, const nmco::Uuid& toolRunId,
                         const std::string& deviceId) const
    {
      bool status {false};

      if (opts.exists("nb-name") && opts.exists("nb-data")) {
        nmdo::AcNetworkBook acnb;

        acnb.setId(opts.getValue("nb-id"));
        acnb.setName(opts.getValue("nb-name"));
        acnb.addData(opts.getValue("nb-data"));

        LOG_DEBUG << acnb.toDebugString() << std::endl;
        acnb.save(t, toolRunId, deviceId);
        status = acnb.isValid();

        if (status) {
          LOG_INFO << "Saved AcNetworkBook" << std::endl;
        }
      }

      return status;
    }

    bool
    trySaveAcServiceBook(pqxx::transaction_base& t, const nmco::Uuid& toolRunId,
                         const std::string& deviceId) const
    {
      bool status {false};

      if (opts.exists("sb-name") && opts.exists("sb-data")) {
        nmdo::AcServiceBook acsb;

        acsb.setName(opts.getValue("sb-name"));
        acsb.addData(opts.getValue("sb-data"));

        LOG_DEBUG << acsb.toDebugString() << std::endl;
        acsb.save(t, toolRunId, deviceId);
        status = acsb.isValid();

        if (status) {
          LOG_INFO << "Saved AcServiceBook" << std::endl;
        }
      }

      return status;
    }

  protected: // Methods part of subclass API
  public: // Methods part of public API
};


int
main(int argc, char** argv)
{
  Tool tool;
  return tool.start(argc, argv);
}
