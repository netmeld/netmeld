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


#ifndef CVE_HPP
#define CVE_HPP


#include <cstdint>
#include <iostream>
#include <string>

#include <pqxx/pqxx>

#include <boost/format.hpp>


class CVE
{
public:
  virtual ~CVE() = default;
  explicit CVE(uint16_t year, uint32_t number);
  explicit CVE(std::string const& cve_id);

  uint16_t year() const;
  uint32_t number() const;

  std::string str() const;

private:
  uint16_t year_;
  uint32_t number_;
};


std::ostream&
operator<<(std::ostream& os, CVE const& cve);


inline
uint16_t
CVE::
year() const
{
  return year_;
}


inline
uint32_t
CVE::
number() const
{
  return number_;
}


#endif  /* CVE_HPP */
