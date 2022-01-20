#if defined(DM_PLATFORM_HTML5) || defined(DM_PLATFORM_ANDROID)
	#define GL_GLEXT_PROTOTYPES
    #include <GLES2/gl2.h>              // OpenGL ES 2.0 library
    #include <GLES2/gl2ext.h>
#else
	#define GLAD_GL_IMPLEMENTATION
	#define KHRONOS_STATIC 1
	#include <raylib/external/glad.h>                  // Required for OpenGL API
	#include <dmsdk/graphics/glfw/glfw.h>
#endif

#if defined(DM_PLATFORM_HTML5) || defined(DM_PLATFORM_ANDROID)
	#define SHADERS_PATH "/resources/shaders/web/"
#else
	#define SHADERS_PATH "/resources/shaders/"
#endif

#ifdef DEBUG_OPENGL_CALLS
	void CheckOpenGLError(const char* stmt, const char* fname, int line) {
		GLenum err = glGetError();
		if (err != GL_NO_ERROR) {
			dmLogError("OpenGL error %08x, at %s:%i - for %s\n", err, fname, line, stmt);
		}
	}

	void CheckOpenGLErrorLabel(const char* label, const char* fname, int line) {
		GLenum err = glGetError();
		if (err != GL_NO_ERROR) {
			dmLogError("OpenGL error %08x, at %s\n", err, label);
		}
	}
    #define GL_CHECK(stmt) do { \
            stmt; \
            CheckOpenGLError(#stmt, __FILE__, __LINE__); \
        } while (0)

	#define GL_CHECKL(label) CheckOpenGLError(label, __FILE__, __LINE__)
#else
    #define GL_CHECK(stmt) stmt
	#define GL_CHECKL(label)
#endif