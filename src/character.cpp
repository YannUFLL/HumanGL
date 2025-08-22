// character.cpp
#include "character.hpp"
#include "gpu.hpp"
#include "cube.hpp"
#include <cmath>
#include <algorithm>

// ---------------------- Helpers: placement only (pas de draw/couleur) ----------------------

void CharacterRenderer::placeTorso(MatrixStack &ms) const
{
    // Le cube unité est centré => on scale directement autour du centre du torse
    ms.scale({P_.torsoW, P_.torsoH, P_.torsoD});
}

void CharacterRenderer::placeHeadFromTorso(MatrixStack &ms) const
{
    // Depuis le centre du torse → haut du torse + moitié de la tête, puis scale
    ms.translate({0.0f, P_.torsoH * 0.5f + P_.headH * 0.5f, 0.0f});
    ms.scale({P_.headH * 0.8f, P_.headH, P_.headH * 0.8f});
}

void CharacterRenderer::placeUpperArm(MatrixStack &ms) const
{
    // Cube centré → décaler de la moitié de la longueur pour accrocher à l’épaule
    ms.translate({0.0f, -P_.upperArmL * 0.5f, 0.0f});
    ms.scale({P_.armR, P_.upperArmL, P_.armR});
}

void CharacterRenderer::placeForearm(MatrixStack &ms, float elbowRot) const
{
    // Depuis l’épaule : aller au coude, plier, puis centrer le segment
    ms.translate({0.0f, -P_.upperArmL, 0.0f});       // coude
    ms.rotate(elbowRot, {1, 0, 0});                  // flexion du coude
    ms.translate({0.0f, -P_.foreArmL * 0.5f, 0.0f}); // centrer l’os
    ms.scale({P_.armR * 0.95f, P_.foreArmL, P_.armR * 0.95f});
}

void CharacterRenderer::placeThigh(MatrixStack &ms) const
{
    // Depuis la hanche : centrer la cuisse
    ms.translate({0.0f, -P_.thighL * 0.5f, 0.0f});
    ms.scale({P_.legR, P_.thighL, P_.legR});
}

void CharacterRenderer::placeShin(MatrixStack &ms, float kneeRot) const
{
    // Depuis la hanche : aller au genou, plier, puis centrer le tibia
    ms.translate({0.0f, -P_.thighL, 0.0f});       // genou
    ms.rotate(kneeRot, {1, 0, 0});                // flexion du genou
    ms.translate({0.0f, -P_.shinL * 0.5f, 0.0f}); // centrer l’os
    ms.scale({P_.legR * 0.95f, P_.shinL, P_.legR * 0.95f});
}

// ---------------------- Rendu bête (1×1×1 à l’origine) : un seul draw par pièce ----------------------

inline void CharacterRenderer::renderPart(MatrixStack &ms, const glm::vec4 &color) const
{
    setModel(uModel_, ms.top());
    glUniform4fv(uColor_, 1, &color[0]);
    draw_unit_cube(cubeVAO_);
}

// ---------------------- Assemblage hiérarchique + animation ----------------------

void CharacterRenderer::draw(float t, AnimMode mode, bool paused)
{
    MatrixStack ms;

    const float tt = paused ? 0.0f : t;

    float walk = 0.0f;
    float elbow = 0.0f;
    float hip = 0.0f;
    float knee = 0.0f;
    float bounce = 0.0f;

    switch (mode)
    {
    case AnimMode::Idle:
        // tout à 0
        break;

    case AnimMode::Walk:
        walk = std::sin(tt * 2.0f) * 0.6f;
        elbow = std::sin(tt * 2.0f + 1.57f) * 0.4f;
        hip = walk;
        knee = std::max(0.0f, -std::sin(tt * 2.0f)) * 0.8f;
        bounce = std::abs(std::sin(tt * 2.0f)) * 0.05f;
        break;

    case AnimMode::Jump:
    {
        const float s = std::sin(tt * 3.0f);
        hip = std::max(0.0f, -s) * 0.5f;
        knee = std::max(0.0f, -s) * 1.2f;   // genoux se plient en flexion
        elbow = 0.2f * s;                   // bouger un peu les bras
        bounce = std::max(0.0f, s) * 0.25f; // montée pendant extension
    }
    break;
    }

    // Légère oscillation du torse + rebond vertical
    ms.translate({0.0f, bounce, 0.0f});
    ms.rotate(0.2f * std::sin(tt * 0.7f), {0, 1, 0});

    // ---- Racine "torse" : tout le monde en hérite ----
    ms.push();
    { // frame du torse (centre)
        // Torse
        ms.push();
        {
            placeTorso(ms);
            renderPart(ms, C_.torso);
        }
        ms.pop();

        // Tête (enfant direct du torse)
        ms.push();
        {
            placeHeadFromTorso(ms);
            renderPart(ms, C_.head);
        }
        ms.pop();

        // ---- Bras droit : pivot d’épaule D gardé le temps bras+avant-bras ----
        ms.push();
        {
            ms.translate({+(P_.torsoW * 0.5f + P_.armR), +P_.torsoH * 0.35f, 0.0f}); // pivot épaule D
            ms.rotate(-walk, {1, 0, 0});                                             // rotation d'épaule (parent)
            // Haut du bras
            ms.push();
            {
                placeUpperArm(ms);
                renderPart(ms, C_.arm);
            }
            ms.pop();
            // Avant-bras
            ms.push();
            {
                placeForearm(ms, elbow);
                renderPart(ms, C_.arm);
            }
            ms.pop();
        }
        ms.pop();

        // ---- Bras gauche ----
        ms.push();
        {
            ms.translate({-(P_.torsoW * 0.5f + P_.armR), +P_.torsoH * 0.35f, 0.0f}); // pivot épaule G
            ms.rotate(+walk, {1, 0, 0});
            ms.push();
            {
                placeUpperArm(ms);
                renderPart(ms, C_.arm);
            }
            ms.pop();
            ms.push();
            {
                placeForearm(ms, -elbow);
                renderPart(ms, C_.arm);
            }
            ms.pop();
        }
        ms.pop();

        // ---- Jambe droite : pivot hanche D gardé le temps cuisse+tibia ----
        ms.push();
        {
            ms.translate({+P_.torsoW * 0.25f, -P_.torsoH * 0.5f, 0.0f}); // pivot hanche D
            ms.rotate(+hip, {1, 0, 0});
            ms.push();
            {
                placeThigh(ms);
                renderPart(ms, C_.leg);
            }
            ms.pop();
            ms.push();
            {
                placeShin(ms, knee);
                renderPart(ms, C_.leg);
            }
            ms.pop();
        }
        ms.pop();

        // ---- Jambe gauche ----
        ms.push();
        {
            ms.translate({-P_.torsoW * 0.25f, -P_.torsoH * 0.5f, 0.0f}); // pivot hanche G
            ms.rotate(-hip, {1, 0, 0});
            ms.push();
            {
                placeThigh(ms);
                renderPart(ms, C_.leg);
            }
            ms.pop();
            ms.push();
            {
                placeShin(ms, knee);
                renderPart(ms, C_.leg);
            }
            ms.pop();
        }
        ms.pop();
    }
    ms.pop(); // retour au monde
}
