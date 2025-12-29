#ifdef __linux__

#include "EglDmaBufImporter.h"
#include <libdrm/drm_fourcc.h>
#include <cstring>
#include <iostream>

// EGL DMA-BUF extension defines
#ifndef EGL_LINUX_DMA_BUF_EXT
#define EGL_LINUX_DMA_BUF_EXT 0x3270
#endif
#ifndef EGL_LINUX_DRM_FOURCC_EXT
#define EGL_LINUX_DRM_FOURCC_EXT 0x3271
#endif
#ifndef EGL_DMA_BUF_PLANE0_FD_EXT
#define EGL_DMA_BUF_PLANE0_FD_EXT 0x3272
#endif
#ifndef EGL_DMA_BUF_PLANE0_OFFSET_EXT
#define EGL_DMA_BUF_PLANE0_OFFSET_EXT 0x3273
#endif
#ifndef EGL_DMA_BUF_PLANE0_PITCH_EXT
#define EGL_DMA_BUF_PLANE0_PITCH_EXT 0x3274
#endif
#ifndef EGL_DMA_BUF_PLANE0_MODIFIER_LO_EXT
#define EGL_DMA_BUF_PLANE0_MODIFIER_LO_EXT 0x3443
#endif
#ifndef EGL_DMA_BUF_PLANE0_MODIFIER_HI_EXT
#define EGL_DMA_BUF_PLANE0_MODIFIER_HI_EXT 0x3444
#endif

EglDmaBufImporter::EglDmaBufImporter() = default;

EglDmaBufImporter::~EglDmaBufImporter() {
    cleanup();
}

bool EglDmaBufImporter::init() {
    if (initialized_) return supported_;

    initialized_ = true;

    // Get current EGL display
    display_ = eglGetCurrentDisplay();
    if (display_ == EGL_NO_DISPLAY) {
        std::cerr << "[EglDmaBufImporter] No current EGL display" << std::endl;
        return false;
    }

    // Check for required extensions
    const char* extensions = eglQueryString(display_, EGL_EXTENSIONS);
    if (!extensions) {
        std::cerr << "[EglDmaBufImporter] Failed to query EGL extensions" << std::endl;
        return false;
    }

    bool hasImageBase = strstr(extensions, "EGL_KHR_image_base") != nullptr;
    bool hasDmaBuf = strstr(extensions, "EGL_EXT_image_dma_buf_import") != nullptr;
    bool hasModifiers = strstr(extensions, "EGL_EXT_image_dma_buf_import_modifiers") != nullptr;

    std::cout << "[EglDmaBufImporter] EGL_KHR_image_base: " << (hasImageBase ? "yes" : "no") << std::endl;
    std::cout << "[EglDmaBufImporter] EGL_EXT_image_dma_buf_import: " << (hasDmaBuf ? "yes" : "no") << std::endl;
    std::cout << "[EglDmaBufImporter] EGL_EXT_image_dma_buf_import_modifiers: " << (hasModifiers ? "yes" : "no") << std::endl;

    if (!hasImageBase || !hasDmaBuf) {
        std::cerr << "[EglDmaBufImporter] Required extensions not available" << std::endl;
        return false;
    }

    // Load extension functions
    eglCreateImageKHR_ = reinterpret_cast<PFNEGLCREATEIMAGEKHRPROC>(
        eglGetProcAddress("eglCreateImageKHR"));
    eglDestroyImageKHR_ = reinterpret_cast<PFNEGLDESTROYIMAGEKHRPROC>(
        eglGetProcAddress("eglDestroyImageKHR"));
    glEGLImageTargetTexture2DOES_ = reinterpret_cast<PFNGLEGLIMAGETARGETTEXTURE2DOESPROC>(
        eglGetProcAddress("glEGLImageTargetTexture2DOES"));

    if (!eglCreateImageKHR_ || !eglDestroyImageKHR_ || !glEGLImageTargetTexture2DOES_) {
        std::cerr << "[EglDmaBufImporter] Failed to load extension functions" << std::endl;
        return false;
    }

    supported_ = true;
    std::cout << "[EglDmaBufImporter] Initialized successfully" << std::endl;
    return true;
}

