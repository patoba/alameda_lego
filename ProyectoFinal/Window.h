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
	bool getLucesPrendidasQuiosko() { return luces_quiosko_prendidas; };
	bool getFiesta() { return fiesta; };
	bool getCaminando() { return caminando; };

	bool getShouldClose() {
		return  glfwWindowShouldClose(mainWindow);
	}
	bool* getsKeys() { return keys; }
	void swapBuffers() { return glfwSwapBuffers(mainWindow); }
	int getCamera() { return camara; };
	int getEstadoManzana() { return estado_manzana; };
	void terminarCaminar() { caminando = false; };
	void siguienteEstadoManzana() { estado_manzana = estado_manzana + 1; if (estado_manzana == 3) estado_manzana = 0; }
	~Window();
private:
	GLFWwindow* mainWindow;
	GLint width, height;
	bool keys[1024];
	GLint bufferWidth, bufferHeight;
	void createCallbacks();
	GLfloat lastX;
	GLfloat lastY;
	GLfloat xChange;
	GLfloat yChange;
	int camara;
	bool luces_quiosko_prendidas;
	bool fiesta;
	bool caminando;
	bool mouseFirstMoved;
	int estado_manzana;
	static void ManejaTeclado(GLFWwindow* window, int key, int code, int action, int mode);
	static void ManejaMouse(GLFWwindow* window, double xPos, double yPos);
};

