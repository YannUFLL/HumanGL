// character_refactored.cpp
#include "character.hpp"
#include "gpu.hpp"

// --- Helpers locaux : dessinent UNE partie à partir d'un pivot déjà positionné ---
// Convention : la matrice courante (ms.top()) est déjà sur le pivot parent.
// On ne refait pas les translations jusqu'au pivot : on profite de la stack.

static void draw_torso(MatrixStack& ms, GLint uModel, GLuint vao, const RigParams& P) {
    ms.push();
    ms.scale({P.torsoW, P.torsoH, P.torsoD});
    setModel(uModel, ms.top());
    glBindVertexArray(vao); glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    ms.pop();
}

static void draw_head_from_torso(MatrixStack& ms, GLint uModel, GLuint vao, const RigParams& P) {
    ms.push();
    // depuis le centre du torse → monter au haut du torse + moitié de la tête
    ms.translate({0.0f, P.torsoH*0.5f + P.headH*0.5f, 0.0f});
    ms.scale({P.headH*0.8f, P.headH, P.headH*0.8f});
    setModel(uModel, ms.top());
    glBindVertexArray(vao); glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    ms.pop();
}

static void draw_upper_arm_local(MatrixStack& ms, GLint uModel, GLuint vao, const RigParams& P) {
    ms.push();
    // Cube centré → décaler de la moitié de la longueur pour accrocher à l’épaule
    ms.translate({0.0f, -P.upperArmL*0.5f, 0.0f});
    ms.scale({P.armR, P.upperArmL, P.armR});
    setModel(uModel, ms.top());
    glBindVertexArray(vao); glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    ms.pop();
}

static void draw_forearm_local(MatrixStack& ms, GLint uModel, GLuint vao, const RigParams& P, float elbowRot) {
    ms.push();
    // depuis l’épaule : aller au coude, plier, puis centrer le segment
    ms.translate({0.0f, -P.upperArmL, 0.0f});      // coude
    ms.rotate(elbowRot, {1,0,0});                   // coude
    ms.translate({0.0f, -P.foreArmL*0.5f, 0.0f});  // centrer l’os
    ms.scale({P.armR*0.95f, P.foreArmL, P.armR*0.95f});
    setModel(uModel, ms.top());
    glBindVertexArray(vao); glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    ms.pop();
}

static void draw_thigh_local(MatrixStack& ms, GLint uModel, GLuint vao, const RigParams& P) {
    ms.push();
    // depuis la hanche : centrer la cuisse
    ms.translate({0.0f, -P.thighL*0.5f, 0.0f});
    ms.scale({P.legR, P.thighL, P.legR});
    setModel(uModel, ms.top());
    glBindVertexArray(vao); glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    ms.pop();
}

static void draw_shin_local(MatrixStack& ms, GLint uModel, GLuint vao, const RigParams& P, float kneeRot) {
    ms.push();
    // depuis la hanche : aller au genou, plier, puis centrer le tibia
    ms.translate({0.0f, -P.thighL, 0.0f});         // genou
    ms.rotate(kneeRot, {1,0,0});                    // genou
    ms.translate({0.0f, -P.shinL*0.5f, 0.0f});     // centrer l’os
    ms.scale({P.legR*0.95f, P.shinL, P.legR*0.95f});
    setModel(uModel, ms.top());
    glBindVertexArray(vao); glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    ms.pop();
}

// --- Assemblage hiérarchique : on garde les pivots avec des push/pop ---

void draw_character(GLint uModel, GLuint cubeVAO, const RigParams& P, float t,
                    AnimMode mode, bool paused) {

    MatrixStack ms;

    // tEffectif = t ou 0 si pause
    const float tt = paused ? 0.0f : t;

    float walk=0, elbow=0, hip=0, knee=0, bounce=0;

    switch (mode) {
    case AnimMode::Idle:
        walk = 0.0f;
        elbow = 0.0f;
        hip = 0.0f;
        knee = 0.0f;
        bounce = 0.0f;
        break;
    case AnimMode::Walk:
        walk   = std::sin(tt*2.0f) * 0.6f;
        elbow  = std::sin(tt*2.0f + 1.57f) * 0.4f;
        hip    =  walk;
        knee   = std::max(0.0f, -std::sin(tt*2.0f)) * 0.8f;
        bounce = std::abs(std::sin(tt*2.0f)) * 0.05f;
        break;
    case AnimMode::Jump:
        // petit saut looping: phase = sin
        {
            const float s = std::sin(tt*3.0f);
            // fléchir → extension
            hip    = std::max(0.0f, -s) * 0.5f;
            knee   = std::max(0.0f, -s) * 1.2f;   // <-- AJOUT: genoux se plient en flexion
            walk   = 0.0f;
            elbow  = 0.2f * s;          // bouger un peu les bras
            bounce = std::max(0.0f, s) * 0.25f; // montée pendant extension
        }
        break;
    }

    

    ms.rotate(0.2f*std::sin(t*0.7f), {0,1,0});

    // ---- Racine "torse" : tout le monde en hérite ----
    ms.push(); { // frame du torse (centre)
        draw_torso(ms, uModel, cubeVAO, P);

        // ---- Tête (enfant direct du torse) ----
        draw_head_from_torso(ms, uModel, cubeVAO, P);

        // ---- Bras droit : on se place UNE FOIS à l’épaule, on garde l’état ----
        ms.push(); {
            ms.translate({+(P.torsoW*0.5f + P.armR), +P.torsoH*0.35f, 0.0f}); // pivot épaule D
            ms.rotate(-walk, {1,0,0}); // rotation d'épaule (parent)
            // Bras + Avant-bras héritent du pivot/rotation d'épaule
            draw_upper_arm_local(ms, uModel, cubeVAO, P);
            draw_forearm_local (ms, uModel, cubeVAO, P, elbow);
        } ms.pop();

        // ---- Bras gauche ----
        ms.push(); {
            ms.translate({-(P.torsoW*0.5f + P.armR), +P.torsoH*0.35f, 0.0f}); // pivot épaule G
            ms.rotate(+walk, {1,0,0});
            draw_upper_arm_local(ms, uModel, cubeVAO, P);
            draw_forearm_local (ms, uModel, cubeVAO, P, elbow);
        } ms.pop();

        // ---- Jambe droite : pivot hanche D gardé le temps cuisse+tibia ----
        ms.push(); {
            ms.translate({+P.torsoW*0.25f, -P.torsoH*0.5f, 0.0f}); // pivot hanche D
            ms.rotate(+hip, {1,0,0});
            draw_thigh_local(ms, uModel, cubeVAO, P);
            draw_shin_local (ms, uModel, cubeVAO, P, knee);
        } ms.pop();

        // ---- Jambe gauche ----
        ms.push(); {
            ms.translate({-P.torsoW*0.25f, -P.torsoH*0.5f, 0.0f}); // pivot hanche G
            ms.rotate(-hip, {1,0,0});
            draw_thigh_local(ms, uModel, cubeVAO, P);
            draw_shin_local (ms, uModel, cubeVAO, P, knee);
        } ms.pop();

    } ms.pop(); // retour au monde
}
