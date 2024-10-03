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

#ifndef SERVICE_HPP
#define SERVICE_HPP

#include <compare>
#include <set>

#include <netmeld/datastore/objects/AbstractDatastoreObject.hpp>
#include <netmeld/datastore/objects/IpAddress.hpp>


namespace netmeld::datastore::objects {

  class Service : public AbstractDatastoreObject {
    // =========================================================================
    // Variables
    // =========================================================================
    private:
    protected:
      std::string            dstFqdn;    // FQDN this is on
      IpAddress              dstAddress; // IP this is on
      IpAddress              srcAddress; // IP this seen from
      bool                   isLocal {false};
      std::string            interfaceName {"-"};
      std::string            zone;
      std::string            serviceName;
      std::string            serviceDescription;
      std::string            serviceReason;
      std::string            protocol;
      std::set<std::string>  dstPorts; // ports this listens on
      std::set<std::string>  srcPorts; // ports this accepts data from

    public:


    // =========================================================================
    // Constructors
    // =========================================================================
    private:
    protected:
    public:
      Service();
      Service(const std::string&, const IpAddress&);


    // =========================================================================
    // Methods
    // =========================================================================
    private:
    protected:
      bool isValidDevice() const;
      bool isValidNetwork() const;

      void saveAsDevice(pqxx::transaction_base&,
                        const nmco::Uuid&, const std::string&);
      void saveAsNetwork(pqxx::transaction_base&, const nmco::Uuid&);

    public:
      void addDstPort(const std::string&);
      void addSrcPort(const std::string&);

      std::string getServiceName() const;

      void setDstAddress(const IpAddress&);
      void setSrcAddress(const IpAddress&);
      void setInterfaceName(const std::string&);
      void setServiceName(const std::string&);
      void setServiceDescription(const std::string&);
      void setServiceReason(const std::string&);
      void setProtocol(const std::string&);

      bool isValid() const override;

      void save(pqxx::transaction_base&,
                const nmco::Uuid&, const std::string&) override;

      std::string toDebugString() const override;

      std::partial_ordering operator<=>(const Service&) const;
      bool operator==(const Service&) const;
  };
}
#endif // SERVICE_HPP
