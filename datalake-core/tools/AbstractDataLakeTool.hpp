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

#ifndef ABSTRACT_DATA_LAKE_TOOL_HPP
#define ABSTRACT_DATA_LAKE_TOOL_HPP


#include <netmeld/core/tools/AbstractTool.hpp>

#include <netmeld/datalake/core/objects/DataLake.hpp>

namespace nmct = netmeld::core::tools;
namespace nmdlco = netmeld::datalake::core::objects;


namespace netmeld::datalake::core::tools {

  class AbstractDataLakeTool : public nmct::AbstractTool
  {
    // =========================================================================
    // Variables
    // =========================================================================
    private: // Variables should generally be private
    protected: // Variables intended for internal/subclass API
      // Inhertied from nmct::AbstractTool at this scope
        // std::string            helpBlurb;
        // std::string            programName;
        // std::string            version;
        // ProgramOptions         opts;
    public: // Variables should rarely appear at this scope


    // =========================================================================
    // Constructors
    // =========================================================================
    private: // Constructors should rarely appear at this scope
    protected: // Constructors intended for internal/subclass API
    public: // Constructors should generally be public
    private:
    protected:
      // Default constructor, provided only for convienence
      AbstractDataLakeTool();
      // Standard constructor, should be primary
      AbstractDataLakeTool(const char*, const char*, const char*);

    public:

    // =========================================================================
    // Methods
    // =========================================================================
    private: // Methods part of internal API
    protected: // Methods part of subclass API
      // Inherited from nmct::AbstractTool at this scope
        // std::string const getDbName() const;
        // virtual void printHelp() const;
        // virtual void printVersion() const;
        // virtual int  runTool();
    public: // Methods part of public API
      // Inherited from nmct::AbstractTool, don't override as primary tool entry point
      // int start(int, char**) noexcept;
    private:
      void addToolBaseOptions() override;

    protected:
      virtual void modifyToolOptions() override;
      virtual void printHelp() const override;
      // Tool specific behavior entry point
      virtual int  runTool() override;

      std::unique_ptr<nmdlco::DataLake> getDataLakeHandler();

    public:
  };
}
#endif // ABSTRACT_DATA_LAKE_TOOL_HPP
