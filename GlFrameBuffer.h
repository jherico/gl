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

#include "GlTexture.h"
#include "GlDebug.h"

namespace gl {

  template <
    class TextureType = Texture2d,
    class DepthType = Texture<GL_TEXTURE_2D, GL_DEPTH_COMPONENT16>
  >
struct FrameBuffer {
  typedef std::shared_ptr<TextureType> TexturePtr;
  typedef std::shared_ptr<DepthType> DepthPtr;
  GLuint frameBuffer;
  TexturePtr texture;
  DepthPtr depth;
  glm::ivec2 size;

  FrameBuffer(TexturePtr color = TexturePtr(), DepthPtr depth = DepthPtr())
      : frameBuffer(0), texture(color), depth(depth) {
  }

  virtual ~FrameBuffer() {
    if (frameBuffer) {
      glDeleteFramebuffers(1, &frameBuffer);
      GL_CHECK_ERROR;
    }
  }

  void init(const glm::ivec2 & size) {
    this->size = size;
    glGenFramebuffers(1, &frameBuffer);
    GL_CHECK_ERROR;

    bind();
    GL_CHECK_ERROR;

    if (!texture) {
      texture = TexturePtr(new TextureType());
      texture->bind();
      texture->parameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      texture->parameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      texture->parameter(GL_TEXTURE_WRAP_S, GL_CLAMP);
      texture->parameter(GL_TEXTURE_WRAP_T, GL_CLAMP);
      texture->image2d(size);
      TextureType::unbind();
    }
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, *texture, 0);

    GL_CHECK_ERROR;
    GLenum bufs = GL_COLOR_ATTACHMENT0;
    glDrawBuffers(1, &bufs);
    GL_CHECK_ERROR;

    if (!depth) {
      depth = DepthPtr(new DepthType());
      depth->bind();
      depth->parameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
      depth->parameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
      depth->parameter(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      depth->parameter(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      depth->image2d(size);
      DepthType::unbind();
    }
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, *depth, 0);

    if (!checkStatus(GL_FRAMEBUFFER)) {
      throw std::runtime_error("Bad framebuffer creation");
    }

    unbind();
  }

  void bind() {
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
  }

  void bindColor() {
    texture->bind(multisample ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D);
  }

  void unbindColor() {
    Texture::unbind(multisample ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D);
  }

  static void unbind() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }

  void activate() {
//    glBindTexture(GL_TEXTURE_2D, 0);
//    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
    viewport(size);
  }
  void deactivate() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
//    texture->bind();
//    glGenerateMipmap(GL_TEXTURE_2D);
//    glGenerateMipmap(GL_TEXTURE_2D);
//    texture->unbind();
  }

  TexturePtr & getTexture() {
    return texture;
  }

  TexturePtr detachTexture() {
    auto result = texture;
    texture = nullptr;
    return result;
  }

  static bool checkStatus(GLenum target) {
      GLuint status = glCheckFramebufferStatus(target);
      switch(status) {
      case GL_FRAMEBUFFER_COMPLETE:
          //printf("framebuffer check ok\n"); fflush(stdout);
          return true;
          break;

      case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
          printf("framebuffer incomplete attachment\n"); fflush(stdout);
          break;

      case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
          printf("framebuffer missing attachment\n"); fflush(stdout);
          break;

      case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
          printf("framebuffer incomplete dimensions\n"); fflush(stdout);
          break;

      case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
          printf("framebuffer incomplete formats\n"); fflush(stdout);
          break;

      case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
          printf("framebuffer incomplete draw buffer\n"); fflush(stdout);
          break;

      case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
          printf("framebuffer incomplete read buffer\n"); fflush(stdout);
          break;

      case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
          printf("framebuffer incomplete multisample\n"); fflush(stdout);
          break;

      case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS :
          printf("framebuffer incomplete layer targets\n"); fflush(stdout);
          break;

      case GL_FRAMEBUFFER_UNSUPPORTED:
          printf("framebuffer unsupported internal format or image\n"); fflush(stdout);
          break;

      default:
          printf("other framebuffer error\n"); fflush(stdout);
          break;
      }

      return false;
  }

};

} // gl


