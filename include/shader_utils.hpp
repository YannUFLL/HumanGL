#ifndef SHADER_UTILS_HPP
#define SHADER_UTILS_HPP

#include <glad/glad.h>

GLuint compileShader(GLenum type, const char *src);
GLuint linkProgram(GLuint vs, GLuint fs);

#endif
