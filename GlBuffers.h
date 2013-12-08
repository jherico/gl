/************************************************************************************

 Authors     :   Bradley Austin Davis <bdavis@saintandreas.org>
 Copyright   :   Copyright Brad Davis. All Rights reserved.

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

 http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.

 ************************************************************************************/

#pragma once

#ifndef GL_ZERO
#error "You must include the gl headers before including this file"
#endif

#include <vector>
#include <cassert>
#include <memory>

namespace gl {

class GlBufferLoader {
public:
  virtual ~GlBufferLoader() {
  }
  virtual const GLvoid* getData() = 0;
  virtual GLsizeiptr getSize() = 0;
};

template<class T> class VectorLoader : public GlBufferLoader {
public:
  const std::vector<T> & data;
  VectorLoader(const std::vector<T> & data)
      : data(data) {
  }

  const GLvoid* getData() {
    const T * ptr = &(this->data[0]);
    return ptr;
  }

  GLsizeiptr getSize() {
    return sizeof(T) * data.size();
  }

  operator GlBufferLoader &() {
    return *this;
  }

};

template<typename T> VectorLoader<T> makeVectorLoader(
    const std::vector<T> & vector) {
  return VectorLoader<T>(vector);
}

template<typename T, size_t SIZE> class ArrayLoader : public GlBufferLoader {
public:
  const T * data;

  ArrayLoader(T * data)
      : data(data) {
  }

  const GLvoid* getData() {
    return (GLvoid*) data;
  }

  GLsizeiptr getSize() {
    return sizeof(T) * SIZE;
  }
};

template<typename T, size_t SIZE> ArrayLoader<T, SIZE> makeArrayLoader(
    T * pointer) {
  return ArrayLoader<T, SIZE>(pointer);
}

template<typename T, size_t SIZE> ArrayLoader<T, SIZE> makeArrayLoader(
    T (&array)[SIZE]) {
  return ArrayLoader<T, SIZE>(array);
}

template<
    GLenum GlBufferType,
    GLenum GlUsageType = GL_STATIC_DRAW
>
class GlBuffer {
  GLuint buffer;
  public:
  GlBuffer()
      : buffer(0) {
    glGenBuffers(1, &buffer);
    assert(buffer != 0);
  }

  template<typename T> GlBuffer(std::vector<T> & data)
      : buffer(0) {
    glGenBuffers(1, &buffer);
    assert(buffer != 0);
    *this << gl::VectorLoader<T>(data);
  }

  template<typename T, size_t SIZE> GlBuffer(T (&array)[SIZE])
      : buffer(0) {
    glGenBuffers(1, &buffer);
    assert(buffer != 0);
    *this << gl::ArrayLoader<T, SIZE>(array);
  }

  GlBuffer(GlBuffer && other)
      : buffer(0) {
    std::swap(other.buffer, buffer);
  }

  virtual ~GlBuffer() {
    glDeleteBuffers(1, &buffer);
  }

  void bind() const {
    glBindBuffer(GlBufferType, buffer);
  }

  static void unbind() {
    glBindBuffer(GlBufferType, 0);
  }

  void operator <<(GlBufferLoader & loader) {
    load(loader);
  }

  void operator <<(GlBufferLoader && loader) {
    load(loader);
  }

  void load(GlBufferLoader & loader) {
    bind();
    glBufferData(GlBufferType, loader.getSize(), loader.getData(),
        GlUsageType);
  }
};

typedef GlBuffer<GL_ELEMENT_ARRAY_BUFFER> IndexBuffer;
typedef std::shared_ptr<IndexBuffer> IndexBufferPtr;

typedef GlBuffer<GL_ARRAY_BUFFER> VertexBuffer;
typedef std::shared_ptr<VertexBuffer> VertexBufferPtr;

} // gl

