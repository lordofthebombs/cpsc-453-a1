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


struct State {
	int segments = 4;
	bool operator == (State const& other) const {
		return segments == other.segments;
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


// Colors
glm::vec3 const RED = glm::vec3(1.f, 0.f, 0.f);
glm::vec3 const GREEN = glm::vec3(0.f, 1.f, 0.f);
glm::vec3 const BLUE = glm::vec3(0.f, 0.f, 1.f);
glm::vec3 const BLACK = glm::vec3(0.f, 0.f, 0.f);

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
void generateSnowflake(CPU_Geometry& snowflake, std::vector<float> startingPoint, std::vector<float> endingPoint, int iterations) {
	float firstPointAlpha = 1.f / 3.f;
	float lastPointAlpha = 2.f / 3.f;
	float endPointAlpha = 1.f;
	if (iterations > 0) {
		std::vector<float> firstPoint = pointOnLine(firstPointAlpha, startingPoint, endingPoint);
		std::vector<float> lastPoint = pointOnLine(lastPointAlpha, startingPoint, endingPoint);
		std::vector<float> firstLastVector = getVectorFromPoints(firstPoint, lastPoint);

		glm::mat4 rotationMat(1); // Creates identity matrix

		// Rotation referenced here https://stackoverflow.com/questions/20923232/how-to-rotate-a-vector-by-a-given-direction
		rotationMat = glm::rotate(rotationMat, 60.0f, glm::vec3(firstLastVector[0], firstLastVector[1], 0.f));
		glm::vec3 rotatedVector = glm::vec3(rotationMat * glm::vec4(glm::vec3(firstLastVector[0], firstLastVector[1], 0.f), 1.0));

		std::vector<float> middle = getPointFromVector(firstPoint, rotatedVector);


		generateSnowflake(snowflake, startingPoint, firstPoint, iterations - 1);
		generateSnowflake(snowflake, firstPoint, middle, iterations - 1);
		generateSnowflake(snowflake, middle, lastPoint, iterations - 1);
		generateSnowflake(snowflake, lastPoint, endingPoint, iterations - 1);
	}
	else {
		snowflake.verts.push_back(point(startingPoint));
		snowflake.verts.push_back(point(endingPoint));
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
	CPU_Geometry snowflake;
	GPU_Geometry snowflakeGPU;
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

	// Initial points for Koch Snowflake
	std::vector<float> kochStart{ 0.5, 0.f };
	std::vector<float> kochEnd{ -0.5, 0.f };


	// Serpinsky triangle generation
	generateSerpinsky(first, second, third, triangles, 0);
	serpinskyAllColored(triangles);

	// Square Diamond generation, works best with GL_LINE_STRIP
	generateSquareDiamond(squareDiamond, 3, squareDiamondPoints);
	//colorAllVerts(squareDiamond, GREEN);

	// Koch Snowflake generation
	generateSnowflake(snowflake, kochStart, kochEnd, 1);
	serpinskyAllColored(snowflake);

	// Uploads data to GPU
	trianglesGPU.setVerts(triangles.verts);
	trianglesGPU.setCols(triangles.cols);

	snowflakeGPU.setVerts(snowflake.verts);
	snowflakeGPU.setCols(snowflake.cols);

	squareDiamondGPU.setVerts(squareDiamond.verts);
	squareDiamondGPU.setCols(squareDiamond.cols);

	snowflakeGPU.setVerts(snowflake.verts);
	snowflakeGPU.setCols(snowflake.cols);



	State state;
	// RENDER LOOP
	while (!window.shouldClose()) {
		glfwPollEvents();

		glEnable(GL_FRAMEBUFFER_SRGB);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		shader.use();
		snowflakeGPU.bind();
		glDrawArrays(GL_LINE_STRIP, 0, GLsizei(snowflake.verts.size()));
		


		glDisable(GL_FRAMEBUFFER_SRGB); // disable sRGB for things like imgui

		window.swapBuffers();
	}

	glfwTerminate();
	return 0;
}
