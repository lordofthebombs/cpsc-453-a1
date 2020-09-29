#define _USE_MATH_DEFINES
#define GLM_SWIZZLE
#include <math.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include "Geometry.h"
#include "GLDebug.h"
#include "Log.h"
#include "ShaderProgram.h"
#include "Shader.h"
#include "Window.h"


// Scene 1 = Serpinsky Triangle, Scene 2 = Square Diamond, Scene 3 = Koch Snowflake
struct State {
	// There is this really weird bug where it doesn't draw at the iteration it is set to and I have no clue why it does that
	int iterations = 0;
	int scene = 1;
	bool operator == (State const& other) const {
		return iterations == other.iterations;
	}
};

// Random float between 0 and 1 found at https://stackoverflow.com/questions/9878965/rand-between-0-and-1
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
				if (state.iterations > 0) {
					state.iterations--;
				}
			}
			if (key == GLFW_KEY_RIGHT) {
				if (state.iterations < 10) {
					state.iterations++;
				}
			}

			if (key == GLFW_KEY_1) {
				if (state.scene != 1) {
					state.scene = 1;
				}
			}

			if (key == GLFW_KEY_2) {
				if (state.scene != 2) {
					state.scene = 2;
				}
			}

			if (key == GLFW_KEY_3) {
				if (state.scene != 3) {
					state.scene = 3;
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


// Colors
glm::vec3 const RED = glm::vec3(1.f, 0.f, 0.f);
glm::vec3 const GREEN = glm::vec3(0.f, 1.f, 0.f);
glm::vec3 const BLUE = glm::vec3(0.f, 0.f, 1.f);
glm::vec3 const BLACK = glm::vec3(0.f, 0.f, 0.f);
glm::vec3 const YELLOW = glm::vec3(1.f, 1.f, 0.f);

// Helper function for entering points as data
glm::vec3 point(std::vector<float> point) {
	return glm::vec3(point[0], point[1], 0.f);
}

// Point rotation referenced from https://www.geeksforgeeks.org/basic-transformations-opengl/#:~:text=To%20rotate%20around%20a%20different,x%2C%20y%2C%20z).
std::vector<float> rotatePoint(std::vector<float> point, std::vector<float> pivot, float angle) {
	int x = 0;
	int y = 1;

	float angleInRadians = angle * (float)(M_PI / 180.f);
	float rotatedX = pivot[x] + (point[x] - pivot[x]) * cos(angleInRadians) - (point[y] - pivot[y]) * sin(angleInRadians);
	float rotatedY = pivot[x] + (point[x] - pivot[x]) * sin(angleInRadians) + (point[y] - pivot[y])* cos(angleInRadians);

	std::vector<float> newPoint{ rotatedX, rotatedY };
	return newPoint;
}

// Gets a point in between p and q given an alpha value
std::vector<float> pointOnLine(float alpha, std::vector<float> p, std::vector<float> q) {
	int x = 0;
	int y = 1;

	float newX = (float)(1 - alpha) * p[x] + alpha * q[x];
	float newY = (float)(1 - alpha) * p[y] + alpha * q[y];

	std::vector<float> newPoint{ newX, newY };
	return newPoint;
}

// Calculates vector made by two points
std::vector<float> getVectorFromPoints(std::vector<float> p, std::vector<float> q) {
	float firstElement = q[0] - p[0];
	float secondElement = q[1] - p[1];
	std::vector<float> vector{ firstElement, secondElement };
	return vector;
}

// Moves a point along a vector
std::vector<float> getPointFromVector(std::vector<float> point, glm::vec3 vector) {
	float newX = point[0] + (float)vector.x;
	float newY = point[1] + (float)vector.y;
	std::vector<float> newPoint{ newX, newY };
	return newPoint;
}

// Gets points for square diamond
glm::vec3 squareDiamondPoint(std::vector<float> point, float factor) {
	return glm::vec3((point[0] * factor), (point[1] * factor), 0.f);
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

void colorAllVerts(CPU_Geometry& cpuGeom, glm::vec3 color) {
	for (int vert = 0; vert < cpuGeom.verts.size(); vert++) cpuGeom.cols.push_back(color);
}

void generateSquareDiamond(CPU_Geometry& squareDiamond, int iterations, std::vector<std::vector<float>> initialPoints) {
	// An interesting observation I made is that the next 2 shapes are half the size of the first 2
	float factor = 0.5f;

	for (int i = 0; i < initialPoints.size(); i++) squareDiamond.verts.push_back(point(initialPoints[i]));

	for (int i = 0; i < iterations; i++) {
		for (int j = 0; j < initialPoints.size(); j++) squareDiamond.verts.push_back(squareDiamondPoint(initialPoints[j], factor));
		squareDiamond.cols.push_back(BLUE);
		squareDiamond.cols.push_back(BLUE);
		squareDiamond.cols.push_back(BLUE);
		squareDiamond.cols.push_back(BLUE);
		squareDiamond.cols.push_back(BLUE);
		squareDiamond.cols.push_back(RED);
		squareDiamond.cols.push_back(RED);
		squareDiamond.cols.push_back(RED);
		squareDiamond.cols.push_back(RED);
		squareDiamond.cols.push_back(RED);
		factor *= 0.5f;
	}

	
}

// Function to generate geometry for koch snowflake
void generateSnowflake(CPU_Geometry& snowflake, std::vector<float> startingPoint, std::vector<float> endingPoint, glm::vec3 color, int iterations) {
	float firstPointAlpha = 1.f / 3.f;
	float lastPointAlpha = 2.f / 3.f;
	float endPointAlpha = 1.f;
	if (iterations > 0) {
		std::vector<float> firstPoint = pointOnLine(firstPointAlpha, startingPoint, endingPoint);
		std::vector<float> lastPoint = pointOnLine(lastPointAlpha, startingPoint, endingPoint);
		std::vector<float> middle = rotatePoint(firstPoint, lastPoint, 60.0);


		generateSnowflake(snowflake, startingPoint, firstPoint, BLUE,iterations - 1);
		generateSnowflake(snowflake, firstPoint, middle, GREEN, iterations - 1);
		generateSnowflake(snowflake, middle, lastPoint, RED, iterations - 1);
		generateSnowflake(snowflake, lastPoint, endingPoint, YELLOW, iterations - 1);
	}
	else {
		snowflake.verts.push_back(point(startingPoint));
		snowflake.verts.push_back(point(endingPoint));
		snowflake.cols.push_back(color);
		snowflake.cols.push_back(color);
	}
}

void clearScene(CPU_Geometry& triangles, CPU_Geometry& squareDiamond, CPU_Geometry& snowflake1, CPU_Geometry& snowflake2, CPU_Geometry& snowflake3) {
	triangles.verts.clear();
	triangles.cols.clear();

	squareDiamond.verts.clear();
	squareDiamond.cols.clear();

	snowflake1.verts.clear();
	snowflake1.cols.clear();

	snowflake2.verts.clear();
	snowflake2.cols.clear();

	snowflake3.verts.clear();
	snowflake3.cols.clear();
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

	CPU_Geometry snowflake1;
	GPU_Geometry snowflake1GPU;
	CPU_Geometry snowflake2;
	GPU_Geometry snowflake2GPU;
	CPU_Geometry snowflake3;
	GPU_Geometry snowflake3GPU;

	CPU_Geometry squareDiamond;
	GPU_Geometry squareDiamondGPU;


	// Initial triangle points for serpinsky triangle and koch snowflake
	std::vector<float> second{ -0.5, -0.5 };
	std::vector<float> third{ 0.5, -0.5 };
	std::vector<float> first{ 0.0, 0.5 };

	// Initial points for square
	std::vector<float> point1{ 0.5, 0.5 };
	std::vector<float> point2{ -0.5, 0.5 };
	std::vector<float> point3{ -0.5, -0.5 };
	std::vector<float> point4{ 0.5, -0.5 };

	// Initial points for diamond
	std::vector<float> point5{ 0.f, 0.5 };
	std::vector<float> point6{ -0.5, 0.f };
	std::vector<float> point7{ 0.f, -0.5 };
	std::vector<float> point8{ 0.5, 0.f };

	std::vector<std::vector<float>> squareDiamondPoints{point1, point2, point3, point4, point1, point5, point6, point7, point8, point5};

	State state;

	// RENDER LOOP
	while (!window.shouldClose()) {
		glfwPollEvents();

		glEnable(GL_FRAMEBUFFER_SRGB);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		shader.use();

		if (!(state == callbacks->getState())) {
			if (callbacks->getState().scene == 1) {
				clearScene(triangles, squareDiamond, snowflake1, snowflake2, snowflake3);
				generateSerpinsky(first, second, third, triangles, callbacks->getState().iterations);
				serpinskyAllColored(triangles);
				trianglesGPU.setVerts(triangles.verts);
				trianglesGPU.setCols(triangles.cols);
				trianglesGPU.bind();
				glDrawArrays(GL_TRIANGLES, 0, GLsizei(triangles.verts.size()));
			}
			else if (callbacks->getState().scene == 2) {
				clearScene(triangles, squareDiamond, snowflake1, snowflake2, snowflake3);
				generateSquareDiamond(squareDiamond, callbacks->getState().iterations, squareDiamondPoints);
				squareDiamondGPU.setVerts(squareDiamond.verts);
				squareDiamondGPU.setCols(squareDiamond.cols);
				squareDiamondGPU.bind();
				glDrawArrays(GL_LINE_STRIP, 0, GLsizei(squareDiamond.verts.size()));
			}
			else if (callbacks->getState().scene == 3) {
				clearScene(triangles, squareDiamond, snowflake1, snowflake2, snowflake3);
				generateSnowflake(snowflake1, first, second, BLUE, callbacks->getState().iterations);
				generateSnowflake(snowflake2, second, third, BLUE, callbacks->getState().iterations);
				generateSnowflake(snowflake3, third, first, BLUE, callbacks->getState().iterations);

				snowflake1GPU.setVerts(snowflake1.verts);
				snowflake1GPU.setCols(snowflake1.cols);
				snowflake1GPU.bind();
				glDrawArrays(GL_LINE_STRIP, 0, GLsizei(snowflake1.verts.size()));

				snowflake2GPU.setVerts(snowflake2.verts);
				snowflake2GPU.setCols(snowflake2.cols);
				snowflake2GPU.bind();
				glDrawArrays(GL_LINE_STRIP, 0, GLsizei(snowflake2.verts.size()));

				snowflake3GPU.setVerts(snowflake3.verts);
				snowflake3GPU.setCols(snowflake3.cols);
				snowflake3GPU.bind();
				glDrawArrays(GL_LINE_STRIP, 0, GLsizei(snowflake3.verts.size()));
				
			}
		}

		
		

		glDisable(GL_FRAMEBUFFER_SRGB); // disable sRGB for things like imgui

		window.swapBuffers();
	}

	glfwTerminate();
	return 0;
}
