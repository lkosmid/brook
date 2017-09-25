
#ifndef GLESCHECKGL_HPP
#define GLESCHECKGL_HPP

namespace brook {

#ifdef RELEASE
#define CHECK_GL() ; 
#define CHECK_GL_NOT_FATAL() ; 
#else
#define CHECK_GL() __check_gles(__LINE__, __FILE__, 1)
#define CHECK_GL_NOT_FATAL() __check_gles(__LINE__, __FILE__, 0)
#endif

  void __check_gles(int line, const char *file, bool fatal);

#ifdef RELEASE
 #define EGL_CHECK(x) \
    x; 
#else
 #define EGL_CHECK(x) \
     x; \
    { \
        EGLint eglError = eglGetError(); \
        if(eglError != EGL_SUCCESS) { \
            fprintf(stderr, "eglGetError() = %p (0x%.8x) at line %i in file %s\n", eglError, eglError, __LINE__, __FILE__); \
            exit(1); \
        } \
    }
#endif


}


#endif

