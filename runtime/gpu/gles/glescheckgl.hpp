
#ifndef GLESCHECKGL_HPP
#define GLESCHECKGL_HPP

namespace brook {

#define CHECK_GL() __check_gles(__LINE__, __FILE__, 1)
#define CHECK_GL_NOT_FATAL() __check_gles(__LINE__, __FILE__, 0)

  void __check_gles(int line, const char *file, bool fatal);

#define EGL_CHECK(x) \
    x; \
    { \
        EGLint eglError = eglGetError(); \
        if(eglError != EGL_SUCCESS) { \
            fprintf(stderr, "eglGetError() = %p (0x%.8x) at line %i in file %s\n", eglError, eglError, __LINE__, __FILE__); \
            exit(1); \
        } \
    }


}


#endif

