#define _USE_MATH_DEFINES
#include <cmath>
#include <GL/glew.h>
#include <GLFW/glfw3.h>


#include <iostream>

#include "Geometry.h"
#include "GLDebug.h"
#include "Log.h"
#include "ShaderProgram.h"
#include "Shader.h"
#include "Window.h"






// EXAMPLE CALLBACKS
class MyCallbacks : public CallbackInterface {

public:
	MyCallbacks(ShaderProgram& shader) : shader(shader) {}

	virtual void keyCallback(int key, int scancode, int action, int mods) {
		if (key == GLFW_KEY_R && action == GLFW_PRESS) {
			shader.recompile();
		}
	}

private:
	ShaderProgram& shader;
};


// END EXAMPLES

void generateTriangle(CPU_Geometry &cpuGeom, float xOffset) {
	cpuGeom.verts.push_back(glm::vec3(-0.5f + xOffset, -0.5f, 0.f));
	cpuGeom.verts.push_back(glm::vec3(0.5f + xOffset, -0.5f, 0.f));
	cpuGeom.verts.push_back(glm::vec3(0.f + xOffset, 0.5f, 0.f));

	cpuGeom.cols.push_back(glm::vec3(1.f, 0.f, 0.f));
	cpuGeom.cols.push_back(glm::vec3(0.f, 1.f, 0.f));
	cpuGeom.cols.push_back(glm::vec3(0.f, 0.f, 1.f));
}

glm::vec3 pointOnCircle(float angle, float radius) {
	return glm::vec3(
		radius * cos(angle),
		radius * sin(angle),
		0.0
	);
}

void generateCircle(CPU_Geometry &cpuGeom, float segmentNumber) {
	float step = (2 * M_PI) / segmentNumber;
	float angle = 0.0;
	for (int i = 0; i < segmentNumber; ++i) {
		cpuGeom.verts.push_back(pointOnCircle(angle, 1.0));
		cpuGeom.cols.push_back(glm::vec3(1.0, 0.0, 0.0));
		angle += step;
	}
}

int main() {
	Log::debug("Starting main");

	// WINDOW
	glfwInit();
	Window window(800, 800, "CPSC 453"); // can set callbacks at construction if desired

	GLDebug::enable();

	// SHADERS
	ShaderProgram shader("shaders/test.vert", "shaders/test.frag");

	// CALLBACKS
	window.setCallbacks(std::make_shared<MyCallbacks>(shader)); // can also update callbacks to new ones

	// GEOMETRY
	CPU_Geometry cpuGeom;
	GPU_Geometry gpuGeom;
	CPU_Geometry cpuGeom2;
	GPU_Geometry gpuGeom2;
	// Triangles
	generateCircle(cpuGeom, 100);
	generateTriangle(cpuGeom2, 0.25f);

	// Uploads data to GPU
	gpuGeom.setVerts(cpuGeom.verts);
	gpuGeom.setCols(cpuGeom.cols);
	gpuGeom2.setVerts(cpuGeom2.verts);
	gpuGeom2.setCols(cpuGeom2.cols);

	// RENDER LOOP
	while (!window.shouldClose()) {
		glfwPollEvents();


		glEnable(GL_FRAMEBUFFER_SRGB);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		shader.use();
		gpuGeom.bind();
		glDrawArrays(GL_LINE_LOOP, 0, cpuGeom.verts.size());

		gpuGeom2.bind();
		glDrawArrays(GL_LINE_STRIP, 0, 3);
		glDisable(GL_FRAMEBUFFER_SRGB); // disable sRGB for things like imgui

		window.swapBuffers();
	}

	glfwTerminate();
	return 0;
}
