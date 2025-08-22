#ifndef CHARACTER_HPP
#define CHARACTER_HPP

#include <glad/glad.h>
#include "MatrixStack.hpp"

struct RigParams {
    float torsoH = 1.4f, torsoW = 0.7f, torsoD = 0.4f;
    float headH = 0.55f;
    float upperArmL = 0.6f, foreArmL = 0.6f, armR = 0.20f;
    float thighL = 0.7f,  shinL   = 0.7f,  legR = 0.20f;
};

enum class AnimMode { Idle, Walk, Jump };

// Déclare des fonctions de haut niveau
void draw_character(GLint uModel, GLuint cubeVAO, const RigParams& P, float t,
                    AnimMode mode, bool paused);

#endif 