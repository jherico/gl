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


struct FrameBuffer {
  GLuint frameBuffer;
  bool multisample;
  TexturePtr texture;
  TexturePtr depth;
  //GLuint texture;
  GLuint depthBuffer;
  GLsizei width, height;

  FrameBuffer(TexturePtr color, TexturePtr depth = TexturePtr())
      : frameBuffer(0), multisample(multisample), texture(color), depth(depth),
        depthBuffer(0), width(0), height(
          0) {
  }

  FrameBuffer(bool multisample = false)
      : frameBuffer(0), multisample(multisample), texture(nullptr), depthBuffer(0), width(0), height(
          0) {
  }

  virtual ~FrameBuffer() {
    if (frameBuffer) {
      glDeleteFramebuffers(1, &frameBuffer);
      GL_CHECK_ERROR;
    }

    if (depthBuffer) {
      glDeleteRenderbuffers(1, &depthBuffer);
      GL_CHECK_ERROR;
    }
  }

  void init(const glm::ivec2 & size, bool multisample_ = false) {
    this->width = size.x;
    this->height = size.y;
    this->multisample = multisample_;

    glGenFramebuffers(1, &frameBuffer);
    bind();
    glFramebufferParameteri(GL_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_WIDTH, width);
    glFramebufferParameteri(GL_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_HEIGHT, height);
    GL_CHECK_ERROR;

    GLint numSamples = 0;
    glGetIntegerv(GL_MAX_SAMPLES_EXT, &numSamples);

    numSamples = std::min(8, numSamples);

    if (!texture) {
      texture = TexturePtr(new Texture());
      if (multisample) {
        texture->bind(GL_TEXTURE_2D_MULTISAMPLE);
        glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, numSamples, GL_RGBA8, width, height, false);
      } else {
        texture->bind(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_FLOAT, 0);
      }
    }

    if (multisample) {
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, *texture, 0);
      Texture::unbind(GL_TEXTURE_2D_MULTISAMPLE);
    } else {
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, *texture, 0);
      Texture::unbind(GL_TEXTURE_2D);
    }

    GL_CHECK_ERROR;
    GLenum bufs = GL_COLOR_ATTACHMENT0;
    glDrawBuffers(1, &bufs);
    GL_CHECK_ERROR;

    if (!depth) {
      depth = TexturePtr(new Texture());
      if (multisample) {
        depth->bind(GL_TEXTURE_2D_MULTISAMPLE);
        glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, numSamples, GL_DEPTH_COMPONENT16, width, height, false);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, *depth, 0);
        Texture::unbind(GL_TEXTURE_2D_MULTISAMPLE);
      } else {
        depth->bind(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, *depth, 0);
        Texture::unbind(GL_TEXTURE_2D);
      }
    }
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
    glViewport(0, 0, width, height);
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