void EglDmaBufImporter::cleanup() {
    display_ = EGL_NO_DISPLAY;
    supported_ = false;
    initialized_ = false;
}

GLuint EglDmaBufImporter::importDmaBuf(int fd, int width, int height, uint32_t format,
                                        uint64_t modifier, uint32_t stride, uint64_t offset) {
    GLuint texture = 0;
    glGenTextures(1, &texture);
    if (texture == 0) return 0;

    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    if (!importDmaBufToTexture(texture, fd, width, height, format, modifier, stride, offset)) {
        glDeleteTextures(1, &texture);
        return 0;
    }

    return texture;
}

bool EglDmaBufImporter::importDmaBufToTexture(GLuint texture, int fd, int width, int height,
                                               uint32_t format, uint64_t modifier,
                                               uint32_t stride, uint64_t offset) {
    if (!supported_ || display_ == EGL_NO_DISPLAY) {
        return false;
    }

    // Validate fd
    if (fd < 0) {
        std::cerr << "[EglDmaBufImporter] Invalid fd: " << fd << std::endl;
        return false;
    }

    // Validate dimensions
    if (width <= 0 || height <= 0 || width > 16384 || height > 16384) {
        std::cerr << "[EglDmaBufImporter] Invalid dimensions: " << width << "x" << height << std::endl;
        return false;
    }

    // CEF passes cef_color_type_t, not DRM fourcc
    // CEF internally uses BGRA regardless of the reported type
    uint32_t drm_format = DRM_FORMAT_ARGB8888;
    (void)format;  // CEF format field not reliable
    format = drm_format;

    EGLint attribs[] = {
        EGL_WIDTH, width,
        EGL_HEIGHT, height,
        EGL_LINUX_DRM_FOURCC_EXT, static_cast<EGLint>(format),
        EGL_DMA_BUF_PLANE0_FD_EXT, fd,
        EGL_DMA_BUF_PLANE0_OFFSET_EXT, static_cast<EGLint>(offset),
        EGL_DMA_BUF_PLANE0_PITCH_EXT, static_cast<EGLint>(stride),
        EGL_DMA_BUF_PLANE0_MODIFIER_LO_EXT, static_cast<EGLint>(modifier & 0xFFFFFFFF),
        EGL_DMA_BUF_PLANE0_MODIFIER_HI_EXT, static_cast<EGLint>(modifier >> 32),
        EGL_NONE
    };

    EGLImage image = eglCreateImageKHR_(display_, EGL_NO_CONTEXT,
                                         EGL_LINUX_DMA_BUF_EXT, nullptr, attribs);

    if (image == EGL_NO_IMAGE_KHR) {
        EGLint err = eglGetError();
        std::cerr << "[EglDmaBufImporter] eglCreateImageKHR failed: 0x"
                  << std::hex << err << std::dec
                  << " (fd=" << fd << " " << width << "x" << height
                  << " stride=" << stride << ")" << std::endl;
        return false;
    }

    // Bind image to texture
    glBindTexture(GL_TEXTURE_2D, texture);
    glEGLImageTargetTexture2DOES_(GL_TEXTURE_2D, image);

    GLenum glErr = glGetError();
    if (glErr != GL_NO_ERROR) {
        std::cerr << "[EglDmaBufImporter] glEGLImageTargetTexture2DOES failed: 0x"
                  << std::hex << glErr << std::dec << std::endl;
        eglDestroyImageKHR_(display_, image);
        return false;
    }

    // Image can be destroyed after binding to texture
    eglDestroyImageKHR_(display_, image);

    return true;
}

#endif // __linux__
