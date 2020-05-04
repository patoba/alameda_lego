#define STB_IMAGE_IMPLEMENTATION

#include <stdio.h>
#include <string.h>
#include <cmath>
#include <vector>
#include <math.h>
#include <glew.h>
#include <glfw3.h>

#include <glm.hpp>
#include <gtc\matrix_transform.hpp>
#include <gtc\type_ptr.hpp>
//para probar el importer
//#include<assimp/Importer.hpp>

#include "Window.h"
#include "Mesh.h"
#include "Shader_light.h"
#include "Camera.h"
#include "Texture.h"
//para iluminación
#include "CommonValues.h"
#include "DirectionalLight.h"
#include "PointLight.h"
#include "Material.h"

#include "Model.h"
#include "Skybox.h"
#include "SpotLight.h"

const float toRadians = 3.14159265f / 180.0f;

Window mainWindow;
std::vector<Mesh*> meshList;
std::vector<Shader> shaderList;
Camera camera;

Model alameda;

//luz direccional
DirectionalLight mainLight;

//para declarar varias luces de tipo pointlight
PointLight pointLights[MAX_POINT_LIGHTS];  //luminarias
SpotLight spotLights[MAX_SPOT_LIGHTS];  //quiosco

Skybox skybox;

GLfloat deltaTime = 0.0f;
GLfloat lastTime = 0.0f;

// Vertex Shader
static const char* vShader = "shaders/shader_light.vert";

// Fragment Shader
static const char* fShader = "shaders/shader_light.frag";
//cálculo del promedio de las normales para sombreado de Phong
void calcAverageNormals(unsigned int * indices, unsigned int indiceCount, GLfloat * vertices, unsigned int verticeCount, 
						unsigned int vLength, unsigned int normalOffset)
{
	for (size_t i = 0; i < indiceCount; i += 3)
	{
		unsigned int in0 = indices[i] * vLength;
		unsigned int in1 = indices[i + 1] * vLength;
		unsigned int in2 = indices[i + 2] * vLength;
		glm::vec3 v1(vertices[in1] - vertices[in0], vertices[in1 + 1] - vertices[in0 + 1], vertices[in1 + 2] - vertices[in0 + 2]);
		glm::vec3 v2(vertices[in2] - vertices[in0], vertices[in2 + 1] - vertices[in0 + 1], vertices[in2 + 2] - vertices[in0 + 2]);
		glm::vec3 normal = glm::cross(v1, v2);
		normal = glm::normalize(normal);
		
		in0 += normalOffset; in1 += normalOffset; in2 += normalOffset;
		vertices[in0] += normal.x; vertices[in0 + 1] += normal.y; vertices[in0 + 2] += normal.z;
		vertices[in1] += normal.x; vertices[in1 + 1] += normal.y; vertices[in1 + 2] += normal.z;
		vertices[in2] += normal.x; vertices[in2 + 1] += normal.y; vertices[in2 + 2] += normal.z;
	}

	for (size_t i = 0; i < verticeCount / vLength; i++)
	{
		unsigned int nOffset = i * vLength + normalOffset;
		glm::vec3 vec(vertices[nOffset], vertices[nOffset + 1], vertices[nOffset + 2]);
		vec = glm::normalize(vec);
		vertices[nOffset] = vec.x; vertices[nOffset + 1] = vec.y; vertices[nOffset + 2] = vec.z;
	}
}


void CreateShaders()
{
	Shader *shader1 = new Shader();
	shader1->CreateFromFiles(vShader, fShader);
	shaderList.push_back(*shader1);
}

