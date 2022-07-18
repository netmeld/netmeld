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
// ======

#ifndef PACKAGE_HPP
#define PACKAGE_HPP

#include <netmeld/datastore/objects/AbstractDatastoreObject.hpp>

namespace netmeld::datastore::objects{

    class Package : public AbstractDatastoreObject
    {
    // =========================================================================
    // Variables
    // =========================================================================
    private: 
    protected:
        std::string packageStatus;
        std::string packageName;
        std::string packageVersion;
        std::string packageArch;
        std::string packageDesc;
    public:
    // =========================================================================
    // Constructors
    // =========================================================================
    private:
    protected:
    public:
      Package();
      explicit Package(const std::string&);

    // =========================================================================
    // Methods
    // =========================================================================
    private:
    protected:
    public:
    //Setters
        void setStatus(const std::string&);
        void setName(const std::string&);
        void setVersion(const std::string&);
        void setArch(const std::string&);
        void setDesc(const std::string&);

        //Getters
        std::string getStatus() const;
        std::string getName() const;
        std::string getVersion() const;
        std::string getArch() const;
        std::string getDesc() const;
        std::string toDebugString() const override;
    
    };
}
#endif