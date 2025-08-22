#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "character.hpp"
#include "gpu.hpp"
#include "helper.hpp"
#include "cube.hpp"
#include "shader_utils.hpp"


static void framebuffer_size_callback(GLFWwindow*, int w, int h) {
    glViewport(0,0,w,h);
}

int main() {
    
    if (!glfwInit()) { std::cerr << "GLFW init failed\n"; return 1; }
    // macOS: OpenGL 4.1 core
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* win = glfwCreateWindow(800, 600, "HumanGL", nullptr, nullptr);
    if (!win) { std::cerr << "Window failed\n"; glfwTerminate(); return 1; }
    glfwMakeContextCurrent(win);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "GLAD load failed\n"; return 1;
    }
    glfwSetFramebufferSizeCallback(win, framebuffer_size_callback);
    glEnable(GL_DEPTH_TEST);

    // Shaders
    std::string vsSrc = loadFile("shaders/simple.vert");
    std::string fsSrc = loadFile("shaders/simple.frag");

GLuint vs = compileShader(GL_VERTEX_SHADER, vsSrc.c_str());
GLuint fs = compileShader(GL_FRAGMENT_SHADER, fsSrc.c_str());
GLuint prog = linkProgram(vs, fs);
    glUseProgram(prog);
    GLint uModel = glGetUniformLocation(prog, "model");

    GLint uView  = glGetUniformLocation(prog, "view");
    GLint uProj  = glGetUniformLocation(prog, "projection");
    GLint uColor = glGetUniformLocation(prog, "uColor");


    // Matrices cam/proj
    glm::mat4 proj = glm::perspective(glm::radians(60.f), 800.f/600.f, 0.1f, 100.f);
    glm::mat4 view = glm::lookAt(glm::vec3(2.5f,2.0f,4.0f), glm::vec3(0,0,0), glm::vec3(0,1,0));
    glUniformMatrix4fv(uProj, 1, GL_FALSE, &proj[0][0]);
    glUniformMatrix4fv(uView, 1, GL_FALSE, &view[0][0]);

    GLuint cubeVAO = make_unit_cube();

    RigParams params; // tu peux modifier les tailles à l’oral
    RigColors colors;
    CharacterRenderer renderer(uModel, uColor, cubeVAO);
renderer.setRig(params);
renderer.setColors(colors);
    AnimMode mode = AnimMode::Walk;
bool paused = false;

bool prev1=false, prev2=false, prev3=false, prevSpace=false, prevQ=false, prevW=false, prevA=false, prevS=false, prevZ=false, prevX=false;


    while (!glfwWindowShouldClose(win)) {
        // --- Input simple (polling) ---
bool k1 = glfwGetKey(win, GLFW_KEY_1) == GLFW_PRESS;
bool k2 = glfwGetKey(win, GLFW_KEY_2) == GLFW_PRESS;
bool k3 = glfwGetKey(win, GLFW_KEY_3) == GLFW_PRESS;
bool kSpace = glfwGetKey(win, GLFW_KEY_SPACE) == GLFW_PRESS;

// toggle mode with edge detection
if (k1 && !prev1) mode = AnimMode::Idle;
if (k2 && !prev2) mode = AnimMode::Walk;
if (k3 && !prev3) mode = AnimMode::Jump;
if (kSpace && !prevSpace) paused = !paused;

prev1 = k1; prev2 = k2; prev3 = k3; prevSpace = kSpace;

// --- Ajuster les dimensions à la volée ---
// Q/W : longueur bras ; A/S : longueur jambes ; Z/X : épaisseur membres
bool kQ = glfwGetKey(win, GLFW_KEY_Q) == GLFW_PRESS;
bool kW = glfwGetKey(win, GLFW_KEY_W) == GLFW_PRESS;
bool kA = glfwGetKey(win, GLFW_KEY_A) == GLFW_PRESS;
bool kS = glfwGetKey(win, GLFW_KEY_S) == GLFW_PRESS;
bool kZ = glfwGetKey(win, GLFW_KEY_Z) == GLFW_PRESS;
bool kX = glfwGetKey(win, GLFW_KEY_X) == GLFW_PRESS;

auto clamp = [](float v, float lo, float hi){ return std::max(lo, std::min(hi, v)); };

if (kQ && !prevQ) { params.upperArmL = clamp(params.upperArmL + 0.05f, 0.2f, 2.0f);
                    params.foreArmL  = clamp(params.foreArmL  + 0.05f, 0.2f, 2.0f); }
if (kW && !prevW) { params.upperArmL = clamp(params.upperArmL - 0.05f, 0.2f, 2.0f);
                    params.foreArmL  = clamp(params.foreArmL  - 0.05f, 0.2f, 2.0f); }

if (kA && !prevA) { params.thighL = clamp(params.thighL + 0.05f, 0.2f, 2.0f);
                    params.shinL  = clamp(params.shinL  + 0.05f, 0.2f, 2.0f); }
if (kS && !prevS) { params.thighL = clamp(params.thighL - 0.05f, 0.2f, 2.0f);
                    params.shinL  = clamp(params.shinL  - 0.05f, 0.2f, 2.0f); }

if (kZ && !prevZ) { params.armR = clamp(params.armR + 0.02f, 0.05f, 0.6f);
                    params.legR = clamp(params.legR + 0.02f, 0.05f, 0.6f); }
if (kX && !prevX) { params.armR = clamp(params.armR - 0.02f, 0.05f, 0.6f);
                    params.legR = clamp(params.legR - 0.02f, 0.05f, 0.6f); }

prevQ=kQ; prevW=kW; prevA=kA; prevS=kS; prevZ=kZ; prevX=kX;

        glClearColor(0.08f, 0.09f, 0.11f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float t = (float)glfwGetTime();

        // dessiner le personnage articulé
            renderer.setRig(params);   // si tu as modifié params via Q/W/A/S/Z/X
renderer.draw(t, mode, paused);



        glfwSwapBuffers(win);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;

}
