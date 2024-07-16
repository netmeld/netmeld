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

#ifndef PORTRANGE_HPP
#define PORTRANGE_HPP

#include <cstdint>
#include <string>
#include <tuple>

#include <netmeld/core/objects/AbstractObject.hpp>

namespace nmco  = netmeld::core::objects;

namespace netmeld::datastore::objects {

  class Iterator {
    private:
      uint16_t value;

    public:
      // traits
      using iterator_category = std::forward_iterator_tag;
      using value_type        = uint16_t;
      using difference_type   = std::ptrdiff_t;
      using pointer           = const uint16_t*;
      using reference         = const uint16_t&;

      explicit Iterator(uint16_t value) : value(value) {}

      reference operator*() const { return value; }
      pointer operator->() { return &value; }

      Iterator& operator++() {
        ++value;
        return *this;
      }
      Iterator& operator++(int) {
        Iterator tmp = *this;
        ++(*this);
        return tmp;
      }
      std::strong_ordering operator<=>(const Iterator& rhs) const {
        return value <=> rhs.value;
      }
      bool operator==(const Iterator& rhs) const {
        return 0 == operator<=>(rhs);
      }
  };

  class PortRange : public nmco::AbstractObject
  {
    // =========================================================================
    // Variables
    // =========================================================================
    private:
    protected:
      uint16_t min;
      uint16_t max;

    public:
      const uint16_t& first;
      const uint16_t& last;

      Iterator begin() const {return Iterator(min); }
      Iterator end()   const {return Iterator(max); }

    // =========================================================================
    // Constructors
    // =========================================================================
    private:
    protected:
    public:
      explicit PortRange();
      explicit PortRange(uint16_t);
      explicit PortRange(uint16_t, uint16_t);
      explicit PortRange(const std::string&);

    // =========================================================================
    // Methods
    // =========================================================================
    private:
      std::string translateFromTypicalServiceAlias(const std::string&) const;

    protected:
    public:
      std::string toString() const;
      std::string toDebugString() const override;
      std::string toHumanString() const;

      std::strong_ordering operator<=>(const PortRange&) const;
      bool operator==(const PortRange&) const;

      PortRange& operator=(const PortRange&);
  };
}
#endif // PORTRANGE_HPP
