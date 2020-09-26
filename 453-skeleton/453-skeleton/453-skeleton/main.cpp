#define _USE_MATH_DEFINES
#include <cmath>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <vector>


#include <iostream>

#include "Geometry.h"
#include "GLDebug.h"
#include "Log.h"
#include "ShaderProgram.h"
#include "Shader.h"
#include "Window.h"


struct State {
	int segments = 4;
	bool operator == (State const& other) const {
		return segments == other.segments;
	}
};

float randomFloat() {
	float r = ((float) rand() / (RAND_MAX));
	return r;
}

// EXAMPLE CALLBACKS
class MyCallbacks : public CallbackInterface {

public:
	MyCallbacks(ShaderProgram& shader) : shader(shader) {}

	virtual void keyCallback(int key, int scancode, int action, int mods) {
		if (action == GLFW_PRESS || action == GLFW_REPEAT) {
			if (key == GLFW_KEY_R) {
				shader.recompile();
			}
			if (key == GLFW_KEY_LEFT) {
				if (state.segments > 0) {
					state.segments--;
				}
			}
			if (key == GLFW_KEY_RIGHT) {
				if (state.segments < 1024) {
					state.segments++;
				}
			}
		}
	}
	State getState() {
		return state;
	}

private:
	State state;
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

// Gets midpoint of two vertices
std::vector<float> midPoint(std::vector<float> vertex1, std::vector<float> vertex2) {
	std::vector<float> point;
	for (int i = 0; i < 2; i++) point.push_back((vertex1[i] * 0.5) + (vertex2[i] * 0.5));
	return point;
}

void generateSerpinsky(std::vector<float> a, std::vector<float> b, std::vector<float> c, CPU_Geometry& triangle, int iterations) {
	if (iterations > 0) {
		std::vector<float> d = midPoint(a, b);
		std::vector<float> e = midPoint(a, c);
		std::vector<float> f = midPoint(b, c);
		generateSerpinsky(a, d, e, triangle, iterations - 1);
		generateSerpinsky(d, b, f, triangle, iterations - 1);
		generateSerpinsky(e, f, c, triangle, iterations - 1);
	}
	else {
		triangle.verts.push_back(glm::vec3(a[0], a[1], 0.f));
		triangle.verts.push_back(glm::vec3(b[0], b[1], 0.f));
		triangle.verts.push_back(glm::vec3(c[0], c[1], 0.f));
	}
}

void serpinskyAllColored(CPU_Geometry& triangle) {
	for (int vert = 0; vert < triangle.verts.size(); vert++) triangle.cols.push_back(glm::vec3(randomFloat(), randomFloat(), randomFloat()));
}

void generateSun(CPU_Geometry& triangles, CPU_Geometry& lines, float segmentNumber) {
	float step = (2 * M_PI) / segmentNumber;
	float angle = 0.0;
	for (int i = 0; i < segmentNumber; ++i) {
		glm::vec3 first = pointOnCircle(angle, 0.5);
		glm::vec3 second = pointOnCircle(angle + step, 0.5);
		glm::vec3 middle = pointOnCircle(angle + (0.5 * step), 0.75);
		glm::vec3 middle2 = pointOnCircle(angle + (0.5 * step), 0.95);

		triangles.verts.push_back(first);
		triangles.verts.push_back(middle);
		triangles.verts.push_back(second);

		glm::vec3 red(1.0, 0.0, 0.0);
		triangles.cols.push_back(red);
		triangles.cols.push_back(red);
		triangles.cols.push_back(red);

		lines.verts.push_back(middle);
		lines.verts.push_back(middle2);
		triangles.cols.push_back(red);
		triangles.cols.push_back(red);

		angle += step;
	}
}

void generateCircle(CPU_Geometry& cpuGeom, float segmentNumber) {
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
	auto callbacks = std::make_shared<MyCallbacks>(shader);
	window.setCallbacks(callbacks); // can also update callbacks to new ones

	// GEOMETRY
	CPU_Geometry triangles;
	GPU_Geometry trianglesGPU;

	// Initial triangle points for serpinsky triangle
	std::vector<float> second{ -0.5, -0.5 };
	std::vector<float> third{ 0.5, -0.5 };
	std::vector<float> first{ 0.0, 0.5 };



	// Serpinsky triangle generation
	generateSerpinsky(first, second, third, triangles, 6);
	serpinskyAllColored(triangles);

	// Uploads data to GPU
	trianglesGPU.setVerts(triangles.verts);
	trianglesGPU.setCols(triangles.cols);


	State state;
	// RENDER LOOP
	while (!window.shouldClose()) {
		glfwPollEvents();

		glEnable(GL_FRAMEBUFFER_SRGB);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		shader.use();
		trianglesGPU.bind();
		glDrawArrays(GL_TRIANGLES, 0, GLsizei(triangles.verts.size()));


		glDisable(GL_FRAMEBUFFER_SRGB); // disable sRGB for things like imgui

		window.swapBuffers();
	}

	glfwTerminate();
	return 0;
}
