#ifndef MATRIX_STACK_HPP
#define MATRIX_STACK_HPP

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

struct MatrixStack {
    std::vector<glm::mat4> st;
    MatrixStack() { st.push_back(glm::mat4(1.0f)); }
    const glm::mat4& top() const { return st.back(); }
    void push() { st.push_back(st.back()); }
    void pop()  { if (st.size() > 1) st.pop_back(); }

    // Transforms in local space
    void translate(const glm::vec3& v) { st.back() = glm::translate(st.back(), v); }
    void rotate(float angleRad, const glm::vec3& axis) { st.back() = glm::rotate(st.back(), angleRad, axis); }
    void scale(const glm::vec3& s) { st.back() = glm::scale(st.back(), s); }
};
#endif


