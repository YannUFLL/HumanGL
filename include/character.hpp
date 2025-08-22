// character.hpp
#ifndef CHARACTER_HPP
#define CHARACTER_HPP

#include <glad/glad.h>
#include <glm/glm.hpp>
#include "MatrixStack.hpp"

// Ta config "taille" (comme avant)
struct RigParams {
    float torsoH = 1.4f, torsoW = 0.7f, torsoD = 0.4f;
    float headH = 0.55f;
    float upperArmL = 0.6f, foreArmL = 0.6f, armR = 0.20f;
    float thighL = 0.7f,  shinL   = 0.7f,  legR = 0.20f;
};

// Palette de couleurs (bonus autorisé : changer les couleurs) 
// cf. sujet "Bonus part" : changer la couleur via une interface, etc. 
struct RigColors {
    glm::vec4 head  {1.0f, 0.9f, 0.6f, 1.0f};
    glm::vec4 torso {0.2f, 0.4f, 0.8f, 1.0f};
    glm::vec4 arm   {0.6f, 0.9f, 0.7f, 1.0f};
    glm::vec4 leg   {0.9f, 0.6f, 0.6f, 1.0f};
};

enum class AnimMode { Idle, Walk, Jump };

// Renderer orienté "une pièce = un seul draw d’un cube 1×1×1"
// conforme aux contraintes du sujet. 
class CharacterRenderer {
public:
    CharacterRenderer(GLint uModel, GLint uColor, GLuint cubeVAO)
        : uModel_(uModel), uColor_(uColor), cubeVAO_(cubeVAO) {}

    void setRig(const RigParams& p)   { P_ = p; }
    void setColors(const RigColors& c){ C_ = c; }

    // Appel par frame
    void draw(float t, AnimMode mode, bool paused);

private:
    // Helpers "placement only" (pas de couleur, pas de draw)
    void placeTorso(MatrixStack& ms) const;
    void placeHeadFromTorso(MatrixStack& ms) const;
    void placeUpperArm(MatrixStack& ms) const;
    void placeForearm(MatrixStack& ms, float elbowRot) const;
    void placeThigh(MatrixStack& ms) const;
    void placeShin(MatrixStack& ms, float kneeRot) const;

    // Rendu bête : set model + set color + draw cube (UN seul draw par pièce)
    inline void renderPart(MatrixStack& ms, const glm::vec4& color) const;

    // Ressources / uniforms
    GLint  uModel_;
    GLint  uColor_;
    GLuint cubeVAO_;

    // État courant
    RigParams P_{};
    RigColors C_{};
};

#endif // CHARACTER_HPP
