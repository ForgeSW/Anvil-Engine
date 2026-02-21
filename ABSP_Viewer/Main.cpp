#include "AnvilBSPFormat.h"
#include <glad/glad.h>
#include <glfw/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <fstream>

glm::vec3 cPos(0, 10, 30), cFront(0, 0, -1);
float yaw = -90.f, pitch = 0.f, dt = 0, lastF = 0;

void mouse_cb(GLFWwindow* w, double x, double y) {
    static float lx = 640, ly = 360, f = true;
    if (f) { lx = x; ly = y; f = false; }
    yaw += (x - lx) * 0.1f; pitch += (ly - y) * 0.1f;
    lx = x; ly = y;
    if (pitch > 89.f) pitch = 89.f; if (pitch < -89.f) pitch = -89.f;
    cFront = glm::normalize(glm::vec3(cos(glm::radians(yaw)) * cos(glm::radians(pitch)), sin(glm::radians(pitch)), sin(glm::radians(yaw)) * cos(glm::radians(pitch))));
}
// you're too lazy to use actual files
const char* vS = "#version 440 core \nlayout(location=0) in vec3 p; layout(location=2) in vec3 n; out vec3 N; uniform mat4 pj,v; void main(){ N=n; gl_Position=pj*v*vec4(p,1.0); }";
const char* fS = "#version 440 core\nin vec3 N; out vec4 C; void main(){ vec3 light = normalize(vec3(0.5, 1.0, 0.3)); float d = max(dot(normalize(N), light), 0.2); C=vec4(vec3(0.7, 0.65, 0.6) * d, 1.0); }";

int main() {
    glfwInit();
    GLFWwindow* W = glfwCreateWindow(1280, 720, "ANVIL VIEWER", 0, 0);
    glfwMakeContextCurrent(W); gladLoadGL(); glEnable(GL_DEPTH_TEST);
    glfwSetInputMode(W, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(W, mouse_cb);

    std::ifstream is("world.absp", std::ios::binary);
    ABSPHeader h; is.read((char*)&h, sizeof(h));
    std::vector<AVertex> vr(h.numVertices); std::vector<AFace> fr(h.numFaces);
    is.read((char*)vr.data(), h.numVertices * sizeof(AVertex));
    is.read((char*)fr.data(), h.numFaces * sizeof(AFace));
    std::cout << "Rendering " << fr.size() << " faces." << std::endl;

    auto vs = glCreateShader(GL_VERTEX_SHADER); glShaderSource(vs, 1, &vS, 0); glCompileShader(vs);
    auto fs = glCreateShader(GL_FRAGMENT_SHADER); glShaderSource(fs, 1, &fS, 0); glCompileShader(fs);
    auto p = glCreateProgram(); glAttachShader(p, vs); glAttachShader(p, fs); glLinkProgram(p);

    uint32_t VAO, VBO; glGenVertexArrays(1, &VAO); glGenBuffers(1, &VBO);
    glBindVertexArray(VAO); glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vr.size() * sizeof(AVertex), vr.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, 0, sizeof(AVertex), (void*)0); glEnableVertexAttribArray(0);
    glVertexAttribPointer(2, 3, GL_FLOAT, 0, sizeof(AVertex), (void*)(5 * sizeof(float))); glEnableVertexAttribArray(2);

    while (!glfwWindowShouldClose(W)) {
        float cf = glfwGetTime(); dt = cf - lastF; lastF = cf;
        if (glfwGetKey(W, GLFW_KEY_W)) cPos += (15.0f * dt) * cFront;
        if (glfwGetKey(W, GLFW_KEY_S)) cPos -= (15.0f * dt) * cFront;

        glClearColor(0.2f, 0.3f, 0.4f, 1.0f); glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(p);
        glUniformMatrix4fv(glGetUniformLocation(p, "pj"), 1, 0, glm::value_ptr(glm::perspective(45.f, 1.77f, 0.1f, 1000.f)));
        glUniformMatrix4fv(glGetUniformLocation(p, "v"), 1, 0, glm::value_ptr(glm::lookAt(cPos, cPos + cFront, { 0,1,0 })));
        glBindVertexArray(VAO);
        for (auto& f : fr) glDrawArrays(GL_TRIANGLE_FAN, f.firstVertex, f.numVertices);
        glfwSwapBuffers(W); glfwPollEvents();
    }
    return 0;
}