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

#ifndef AC_BOOK_HPP
#define AC_BOOK_HPP

#include <set>

#include <netmeld/core/objects/AbstractObject.hpp>


namespace netmeld::core::objects {

  template<typename TData>
  class AcBook : public AbstractObject {
    // =========================================================================
    // Variables
    // =========================================================================
    private: // Variables will probably rarely appear at this scope
    protected: // Variables intended for internal/subclass API
      std::string      id;
      std::string      name;
      std::set<TData>  data;

    public: // Variables should rarely appear at this scope

    // =========================================================================
    // Constructors
    // =========================================================================
    private: // Constructors which should be hidden from API users
    protected: // Constructors part of subclass API
    public: // Constructors part of public API
      AcBook();

    // =========================================================================
    // Methods
    // =========================================================================
    private: // Methods which should be hidden from API users
    protected: // Methods part of subclass API
    public: // Methods part of public API
      virtual void setId(const std::string&);
      virtual void setName(const std::string&);
      virtual void addData(const TData&);
      virtual void addData(const std::set<TData>&);
      virtual void removeData(const TData&);
      virtual std::set<TData> getData() const;

      // Inherited from AbstractObject at this scope
        // virtual void saveAsMetadata(pqxx::transaction_base&, Uuid);
        // friend std::ostream& operator<<(std::ostream&, const AbstractObject&);

      // Always overriden from AbstractObject
      virtual bool isValid() const override;
      virtual void save(pqxx::transaction_base&, const Uuid&, const std::string&) override;
      std::string toDebugString() const override;
  };
#include "AcBook.ipp"
}
#endif // AC_BOOK_HPP
