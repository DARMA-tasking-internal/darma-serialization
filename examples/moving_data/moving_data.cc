/*
//@HEADER
// ************************************************************************
//
//                      moving_data.cc
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

#include <darma/serialization/simple_handler.h>
#include <darma/serialization/serializers/all.h>

#include <darma/utility/demangle.h>

#include <iostream>

//==============================================================================
// Simple SerializationHandler use:
//   (for when you only need to pack one object or whatever)
//==============================================================================

template <typename SerializationHandler, typename Object>
auto pack_it_the_easy_way(Object obj) {

  std::cout << "Serializing an object into a buffer:" << std::endl;
  std::cout << "  -> Value: " << obj << std::endl;

  //----------------------------------------------------------------------------
  // Here's all you  have to do:
  auto buffer = SerializationHandler::serialize(obj);
  //----------------------------------------------------------------------------

  std::cout << "  -> Buffer size: " << buffer.capacity() << std::endl;
  std::cout << "  -> Buffer data starts at: 0x" << std::hex << intptr_t(buffer.data()) << std::dec << std::endl;

  return std::move(buffer);
};

template <typename SerializationHandler, typename T, typename SerializationBuffer>
void unpack_it_the_easy_way(SerializationBuffer&& buffer) {
  std::cout << "Deserializing an object into a buffer:" << std::endl;
  std::cout << "  -> Buffer size: " << buffer.capacity() << std::endl;
  std::cout << "  -> Buffer data starts at: 0x" << std::hex << intptr_t(buffer.data()) << std::dec << std::endl;

  //----------------------------------------------------------------------------
  // Here's all you  have to do:
  T value = SerializationHandler::template deserialize<T>(std::forward<SerializationBuffer>(buffer));
  //----------------------------------------------------------------------------

  std::cout << "  -> When unpacked as type " << darma::utility::try_demangle<T>::name() << "," << std::endl;
  std::cout << "       value is: " << value << std::endl;
};


//==============================================================================
// Serializing using Archives
//   (for when you need to pass something off to somewhere else, or when you
//   need finer-grained control, etc.)
//==============================================================================

template <typename SerializationHandler, typename Object, typename Object2, typename Object3>
auto pack_things_with_archives(Object obj, Object2 obj2, Object3 obj3) {

  std::cout << "Serializing objects into a buffer:" << std::endl;
  std::cout << "  -> Values: " << obj << ", " << obj2 << ", " << obj3 << std::endl;

  //----------------------------------------------------------------------------
  // Here's what you do:
  // Return type meets the requirements of SizingArchive
  auto sizing_archive = SerializationHandler::make_sizing_archive();
  sizing_archive | obj | obj2 | obj3;
  // You can peek at the size using the SerializationHandler::get_size(SizingArchive)
  // static method requirement:
  std::cout << "  -> Size (according to SizingArchive): " << SerializationHandler::get_size(sizing_archive) << std::endl;
  // Make the packing archive from the sizing one (return type meets the
  // requirements of PackingArchive)
  auto packing_archive = SerializationHandler::make_packing_archive(std::move(sizing_archive));
  packing_archive | obj | obj2 | obj3;
  // You can now extract the buffer from the PackingArchive when you're done with it:
  auto buffer = SerializationHandler::extract_buffer(std::move(packing_archive));
  //----------------------------------------------------------------------------

  std::cout << "  -> Buffer size: " << buffer.capacity() << std::endl;
  std::cout << "  -> Buffer data starts at: 0x" << std::hex << intptr_t(buffer.data()) << std::dec << std::endl;

  return std::move(buffer);
};

template <typename SerializationHandler, typename SerializationBuffer, typename Object, typename Object2, typename Object3>
void unpacking_things_with_archives(SerializationBuffer&& buffer, Object& obj, Object2& obj2, Object3& obj3) {
  std::cout << "Unpacking objects from a buffer:" << std::endl;
  std::cout << "  -> Buffer size: " << buffer.capacity() << std::endl;
  std::cout << "  -> Buffer data starts at: 0x" << std::hex << intptr_t(buffer.data()) << std::dec << std::endl;

  //----------------------------------------------------------------------------
  // Here's what you do:
  // You can construct the UnpackingArchive directly from any compatible object
  // meeting the requirements of SerializationBuffer:
  auto unpacking_archive = SerializationHandler::make_unpacking_archive(std::forward<SerializationBuffer>(buffer));
  unpacking_archive | obj | obj2 | obj3;
  //----------------------------------------------------------------------------

  std::cout << "  -> Values: " << obj << ", " << obj2 << ", " << obj3 << std::endl;
};

//==============================================================================

int main() {

  // Template parameter to SimpleSerializationHandler is an allocator that it
  // uses to create buffers; defaults to std::allocator
  using handler_t = darma::serialization::SimpleSerializationHandler<>;

  int i = 42;
  double val = 3.14;
  std::string my_str = "hello world";

  //----------------------------------------------------------------------------
  {
    auto buffer = pack_it_the_easy_way<handler_t>(i);
    unpack_it_the_easy_way<handler_t, int>(std::move(buffer));

    buffer = pack_it_the_easy_way<handler_t>(val);
    unpack_it_the_easy_way<handler_t, double>(std::move(buffer));

    buffer = pack_it_the_easy_way<handler_t>(my_str);
    unpack_it_the_easy_way<handler_t, std::string>(std::move(buffer));
  }
  //----------------------------------------------------------------------------

  std::cout << "\n----------------------------------------\n" << std::endl;

  //----------------------------------------------------------------------------
  {
    auto buffer = pack_things_with_archives<handler_t>(i, val, my_str);
    // let's reset i, val, and my_str so that it's clear that we're not cheating
    i = 0; val = 0.0; my_str = "";
    unpacking_things_with_archives<handler_t>(std::move(buffer), i, val, my_str);
  }
  //----------------------------------------------------------------------------

  std::cout << "\n----------------------------------------\n" << std::endl;

  //----------------------------------------------------------------------------
  {
    auto buffer = pack_things_with_archives<handler_t>(i, val, my_str);

    // You don't always have to use the same buffer:
    char message[buffer.capacity()];
    std::memcpy(message, buffer.data(), buffer.capacity()); // here's our fake MPI_Send or whatever
    // Create a serialization buffer that refers to the message off of the wire:
    auto mbuff = darma::serialization::NonOwningSerializationBuffer(message, buffer.capacity());

    // Since `mbuff` is of a type that meets the requirements of SerializationBuffer,
    // it can be used anywhere we were using `buffer`:

    // let's reset i, val, and my_str so that it's clear that we're not cheating
    i = 0; val = 0.0; my_str = "";
    unpacking_things_with_archives<handler_t>(mbuff, i, val, my_str);
  }
  //----------------------------------------------------------------------------

}
