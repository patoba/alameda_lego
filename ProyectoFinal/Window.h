#pragma once
#include<stdio.h>
#include<glew.h>
#include<glfw3.h>

class Window
{
public:
	Window();
	Window(GLint windowWidth, GLint windowHeight);
	int Initialise();
	GLfloat getBufferWidth() { return bufferWidth; }
	GLfloat getBufferHeight() { return bufferHeight; }
	GLfloat getXChange();
	GLfloat getYChange();
	GLfloat getLuzPuntual1x() { return luzPuntual1x; };
	GLfloat getLuzPuntual1y() { return luzPuntual1y; };
	GLfloat getLuzPuntual1z() { return luzPuntual1z; };
	GLfloat getLuzPuntual2x() { return luzPuntual2x; };
	GLfloat getLuzPuntual2y() { return luzPuntual2y; };
	GLfloat getLuzPuntual2z() { return luzPuntual2z; };
	GLfloat getLuzPuntual1_on() { return luzPuntual1_on; };
	GLfloat getLuzPuntual2_on() { return luzPuntual2_on; };
	GLfloat getLinternaEncendida() { return linterna_encendida; }
	GLfloat getmuevex() { return muevex; }
	bool getShouldClose() {
		return  glfwWindowShouldClose(mainWindow);}
	bool* getsKeys() { return keys; }
	void swapBuffers() { return glfwSwapBuffers(mainWindow); }
	
	~Window();
private: 
	GLFWwindow *mainWindow;
	GLint width, height;
	bool keys[1024];
	GLint bufferWidth, bufferHeight;
	void createCallbacks();
	GLfloat lastX;
	GLfloat lastY;
	GLfloat xChange;
	GLfloat yChange;
	GLfloat muevex;
	GLfloat luzPuntual1x, luzPuntual1y, luzPuntual1z;
	GLfloat luzPuntual2x, luzPuntual2y, luzPuntual2z;
	GLfloat luzPuntual1_on, luzPuntual2_on;
	GLfloat linterna_encendida;
	bool mouseFirstMoved;
	static void ManejaTeclado(GLFWwindow* window, int key, int code, int action, int mode);
	static void ManejaMouse(GLFWwindow* window, double xPos, double yPos);

};

