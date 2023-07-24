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

// NOTE This implementation is included in the header (at the end) since it
//      leverages templating.

#include <netmeld/datastore/parsers/ParserHelper.hpp>
#include <netmeld/datastore/utils/QueriesCommon.hpp>

namespace nmcu = netmeld::core::utils;
namespace nmdp = netmeld::datastore::parsers;
namespace nmdu = netmeld::datastore::utils;

namespace netmeld::datastore::tools {
  // ===========================================================================
  // Constructors
  // ===========================================================================
  template<typename P, typename R>
  AbstractImportSpiritTool<P,R>::AbstractImportSpiritTool()
  {}

  template<typename P, typename R>
  AbstractImportSpiritTool<P,R>::AbstractImportSpiritTool(
      const char* _helpBlurb,
      const char* _programName,
      const char* _version) :
    AbstractImportTool<P,R>(_helpBlurb, _programName, _version)
  {}


  // ===========================================================================
  // Tool Entry Points (execution order)
  // ===========================================================================

  template<typename P, typename R>
  void
  AbstractImportSpiritTool<P,R>::parseData() // Could pass the parser as an argument
  {
    this->executionStart = nmco::Time();
    this->tResults = nmdp::fromFilePath<P,R>(this->dataPath.string());
    this->executionStop = nmco::Time();
  }
}
