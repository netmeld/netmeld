// =============================================================================
// Copyright 2024 National Technology & Engineering Solutions of Sandia, LLC
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

/* Notes:
   - This unit is part of the complilation process to help ensure consistency
     between templates and the actual data
   - Various data is included and most is commented solely for educational
     purposes
     - In non-template, remove data as makes sense

   Guidelines:
   - Base classes contain some implementation (even if NOOP) for every method
     - Method overriding is intentional to alter behaviour, not scope hiding
     - Final has not been used to facilitate new concepts
       - This may change as code base matures
   - Import tools are never "base" classes
   - Data order
     - 1st tier: Variables, Constructors, Methods
     - 2nd tier: private, protected, public
   - Section headers should generally be left to help code organization
   - Parser logic should be separate
*/

#include <netmeld/datastore/tools/AbstractImportJsonTool.hpp>

#include "Parser.hpp"

namespace nmdo = netmeld::datastore::objects;
namespace nmdt = netmeld::datastore::tools;


// =============================================================================
// Import tool definition
// =============================================================================
template<typename P, typename R>
class Tool : public nmdt::AbstractImportJsonTool<P,R>
{
  // ===========================================================================
  // Variables
  // ===========================================================================
  private: // Variables should generally be private
  protected: // Variables intended for internal/subclass API
    // Inhertied from AbstractTool at this scope
      // std::string            helpBlurb;
      // std::string            programName;
      // std::string            version;
      // ProgramOptions         opts;
    // Inhertied from AbstractImportJsonTool at this scope
      // TResults                 tResults;
      // nmco::Uuid               toolRunId;
      // nmco::Time               executionStart;
      // nmco::Time               executionStop;
      // nmco::DeviceInformation  devInfo;
  public: // Variables should rarely appear at this scope


  // ===========================================================================
  // Constructors
  // ===========================================================================
  private: // Constructors should rarely appear at this scope
  protected: // Constructors intended for internal/subclass API
  public: // Constructors should generally be public
    Tool() : nmdt::AbstractImportJsonTool<P,R>
      (
       "aws iam get-account-authorization-details",  // command line tool imports data from
       PROGRAM_NAME,           // program name (set in CMakeLists.txt)
       PROGRAM_VERSION         // program version (set in CMakeLists.txt)
      )
    {}


  // ===========================================================================
  // Methods
  // ===========================================================================
  private: // Methods part of internal API
    // Overriden from AbstractImportJsonTool
    void
    addToolOptions() override
    {
      this->opts.removeRequiredOption("device-id");
      this->opts.addAdvancedOption("device-id", std::make_tuple(
            "device-id",
            po::value<std::string>(),
            "(Not used) Name of device.")
          );

      this->opts.removeOptionalOption("device-type");
      this->opts.removeOptionalOption("device-color");
    }

    // Overriden from AbstractImportJsonTool
    void
    specificInserts(pqxx::transaction_base& t) override
    {
      const auto& toolRunId {this->getToolRunId()};
      const auto& deviceId  {this->getDeviceId()};

      LOG_DEBUG << this->tResults << std::endl;
      for (auto& result : this->tResults) {
          for (auto& user : result.users) {
            // muck

            // save
            LOG_DEBUG << user.toDebugString() << std::endl;
            user.save(t, toolRunId, deviceId);

            // link
          }
          for (auto& group : result.groups) {
            // muck

            // save
            LOG_DEBUG << group.toDebugString() << std::endl;
            group.save(t, toolRunId, deviceId);

            // link
          }
          for (auto& role : result.roles) {
            // muck

            // save
            LOG_DEBUG << role.toDebugString() << std::endl;
            role.save(t, toolRunId, deviceId);

            // link
          }
          for (auto& policy : result.policies) {
            // muck

            // save
            LOG_DEBUG << policy.toDebugString() << std::endl;
            policy.save(t, toolRunId, deviceId);

            // link
          }
          for (auto& document : result.documents) {
            // muck

            // save
            LOG_DEBUG << document.toDebugString() << std::endl;
            document.save(t, toolRunId, deviceId);

            // link
          }
          for (auto& statement : result.statements) {
            // muck

            // save
            LOG_DEBUG << statement.toDebugString() << std::endl;
            statement.save(t, toolRunId, deviceId);

            // link
          }
          for (auto& policyDocument : result.policyDocuments) {
            // muck

            // save
            LOG_DEBUG << policyDocument.toDebugString() << std::endl;
            policyDocument.save(t, toolRunId, deviceId);

            // link
          }
          for (auto& policyVersion : result.policyVersions) {
            // muck

            // save
            LOG_DEBUG << policyVersion.toDebugString() << std::endl;
            policyVersion.save(t, toolRunId, deviceId);

            // link
          }
          for (auto& roleProfile : result.roleProfiles) {
            // muck

            // save
            LOG_DEBUG << roleProfile.toDebugString() << std::endl;
            roleProfile.save(t, toolRunId, deviceId);

            // link
          }
          for (auto& roleBoundary : result.roleBoundaries) {
            // muck

            // save
            LOG_DEBUG << roleBoundary.toDebugString() << std::endl;
            roleBoundary.save(t, toolRunId, deviceId);

            // link
          }
          for (auto& amp : result.amps) {
            // muck

            // save
            LOG_DEBUG << amp.toDebugString() << std::endl;
            amp.save(t, toolRunId, deviceId);

            // link
          }
          for (auto& userGroup : result.userGroups) {
            // muck

            // save
            LOG_DEBUG << userGroup.toDebugString() << std::endl;
            userGroup.save(t, toolRunId, deviceId);

            // link
          }
      }
    }

  protected: // Methods part of subclass API
    // Inherited from AbstractTool at this scope
      // std::string const getDbName() const;
      // virtual void printVersion() const;
    // Inherited from AbstractImportJsonTool at this scope
      // fs::path    const getDataPath() const;
      // std::string const getDeviceId() const;
      // nmco::Uuid   const getToolRunId() const;
      // virtual void parseData();
      // virtual void printHelp() const;
      // virtual int  runTool();
      // virtual void setToolRunId();
      // virtual void toolRunMetadataInserts(pqxx::transaction_base&) const;
  public: // Methods part of public API
    // Inherited from AbstractTool, don't override as primary tool entry point
      // int start(int, char**) noexcept;
};


// =============================================================================
// Program entry point
// =============================================================================
int main(int argc, char** argv) {
  Tool<Parser, Result> tool;
  return tool.start(argc, argv);
}