int main() 
{
	mainWindow = Window(800, 600); 
	mainWindow.Initialise();

	CreateShaders();

	camera = Camera(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), -60.0f, 0.0f, 5.0f, 0.5f);

	//luz direccional, sólo 1 y siempre debe de existir
	mainLight = DirectionalLight(1.0f, 1.0f, 1.0f, 
								0.3f, 0.3f,
								0.0f, 0.0f, -1.0f);
	
	//Luminarias (6)
	unsigned int pointLightCount = 0;
	pointLights[0] = PointLight(1.0f, 1.0f, 1.0f,  //Blancas
		0.0f, 1.0f,
		2.0f, 1.5f, 1.5f,//Posicion
		0.3f, 0.2f, 0.1f);
	pointLights[1] = PointLight(1.0f, 1.0f, 1.0f,  //Blancas
		0.0f, 1.0f,
		2.0f, 1.5f, 1.5f,//Posicion
		0.3f, 0.2f, 0.1f);
	pointLights[2] = PointLight(1.0f, 1.0f, 1.0f,  //Blancas
		0.0f, 1.0f,
		2.0f, 1.5f, 1.5f,//Posicion
		0.3f, 0.2f, 0.1f);
	pointLights[3] = PointLight(1.0f, 1.0f, 1.0f,  //Blancas
		0.0f, 1.0f,
		2.0f, 1.5f, 1.5f,//Posicion
		0.3f, 0.2f, 0.1f);
	pointLights[4] = PointLight(1.0f, 1.0f, 1.0f,  //Blancas
		0.0f, 1.0f,
		2.0f, 1.5f, 1.5f,//Posicion
		0.3f, 0.2f, 0.1f);
	pointLights[5] = PointLight(1.0f, 1.0f, 1.0f,  //Blancas
		0.0f, 1.0f,
		2.0f, 1.5f, 1.5f,  //Posicion
		0.3f, 0.2f, 0.1f);

	//Quiosko (3)  
	unsigned int spotLightCount = 0;
	spotLights[0] = SpotLight(1.0f, .0f, 1.0f,
		0.0f, 2.0f,
		0.0f, 0.0f, 0.0f,  // Posicion
		0.0f, -1.0f, 0.0f,  //Vector Direccion
		1.0f, 0.0f, 0.0f,
		20.0f);

	spotLights[1] = SpotLight(1.0f, .0f, .0f,
		0.0f, 2.0f,
		0.0f, -1.5f, 0.f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		10.0f); 
	spotLights[2] = SpotLight(.0f, .0f, 1.0f,
		0.0f, 2.0f,
		0.0f, -1.5f, 0.f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		10.0f);

	//skyBox

	std::vector<std::string> skyboxFaces;
	skyboxFaces.push_back("Textures/Skybox/dia1/rt.tga");//rt
	skyboxFaces.push_back("Textures/Skybox/dia1/lf.tga");//lf
	skyboxFaces.push_back("Textures/Skybox/dia1/dn.tga");//dn
	skyboxFaces.push_back("Textures/Skybox/dia1/up.tga");//up
	skyboxFaces.push_back("Textures/Skybox/dia1/lf.tga");//bk
	skyboxFaces.push_back("Textures/Skybox/dia1/lf.tga");//ft
	Skybox dia1 = Skybox(skyboxFaces);

	skyboxFaces.clear();
	skyboxFaces.push_back("Textures/Skybox/dia2/lf.tga");//rt
	skyboxFaces.push_back("Textures/Skybox/dia2/lf.tga");//lf
	skyboxFaces.push_back("Textures/Skybox/dia2/dn.tga");//dn
	skyboxFaces.push_back("Textures/Skybox/dia2/up.tga");//up
	skyboxFaces.push_back("Textures/Skybox/dia2/lf.tga");//bk
	skyboxFaces.push_back("Textures/Skybox/dia2/lf.tga");//ft
	Skybox dia2 = Skybox(skyboxFaces);

	skyboxFaces.clear();
	skyboxFaces.push_back("Textures/Skybox/dia3/rt.tga");//rt
	skyboxFaces.push_back("Textures/Skybox/dia3/lf.tga");//lf
	skyboxFaces.push_back("Textures/Skybox/dia3/dn.tga");//dn
	skyboxFaces.push_back("Textures/Skybox/dia3/up.tga");//up
	skyboxFaces.push_back("Textures/Skybox/dia3/rt.tga");//bk
	skyboxFaces.push_back("Textures/Skybox/dia3/rt.tga");//ft
	Skybox dia3 = Skybox(skyboxFaces);

	skyboxFaces.clear();
	skyboxFaces.push_back("Textures/Skybox/noche/rt.tga");//rt
	skyboxFaces.push_back("Textures/Skybox/noche/lf.tga");//lf
	skyboxFaces.push_back("Textures/Skybox/noche/dn.tga");//dn
	skyboxFaces.push_back("Textures/Skybox/noche/up.tga");//up
	skyboxFaces.push_back("Textures/Skybox/noche/lf.tga");//bk
	skyboxFaces.push_back("Textures/Skybox/noche/lf.tga");//ft
	Skybox noche = Skybox(skyboxFaces);


	//modelos

	alameda = Model();
	alameda.LoadModel("Model/prueba.obj");

	//Materiales

	Material Material_brillante = Material(4.0f, 256);

	GLuint uniformProjection = 0, uniformModel = 0, uniformView = 0, uniformEyePosition = 0,
		uniformSpecularIntensity = 0, uniformShininess = 0;
	glm::mat4 projection = glm::perspective(45.0f, (GLfloat)mainWindow.getBufferWidth() / mainWindow.getBufferHeight(), 0.1f, 300.0f);
	

	//tiempo
	int tiempo = 0.f;
	float f = 1500.f;

	//animacion barco
	float barco_x = 11, barco_y = 0, barco_z = 4;  // condiciones inciciales del barco
	float angulo = -90, ang_temp = 0;
	const float f_barco = 1000.f;
	const float movx = 2.f * barco_x / f;
	const float movz = 2.f * barco_z / f;
	while (!mainWindow.getShouldClose())
	{
		//configuracion dia - noche
		int intervalo = (int)(tiempo / f) % 6;
		if (intervalo == 0) {
			skybox = dia1;
			pointLightCount = 0;
		}
		else if (intervalo == 1) {
			skybox = dia2;
		}
		else if (intervalo == 2) {
			skybox = dia3;
		}
		else {  //noche
			skybox = noche;
			pointLightCount = MAX_POINT_LIGHTS;
		}

		if (mainWindow.getLucesPrendidasQuiosko()) {
			spotLightCount = 0;
		}
		else {
			if ((int)(tiempo / 200) % 2)  // cambiar colores
				spotLightCount = MAX_SPOT_LIGHTS;
			else
				spotLightCount = 0;
		}

		//Barco
		int inter = (int)(tiempo / f_barco);
		int estado = inter % 4;
		if (estado == 0) {
			barco_x -= movx;//pos
		}
		else if (estado == 1) {//neg
			barco_z -= movz;
		}
		else if (estado == 2) {//pos
			barco_x += movx;//pos
		}
		else {//neg
			barco_z += movz;
		}
		if ((int)tiempo % (int)f_barco >= .75f * f_barco) {
			angulo += 90.f / (f_barco / 4);
		}

		//Recibir eventos del usuario
		glfwPollEvents();

		camera.keyControl(mainWindow.getsKeys(), deltaTime);
		camera.mouseControl(mainWindow.getXChange(), mainWindow.getYChange());
		// Clear the window
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 

		skybox.DrawSkybox(camera.calculateViewMatrix(), projection);
		shaderList[0].UseShader();
		uniformModel = shaderList[0].GetModelLocation();
		uniformProjection = shaderList[0].GetProjectionLocation();
		uniformView = shaderList[0].GetViewLocation();
		uniformEyePosition = shaderList[0].GetEyePositionLocation();
		uniformSpecularIntensity = shaderList[0].GetSpecularIntensityLocation();
		uniformShininess = shaderList[0].GetShininessLocation();

		glm::vec3 lowerLight = camera.getCameraPosition();
		lowerLight.y -= 0.3f;
		spotLights[0].SetFlash(lowerLight, camera.getCameraDirection());

		shaderList[0].SetDirectionalLight(&mainLight);
		shaderList[0].SetPointLights(pointLights, pointLightCount);
		shaderList[0].SetSpotLights(spotLights, spotLightCount);
		glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(camera.calculateViewMatrix()));
		glUniform3f(uniformEyePosition, camera.getCameraPosition().x, camera.getCameraPosition().y, camera.getCameraPosition().z);

		glm::mat4 model(1.0);
		glm::mat4 aux(1.0);

		//aqui se insertan Modelos
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(0.f, -10.f, 0.f));
		model = glm::scale(model, glm::vec3(1000.f, 1000.f, 1000.f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
		alameda.RenderModel();

		mainWindow.swapBuffers();

		tiempo++;
	}

	return 0;
}