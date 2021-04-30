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

#ifndef INTERFACE_HPP
#define INTERFACE_HPP

#include <netmeld/datastore/objects/AbstractDatastoreObject.hpp>
#include <netmeld/datastore/objects/IpAddress.hpp>
#include <netmeld/datastore/objects/MacAddress.hpp>


namespace netmeld::datastore::objects {

  class Interface : public AbstractDatastoreObject {
    // =========================================================================
    // Variables
    // =========================================================================
    private:
    protected:
      std::string  name;
      std::string  mediaType {"ethernet"};
      bool         isUp      {false};
      MacAddress   macAddr;

      // Linux specific
      std::string  flags;
      uint32_t     mtu {0}; // never used

    public:


    // =========================================================================
    // Constructors
    // =========================================================================
    private:
    protected:
    public:
      Interface();
      explicit Interface(const std::string&);


    // =========================================================================
    // Methods
    // =========================================================================
    private:
    protected:
    public:
      void addIpAddress(IpAddress&);

      void setName(const std::string&);
      void setMediaType(const std::string&);
      void setMacAddress(const MacAddress&);
      void setUp();
      void setDown();

      std::string getName() const;
      MacAddress getMacAddress() const;
      std::vector<IpAddress> getIpAddresses() const;

      bool isValid() const override;

      void save(pqxx::transaction_base&,
                const nmco::Uuid&, const std::string&) override;
      void saveAsMetadata(pqxx::transaction_base&,
                          const nmco::Uuid&) override;

      std::string toDebugString() const override;

      // Linux specific
      void setFlags(const std::string&);
      void setMtu(uint32_t);
  };
}
#endif // INTERFACE_HPP
