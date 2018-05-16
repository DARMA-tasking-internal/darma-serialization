/*
//@HEADER
// ************************************************************************
//
//                      handler_concept.h
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

#ifndef DARMASERIALIZATION_HANDLER_CONCEPT_H
#define DARMASERIALIZATION_HANDLER_CONCEPT_H

/**
 *  @file handler_concept.h
 *  @brief Definitions, traits, and helpers for the SerializationHandler concept
 *
 *  @concept SerializationHandler
 *
 *  #### By example:
 *
 *  ~~~~{.cpp}
 *  // MySerializationHandler meets the requirements of SerializationHandler:
 *  struct MySerializationHandler {
 *    private:
 *
 *      // exposition only:
 *      struct MySizingArchive; // meets SizingArchive requirements
 *      struct MyPackingArchive; // meets PackingArchive requirements
 *      struct MyUnpackingArchive; // meets UnpackingArchive requirements
 *      struct MySerializationBuffer; // meets SerializationBuffer requirements
 *
 *    public:
 *
 *      template <typename T>
 *      static constexpr bool compatible_sizing_archive_v =
 *        std::is_same<T, MySizingArchive>::value;
 *
 *      template <typename T>
 *      static constexpr bool compatible_packing_archive_v =
 *        std::is_same<T, MyPackingArchive>::value;
 *
 *      template <typename T>
 *      static constexpr bool compatible_unpacking_archive_v =
 *        std::is_same<T, MyUnpackingArchive>::value;
 *
 *      MySizingArchive make_sizing_archive();
 *
 *      MyPackingArchive make_packing_archive(MySizingArchive&&);
 *
 *      template <typename SerializationBuffer>
 *      MyUnpackingArchive make_unpacking_archive(SerializationBuffer const& buffer);
 *
 *      // "Peak" at the size of a compatible SizingArchive
 *      size_t get_size(MySizingArchive const&);
 *
 *      // extract the packed data from a packing archive
 *      MySerializationBuffer extract_buffer(MyPackingArchive&&);
 *
 *      // convenience: do the serialization all in one step
 *      template <typename... Ts>
 *      MySerializationBuffer serialize(Ts const&...);
 *
 *      // convenience: unpack an item directly from a buffer
 *      template <typename T, typename Serialization Buffer>
 *      void deserialize(SerializationBuffer const&, void* dest);
 *  };
 *  ~~~~
 *
 *  #### Formal specification:
 *
 *  Given:
 *
 *  - `SH`, a type that meets the requirements of `SerializationHandler`
 *  - `sh`, a value of type `SH`
 *  - `T`, any complete type
 *  - `SAR`, a type that meets the requirements of `SizingArchive`
 *    and for which `SH::compatible_sizing_archive_v<SAR>` is `true`.
 *  - `sar`, a value of type `SAR`
 *  - `rsar`, an rvalue of type `SAR`
 *  - `PAR`, a type that meets the requirements of `PackingArchive`
 *    and for which `SH::compatible_packing_archive_v<PAR>` is `true`.
 *  - `par`, a value of type `PAR`
 *  - `rpar`, an rvalue of type `PAR`
 *  - `UAR`, a type that meets the requirements of `UnpackingArchive`
 *    and for which `SH::compatible_unpacking_archive_v<UAR>` is `true`.
 *  - `uar`, a value of type `UAR`
 *  - `SB`, a type that meets the requirements of `SerializationBuffer`
 *  - `rsb`, an rvalue of type `SB`,
 *  - `s`, a value of a type that meets the requirements of `Integral`
 *
 *  the type `SH` shall meet the requirements of `MoveConstructible`,
 *  `MoveAssignable`, and the requirements of described below:
 *
 *  - *Expression:* `SH::is_serialization_handler_t`
 *    + *Return Type:* Is or inherits from `true_type`
 *    + *Remark:* Tag used to shortcut detection of other concept requirements
 *      when performing concept checking. Represents a contract with the compiler that `SH`
 *      fullfills the requirements of `SerializationHandler`
 *  - *Expression:* `SH::compatible_sizing_archive_v<T>`
 *    + *Return Type:* Contextually convertible to `bool`
 *  - *Expression:* `SH::compatible_packing_archive_v<T>`
 *    + *Return Type:* Contextually convertible to `bool`
 *  - *Expression:* `SH::compatible_unpacking_archive_v<T>`
 *    + *Return Type:* Contextually convertible to `bool`
 *  - *Expression:* `sh.make_sizing_archive()`
 *    + *Return Type:* A type `RSAR`, such that
 *      - `RSAR` meets the requirements of `SizingArchive`
 *      - `SH::compatible_sizing_archive_v<RSAR>` is `true`
 *  - *Expression:* `sh.make_packing_archive(rsar)`
 *    + *Return Type:* A type `RPAR`, such that
 *      - `RPAR` meets the requirements of `PackingArchive`
 *      - `SH::compatible_packing_archive_v<RPAR>` is `true`
 *    + *Operational Semantics:*
 *      - *Preconditions:* `rsar` is valid for use as a `SizingArchive`
 *      - *Returns:* A `PackingArchive` that is at least large
 *        enough to hold all of the objects sized with `rsar` in its lifetime.
 *      - *Postconditions:* `rsar` is no longer valid for use as a `SizingArchive`
 *
 *  @todo finish formal specification of concept
 *
 *
 */

#include <tinympl/detection.hpp>

#include <type_traits>

namespace darma {
namespace serialization {

/** @brief Binary type trait indicating whether `T` meets the requirements of `SerializationHandler`
 */
template <typename T>
struct is_serialization_handler {
  private:

    template <typename U>
    using _tag_detection_archetype = typename U::is_serialization_handler_t;

  public:
    // TODO implement detection of other concept requirements
    using value = tinympl::detected_or_t<std::false_type,
      _tag_detection_archetype, T
    >::value;
};

template <typename T>
constexpr auto is_serialization_handler_v = is_serialization_handler<T>::value;


} // end namespace serialization
} // end namespace darma

#endif //DARMASERIALIZATION_HANDLER_CONCEPT_H
