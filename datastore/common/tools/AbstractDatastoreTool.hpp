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

#ifndef ABSTRACT_DATASTORE_TOOL_HPP
#define ABSTRACT_DATASTORE_TOOL_HPP

#include <netmeld/core/tools/AbstractTool.hpp>

namespace nmct = netmeld::core::tools;


namespace netmeld::datastore::tools {

  class AbstractDatastoreTool : public nmct::AbstractTool
  {
    // =========================================================================
    // Variables
    // =========================================================================
    private:
    protected:
    public:

    // =========================================================================
    // Constructors and Destructors
    // =========================================================================
    private:
    protected:
      // Default constructor, provided only for convienence
      AbstractDatastoreTool();
      // Standard constructor, should be primary
      AbstractDatastoreTool(const char*, const char*, const char*);

    public:
      virtual ~AbstractDatastoreTool() override = default;


    // =========================================================================
    // Methods
    // =========================================================================
    private:
    protected:
      void addModuleOptions() override;

      void addRequiredDeviceId();

      const std::string getDbName() const;
      const std::string getDbArgs() const;
      const std::string getDbConnectString() const;

    public:
  };
}
#endif // ABSTRACT_DATASTORE_TOOL_HPP
