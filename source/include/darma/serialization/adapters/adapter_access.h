/*
//@HEADER
// ************************************************************************
//
//                      adapter_access.h
//                         DARMA
//              Copyright (C) 2018 Sandia Corporation
//
// Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
// the U.S. Government retains certain rights in this software.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
// 1. Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
//
// 3. Neither the name of the Corporation nor the names of the
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY SANDIA CORPORATION "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL SANDIA CORPORATION OR THE
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Questions? Contact the DARMA developers (darma-admins@sandia.gov)
//
// ************************************************************************
//@HEADER
*/

#ifndef DARMASERIALIZATION_ADAPTER_ACCESS_H
#define DARMASERIALIZATION_ADAPTER_ACCESS_H

#include <tinympl/detection.hpp>

#include <type_traits>

namespace darma {
namespace serialization {

struct ArchiveAdapterAccess {

  private:

    template <typename ImplT>
    static inline char*&
    size_reference(ImplT& ar) {
      return ar.size_reference();
    }

    template <typename ImplT>
    static inline char*&
    buffer_spot_reference(ImplT& ar) {
      return ar.buffer_spot_reference();
    }

    template <typename>
    friend struct SizingArchiveAdapter;

    template <typename>
    friend struct PackingArchiveAdapter;
};

/**
 * @todo document this
 * @tparam SizingArchiveImplementation
 *  SizingArchiveImplementation requires a `size_reference()` method
 *  that is accessible by ArchiveAdapterAccess and returns a reference to an
 *  object of a type that meets the requirements of `Integral`
 */
template <typename SizingArchiveImplementation>
struct SizingArchiveAdapter : SizingArchiveImplementation {

  using is_sizing_archive_t = std::true_type;
  using is_archive_t = std::true_type;

  SizingArchiveAdapter() = default;
  SizingArchiveAdapter(SizingArchiveAdapter const&) = delete;
  SizingArchiveAdapter(SizingArchiveAdapter&&) = default;
  SizingArchiveAdapter& operator=(SizingArchiveAdapter const&) = default;
  SizingArchiveAdapter& operator=(SizingArchiveAdapter&&) = default;
  ~SizingArchiveAdapter() = default;

  using SizingArchiveImplementation::SizingArchiveImplementation;

  static constexpr bool is_sizing() { return true; }
  static constexpr bool is_packing() { return false; }
  static constexpr bool is_unpacking() { return false; }

  void add_to_size_raw(size_t size) {
    // should pretty much never be used, but whatever:
    ArchiveAdapterAccess::size_reference(*static_cast<SizingArchiveImplementation*>(this)) += size;
  }
};

/**
 * @todo document this
 * @tparam PackingArchiveImplementation
 *  PackingArchiveImplementation requires a `buffer_spot_reference()` method
 *  that is accessible by ArchiveAdapterAccess and returns an object contextually
 *  convertible to `char*&`
 */
template <typename PackingArchiveImplementation>
struct PackingArchiveAdapter : PackingArchiveImplementation {
  using is_packing_archive_t = std::true_type;
  using is_archive_t = std::true_type;

  PackingArchiveAdapter() = default;
  PackingArchiveAdapter(PackingArchiveAdapter const&) = delete;
  PackingArchiveAdapter(PackingArchiveAdapter&&) = default;
  PackingArchiveAdapter& operator=(PackingArchiveAdapter const&) = default;
  PackingArchiveAdapter& operator=(PackingArchiveAdapter&&) = default;
  ~PackingArchiveAdapter() = default;

  using PackingArchiveImplementation::PackingArchiveImplementation;

  static constexpr bool is_sizing() { return false; }
  static constexpr bool is_packing() { return true; }
  static constexpr bool is_unpacking() { return false; }

  template <typename ContiguousIterator>
  void pack_data_raw(ContiguousIterator begin, ContiguousIterator end) {
    using value_type =
      std::remove_const_t<std::remove_reference_t<decltype(*begin)>>;
    // Use memcpy, since std::copy invokes the assignment operator, and "raw"
    // implies that this isn't necessary
    auto size = std::distance(begin, end) * sizeof(value_type);
    char*& spot = ArchiveAdapterAccess::buffer_spot_reference(*static_cast<PackingArchiveImplementation*>(this));
    std::memcpy(spot, static_cast<void const*>(begin), size);
    spot += size;
  }
};

} // end namespace serialization
} // end namespace darma

#endif //DARMASERIALIZATION_ADAPTER_ACCESS_H
