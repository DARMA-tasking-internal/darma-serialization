/*
//@HEADER
// ************************************************************************
//
//                      copy_constructor_archive.cc
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

#include <darma/serialization/serialization_traits.h>
#include <darma/serialization/operator_overloads.h>
#include <darma/serialization/adapters/adapter_access.h>

#include <cstdlib>
#include <type_traits>
#include <cstring>
#include <memory>
#include <string>
#include <iostream>

// An example of how to hook directly into copy constructor behavior using
// the ADL-based customization points

struct CopySizingArchiveImplementation {
  size_t size = 0;
  // Interface for the SizingArchiveAdapter to use:
  size_t& size_reference() { return size; }
};
using CopySizingArchive =
  darma::serialization::SizingArchiveAdapter<CopySizingArchiveImplementation>;

struct CopyPackingArchiveImplementation {
  char* spot = nullptr;
  // Interface for the PackingArchiveAdapter to use:
  char*& buffer_spot_reference() { return spot; }
};
using CopyPackingArchive =
  darma::serialization::PackingArchiveAdapter<CopyPackingArchiveImplementation>;

template <typename Allocator=std::allocator<char>>
struct CopyUnpackingArchive {
  char* spot = nullptr;
  using allocator_type = typename std::allocator_traits<Allocator>::template rebind_alloc<char>;
  allocator_type allocator;

  template <typename RawDataType>
  void unpack_data_raw(void* allocated_dest, size_t n_items = 1) {
    std::memcpy(allocated_dest, spot, n_items * sizeof(RawDataType));
    spot += n_items * sizeof(RawDataType);
  }

  template <typename T>
  T unpack_next_item_as() {
    auto* old_spot = spot;
    spot += sizeof(T);
    return *(new T(std::move(*reinterpret_cast<T*>(old_spot))));
  }

  template <typename T>
  void unpack_next_item_at(void* allocated) {
    new (allocated) T(std::move(*reinterpret_cast<T*>(spot)));
    spot += sizeof(T);
  }

  auto const& get_allocator() const {
    return allocator;
  }
  auto& get_allocator() {
    return allocator;
  }

  template <typename NeededAllocatorT>
  NeededAllocatorT get_allocator_as() const {
    // Let's hope this is a compatible type
    return allocator;
  }

  static constexpr bool is_sizing() { return false; }
  static constexpr bool is_packing() { return false; }
  static constexpr bool is_unpacking() { return true; }
  using is_unpacking_archive_t = std::true_type;
  using is_archive_t = std::true_type;
};


// Ensure the "regular" serializer never gets used:
template <typename T>
void darma_compute_size(T const& obj, CopySizingArchive& ar) {
  ar.size += sizeof(T);
}

// Ensure the "regular" serializer never gets used:
template <typename T>
void darma_pack(T const& obj, CopyPackingArchive& ar) {
  new (ar.spot) T(obj);
  ar.spot += sizeof(T);
}

// Ensure the "regular" serializer never gets used:
template <typename T>
void darma_unpack(darma::serialization::allocated_buffer_for<T> alloc, CopyUnpackingArchive<>& ar) {
  ar.unpack_next_item_at<T>(alloc.pointer);
}


int main() {
  int i = 42;
  double val = 3.14;
  std::string str = "hello world";

  auto s_ar = CopySizingArchive{};
  s_ar | i | val | str;

  char buffer[s_ar.size];
  auto p_ar = CopyPackingArchive();
  p_ar.spot = buffer;
  p_ar | i | val | str;


  i = 0; val = 0; str = "";
  auto u_ar = CopyUnpackingArchive<>{buffer, std::allocator<char>()};
  u_ar | i | val | str;
  std::cout << i << ", " << val << ", " << str << std::endl;
}