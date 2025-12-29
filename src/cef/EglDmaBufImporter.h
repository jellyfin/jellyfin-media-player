#pragma once

#ifdef __linux__

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES3/gl3.h>
#include <cstdint>

// Extension function types
typedef EGLImage (EGLAPIENTRYP PFNEGLCREATEIMAGEKHRPROC)(EGLDisplay dpy, EGLContext ctx, EGLenum target, EGLClientBuffer buffer, const EGLint *attrib_list);
typedef EGLBoolean (EGLAPIENTRYP PFNEGLDESTROYIMAGEKHRPROC)(EGLDisplay dpy, EGLImage image);
typedef void (GL_APIENTRYP PFNGLEGLIMAGETARGETTEXTURE2DOESPROC)(GLenum target, void* image);

class EglDmaBufImporter {
public:
    EglDmaBufImporter();
    ~EglDmaBufImporter();

    // Initialize with current EGL display
    bool init();
    void cleanup();

    // Check if DMA-BUF import is supported
    bool isSupported() const { return supported_; }

    // Import DMA-BUF as texture (returns GL texture ID, 0 on failure)
    // Caller owns the texture and must delete it
    GLuint importDmaBuf(int fd, int width, int height, uint32_t format,
                        uint64_t modifier, uint32_t stride, uint64_t offset);

    // Import and bind to existing texture
    bool importDmaBufToTexture(GLuint texture, int fd, int width, int height,
                                uint32_t format, uint64_t modifier,
                                uint32_t stride, uint64_t offset);

private:
    EGLDisplay display_ = EGL_NO_DISPLAY;
    bool supported_ = false;
    bool initialized_ = false;

    // Extension functions
    PFNEGLCREATEIMAGEKHRPROC eglCreateImageKHR_ = nullptr;
    PFNEGLDESTROYIMAGEKHRPROC eglDestroyImageKHR_ = nullptr;
    PFNGLEGLIMAGETARGETTEXTURE2DOESPROC glEGLImageTargetTexture2DOES_ = nullptr;
};

#endif // __linux__
