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
// Maintained by Sandia National Laboratories <Netmeld@sandia.gov>


#ifndef NETWORKING_IPP
#define NETWORKING_IPP


// ----------------------------------------------------------------------

template<typename T>
T
mask_from_cidr(uint16_t const cidr)
{
  typename T::bytes_type bytes;

  for (size_t i = 0U; i < bytes.size(); ++i) {
    bytes.at(i) = ((i < (cidr / 8U)) ? 0xFF : 0x00);
  }

  if ((cidr % 8U) && ((cidr / 8U) < bytes.size())) {
    bytes.at(cidr / 8U) = static_cast<uint8_t>(0xFF << (8U - (cidr % 8U)));
  }

  return T{bytes};
}


// ----------------------------------------------------------------------

template<typename T>
inline
T
IPvX_Addr_with_Prefix<T>::
addr() const
{
  return addr_;
}


template<typename T>
inline
uint16_t
IPvX_Addr_with_Prefix<T>::
cidr() const
{
  return cidr_;
}


template<typename T>
inline
T
IPvX_Addr_with_Prefix<T>::
mask() const
{
  return mask_from_cidr<T>(cidr_);
}


template<typename T>
inline
void
IPvX_Addr_with_Prefix<T>::
set_addr(T const& a)
{
  addr_ = a;
}


template<typename T>
inline
void
IPvX_Addr_with_Prefix<T>::
set_cidr(uint16_t const c)
{
  cidr_ = c;
}


template<typename T>
inline
void
IPvX_Addr_with_Prefix<T>::
set_mask(T const& m)
{
  set_cidr(cidr_from_mask<T>(m));
}


template<typename T>
inline
bool
IPvX_Addr_with_Prefix<T>::
operator==(IPvX_Addr_with_Prefix<T> const& other) const
{
  return ((addr_ == other.addr_) &&
          (cidr_ == other.cidr_));
}


template<typename T>
inline
bool
IPvX_Addr_with_Prefix<T>::
operator<(IPvX_Addr_with_Prefix<T> const& other) const
{
  return ((addr_ == other.addr_) ?
          (cidr_ < other.cidr_) :
          (addr_ < other.addr_));
}


// ----------------------------------------------------------------------

inline
IP_Addr
IP_Addr_with_Prefix::
addr() const
{
  return addr_;
}


inline
uint16_t
IP_Addr_with_Prefix::
cidr() const
{
  return cidr_;
}


inline
void
IP_Addr_with_Prefix::
set_addr(IP_Addr const& a)
{
  addr_ = a;
}


inline
void
IP_Addr_with_Prefix::
set_cidr(uint16_t const c)
{
  cidr_ = c;
}


inline
bool
IP_Addr_with_Prefix::
operator==(IP_Addr_with_Prefix const& other) const
{
  return ((addr_ == other.addr_) &&
          (cidr_ == other.cidr_));
}


inline
bool
IP_Addr_with_Prefix::
operator<(IP_Addr_with_Prefix const& other) const
{
  return ((addr_ == other.addr_) ?
          (cidr_ < other.cidr_) :
          (addr_ < other.addr_));
}


// ----------------------------------------------------------------------


#endif  /* NETWORKING_IPP */
