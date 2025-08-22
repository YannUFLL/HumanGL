#ifndef GPU_HPP
#define GPU_HPP

#include <glad/glad.h>
#include <glm/glm.hpp>

inline void setModel(GLint uModel, const glm::mat4 &M)
{
    glUniformMatrix4fv(uModel, 1, GL_FALSE, &M[0][0]);
}
#endif