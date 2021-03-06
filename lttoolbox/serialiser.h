// Copyright (C) 2005 Universitat d'Alacant / Universidad de Alicante
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 2 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.

#ifndef SERIALISER_H
#define SERIALISER_H

#include "exception.h"

#include <lttoolbox/stdint_shim.h>
#include <cstddef>
#include <limits>
#include <ios>
#include <limits>
#include <map>
#include <ostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace {
template <typename SerialisedType>
static unsigned char compressedSize(const SerialisedType &SerialisedType_) {
  unsigned char compressedSize_ = 0;

  // Have to be careful here, if we shift >> 64 it's the same as >> 0 so make
  // sure only to shift up to >> 56
  for (; (SerialisedType_ >>
          std::numeric_limits<unsigned char>::digits * compressedSize_ > 255);
       ++compressedSize_) {
  }
  ++compressedSize_;

  return compressedSize_;
}

template <typename SerialisedType> class Serialiser;

template <typename value_type>
class Serialiser<std::basic_string<value_type> > {
public:
  inline static void
  serialise(const std::basic_string<value_type> &SerialisedType_,
            std::ostream &Output);
};

template <typename first_type, typename second_type>
class Serialiser<std::pair<first_type, second_type> > {
public:
  inline static void
  serialise(const std::pair<first_type, second_type> &SerialisedType_,
            std::ostream &Output);
};

template <> class Serialiser<signed int> {
public:
  inline static void serialise(const signed int &SerialisedType_,
                               std::ostream &Output);
};

template <> class Serialiser<size_t> {
public:
  inline static void serialise(const size_t &SerialisedType_,
                               std::ostream &Output);
};

template <> class Serialiser<wchar_t> {
public:
  inline static void serialise(const wchar_t &SerialisedType_,
                               std::ostream &Output);
};

template <> class Serialiser<char> {
public:
  inline static void serialise(const char &SerialisedType_,
                               std::ostream &Output);
};

template<> class Serialiser<double> {
public:
  inline static void serialise(const double &SerialisedType_,
                               std::ostream &Output);
};

template <typename Container>
class Serialiser {
public:
  inline static void serialise(const Container &SerialisedType_,
                               std::ostream &Output);
};
}

template <typename SerialisedType>
inline void serialise(const SerialisedType &SerialisedType_,
                      std::ostream &Output) {
  Serialiser<SerialisedType>::serialise(SerialisedType_, Output);
}

template <typename value_type>
void Serialiser<std::basic_string<value_type> >::serialise(
    const std::basic_string<value_type> &SerialisedType_,
    std::ostream &Output) {
  ::serialise(SerialisedType_.size(), Output);

  for (typename std::basic_string<value_type>::const_iterator
           SerialisedType_iterator = SerialisedType_.begin();
       // Call .end() each iteration to save memory.
       SerialisedType_iterator != SerialisedType_.end();
       ++SerialisedType_iterator) {
    ::serialise(*SerialisedType_iterator, Output);
  }
}

template <typename first_type, typename second_type>
void Serialiser<std::pair<first_type, second_type> >::serialise(
    const std::pair<first_type, second_type> &SerialisedType_,
    std::ostream &Output) {
  ::serialise(SerialisedType_.first, Output);
  ::serialise(SerialisedType_.second, Output);
}

template <typename integer_type>
void int_serialise(const integer_type &SerialisedType_,
                   std::ostream &Output) {
  try {
    Output.put(compressedSize(SerialisedType_));

    if (!Output) {
      std::stringstream what_;
      what_ << "can't serialise size " << std::hex
            << /* [1] */ +compressedSize(SerialisedType_) << std::dec;
      throw SerialisationException(what_.str().c_str());
    }

    for (unsigned char CompressedSize = compressedSize(SerialisedType_);
         CompressedSize != 0; Output.put(static_cast<unsigned char>(
             SerialisedType_ >>
             std::numeric_limits<unsigned char>::digits * --CompressedSize))) {
      if (!Output) {
        std::stringstream what_;
        what_ << "can't serialise byte " << std::hex
              << /* [1] */ +static_cast<unsigned char>(
                     SerialisedType_ >>
                     std::numeric_limits<unsigned char>::digits *
                         CompressedSize) << std::dec;
        throw SerialisationException(what_.str().c_str());
      }
    }
  } catch (const SerialisationException &exc) {
    std::stringstream what_;
    what_ << "can't serialise const " << sizeof(integer_type) << " byte integer type: "
          << exc.what();
    throw SerialisationException(what_.str().c_str());
  }
}

void Serialiser<signed int>::serialise(const signed int &SerialisedType_,
                                   std::ostream &Output) {
  // Assumes size_t >= int and signed int != size_t
  Serialiser<size_t>::serialise(SerialisedType_, Output);
}

void Serialiser<size_t>::serialise(const size_t &SerialisedType_,
                                   std::ostream &Output) {
  // Assumes size_t == uint64_t, ie it is fixed across platforms
  int_serialise(SerialisedType_, Output);
}

void Serialiser<wchar_t>::serialise(const wchar_t &SerialisedType_,
                                    std::ostream &Output) {
  int_serialise(SerialisedType_, Output);
}

void Serialiser<char>::serialise(const char &SerialisedType_,
                                 std::ostream &Output) {
  int_serialise(SerialisedType_, Output);
}

void Serialiser<double>::serialise(const double &SerialisedType_,
                                   std::ostream &Output) {
  union {
    uint64_t i;
    double d;
  } u;
  u.d = SerialisedType_;
  Serialiser<uint64_t>::serialise(u.i, Output);
}

template <typename Container>
void Serialiser<Container>::serialise(
    const Container &SerialisedType_, std::ostream &Output) {
  ::serialise(SerialisedType_.size(), Output);

  for (typename Container::const_iterator value_type_ =
           SerialisedType_.begin();
       // Call .end() each iteration to save memory.
       value_type_ != SerialisedType_.end(); ++value_type_) {
    ::serialise(*value_type_, Output);
  }
}

// [1] operator+ promotes its operand to a printable integral type.

#endif // SERIALISER_H
