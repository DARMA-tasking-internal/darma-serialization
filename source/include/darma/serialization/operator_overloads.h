/*
//@HEADER
// ************************************************************************
//
//                      operator_overloads.h
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

#ifndef DARMASERIALIZATION_OPERATOR_OVERLOADS_H
#define DARMASERIALIZATION_OPERATOR_OVERLOADS_H

#include "serialization_traits.h"
#include "archive_concept.h"

#include <type_traits>
#include <memory>

namespace darma {
namespace serialization {
namespace detail {

template <typename Archive, typename T>
using _has_darma_compute_size_archetype = decltype(
  darma_compute_size(std::declval<T const&>(), std::declval<Archive&>())
);

template <typename Archive, typename T>
using _has_darma_pack_archetype = decltype(
  darma_pack(std::declval<T const&>(), std::declval<Archive&>())
);

template <typename Archive, typename T>
using _has_darma_unpack_archetype = decltype(
  darma_unpack(std::declval<darma::serialization::allocated_buffer_for<T>>(), std::declval<Archive&>())
);

} // end namespace detail
} // end namespace serialization
} // end namespace darma

// Put heavily-constrained operator| overloads in the global namespace

template <typename Archive, typename T>
std::enable_if_t<
  darma::serialization::is_archive_v<Archive>
  and darma::serialization::is_sizing_archive_v<Archive>
  and tinympl::is_detected_exact<void,
    darma::serialization::detail::_has_darma_compute_size_archetype,
    Archive, T
  >::value,
  Archive&
>
operator|(Archive& ar, T const& val) {
  // Call with an unqualified name to trigger ADL
  darma_compute_size(val, ar);
  return ar;
};

template <typename Archive, typename T>
std::enable_if_t<
  darma::serialization::is_archive_v<Archive>
    and darma::serialization::is_sizing_archive_v<Archive>
    and tinympl::is_detected_exact<void,
      darma::serialization::detail::_has_darma_compute_size_archetype,
      Archive, T
    >::value,
  Archive&
>
operator%(Archive& ar, T const& val) {
  // Call with an unqualified name to trigger ADL
  darma_compute_size(val, ar);
  return ar;
};

template <typename Archive, typename T>
std::enable_if_t<
  darma::serialization::is_archive_v<Archive>
    and darma::serialization::is_packing_archive_v<Archive>
    and tinympl::is_detected_exact<void,
      darma::serialization::detail::_has_darma_pack_archetype,
      Archive, T
    >::value,
  Archive&
>
operator|(Archive& ar, T const& val) {
  // Call with an unqualified name to trigger ADL
  darma_pack(val, ar);
  return ar;
};

template <typename Archive, typename T>
std::enable_if_t<
  darma::serialization::is_archive_v<Archive>
    and darma::serialization::is_packing_archive_v<Archive>
    and tinympl::is_detected_exact<void,
      darma::serialization::detail::_has_darma_pack_archetype,
      Archive, T
    >::value,
  Archive&
>
operator<<(Archive& ar, T const& val) {
  // Call with an unqualified name to trigger ADL
  darma_pack(val, ar);
  return ar;
};


template <typename Archive, typename T>
std::enable_if_t<
  darma::serialization::is_archive_v<Archive>
    and darma::serialization::is_unpacking_archive_v<Archive>
    and tinympl::is_detected_exact<void,
      darma::serialization::detail::_has_darma_unpack_archetype,
      Archive, T
    >::value,
  Archive&
>
operator|(Archive& ar, T& val) {
  // First, destroy val
  using alloc_t = typename std::allocator_traits<typename Archive::allocator_type>::template rebind_alloc<T>;
  alloc_t alloc = ar.template get_allocator_as<alloc_t>();
  std::allocator_traits<alloc_t>::destroy(alloc, &val);
  // Call with an unqualified name to trigger ADL
  darma_unpack(darma::serialization::allocated_buffer_for<T>{static_cast<void*>(&val)}, ar);
  return ar;
};

template <typename Archive, typename T>
std::enable_if_t<
  darma::serialization::is_archive_v<Archive>
    and darma::serialization::is_unpacking_archive_v<Archive>
    and tinympl::is_detected_exact<void,
      darma::serialization::detail::_has_darma_unpack_archetype,
      Archive, T
    >::value,
  Archive&
>
operator>>(Archive& ar, T& val) {
  // First, destroy val
  using alloc_t = typename std::allocator_traits<typename Archive::allocator_type>::template rebind_alloc<T>;
  alloc_t alloc = ar.template get_allocator_as<alloc_t>();
  std::allocator_traits<alloc_t>::destroy(alloc, &val);
  // Call with an unqualified name to trigger ADL
  darma_unpack(darma::serialization::allocated_buffer_for<T>{static_cast<void*>(&val)}, ar);
  return ar;
};

#endif //DARMASERIALIZATION_OPERATOR_OVERLOADS_H
