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

#ifndef ABSTRACT_OBJECT_HPP
#define ABSTRACT_OBJECT_HPP

#include <netmeld/core/objects/Uuid.hpp>
#include <netmeld/core/utils/LoggerSingleton.hpp>
#include <netmeld/core/utils/QueriesCommon.hpp>

namespace nmcu = netmeld::core::utils;

namespace netmeld::core::objects {

  class AbstractObject {
    // =========================================================================
    // Variables
    // =========================================================================
    private:
    protected:
    public:

    // =========================================================================
    // Constructors
    // =========================================================================
    private:
    protected:
    public:
      AbstractObject();

    // =========================================================================
    // Methods
    // =========================================================================
    private:
    protected:
    public:
      virtual bool isValid() const;

      virtual void save(pqxx::transaction_base&,
                        const Uuid&, const std::string&);
      virtual void saveAsMetadata(pqxx::transaction_base&, const Uuid&);

      virtual std::string toDebugString() const;

      friend std::ostream& operator<<(std::ostream&, const AbstractObject&);
  };
}

#endif // ABSTRACT_OBJECT_HPP
