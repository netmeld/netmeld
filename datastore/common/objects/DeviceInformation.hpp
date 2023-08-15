// =============================================================================
// Copyright 2023 National Technology & Engineering Solutions of Sandia, LLC
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

#ifndef DEVICE_INFORMATION_HPP
#define DEVICE_INFORMATION_HPP

#include <netmeld/datastore/objects/AbstractDatastoreObject.hpp>


namespace netmeld::datastore::objects {

  class DeviceInformation : public AbstractDatastoreObject {
    // =========================================================================
    // Variables
    // =========================================================================
    private: // Variables will probably rarely appear at this scope
    protected: // Variables intended for internal/subclass API
      std::string deviceId;
      std::string deviceType;
      std::string deviceColor;
      std::string vendor;
      std::string model;
      std::string hardwareRevision;
      std::string serialNumber;
      std::string description;

    public: // Variables should rarely appear at this scope

    // =========================================================================
    // Constructors
    // =========================================================================
    private: // Constructors which should be hidden from API users
    protected: // Constructors part of subclass API
    public: // Constructors part of public API
      DeviceInformation();
      explicit DeviceInformation(const std::string&);

    // =========================================================================
    // Methods
    // =========================================================================
    private: // Methods which should be hidden from API users
    protected: // Methods part of subclass API
    public: // Methods part of public API
      void setDeviceId(const std::string&);
      void setDeviceColor(const std::string&);
      void setDeviceType(const std::string&);
      void setDescription(const std::string&);
      void setHardwareRevision(const std::string&);
      void setModel(const std::string&);
      void setSerialNumber(const std::string&);
      void setVendor(const std::string&);

      std::string getDeviceId() const;
      std::string getDeviceType() const;

      bool isValid() const override;

      void save(pqxx::transaction_base&,
                const nmco::Uuid&, const std::string& x="") override;

      std::string toDebugString() const override;

      std::strong_ordering operator<=>(const DeviceInformation&) const;
      bool operator==(const DeviceInformation&) const;
  };
}
#endif // DEVICE_INFORMATION_HPP
