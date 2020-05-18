#define STB_IMAGE_IMPLEMENTATION

#include <stdio.h>
#include <cmath>
#include <vector>
#include <math.h>
#include <glew.h>
#include <glfw3.h>
#include <string>
#include <sstream>

#include <iostream>     // std::cout
#include <fstream>      // std::ifstream

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

#include"Model.h"
#include "Skybox.h"
#include"SpotLight.h"


const float toRadians = 3.14159265f / 180.0f;

Window mainWindow;
std::vector<Mesh*> meshList;
std::vector<Shader> shaderList;
Camera camera;

//luz direccional
DirectionalLight mainLight;
//para declarar varias luces de tipo pointlight
PointLight pointLights[MAX_POINT_LIGHTS];
SpotLight *spotLights;


Model alameda;
Model arbol_manzano;
Model barco;
Model banca;
Model bath;
Model ballena;
Model boteBasura;
Model fuente;
Model lampara;
Model manzana;
Model pino;
Model rejaChica;
Model rejaGrande;

Skybox skybox;

GLfloat deltaTime = 0.0f;
GLfloat lastTime = 0.0f;

// Materiales

Material Material_brillante;
Material Material_opaco;

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

//PARA KEYFRAMES
//POSICION DELFIN
glm::vec3 posicion_delfin = glm::vec3(-3.0f, 1.f, -130.f);

float reproduciranimacion, habilitaranimacion, guardoFrame, reinicioFrame, ciclo, ciclo2, contador = 0;
std::string archivo_keyframes = "datos.csv";
bool animacion = false;

//NEW// Keyframes
float movAvion_x = 0.0f, movAvion_y = 0.0f;
float giroAvion = 180;

#define MAX_FRAMES 30
int i_max_steps = 90;  //numero entre
int i_curr_steps = 29;  //guardados en memoria
typedef struct _frame
{
	//Variables para GUARDAR Key Frames
	float movAvion_x;		//Variable para PosicionX
	float movAvion_y;		//Variable para PosicionY
	float movAvion_xInc;		//Variable para IncrementoX
	float movAvion_yInc;		//Variable para IncrementoY
	float giroAvion;
	float giroAvionInc;
}FRAME;

FRAME KeyFrame[MAX_FRAMES];
int FrameIndex = i_curr_steps;			//introducir datos
bool play = false;
int playIndex = 0;


void resetElements(void)
{

	movAvion_x = KeyFrame[0].movAvion_x;
	movAvion_y = KeyFrame[0].movAvion_y;
	giroAvion = KeyFrame[0].giroAvion;
}

void interpolation(void)
{
	KeyFrame[playIndex].movAvion_xInc = (KeyFrame[playIndex + 1].movAvion_x - KeyFrame[playIndex].movAvion_x) / i_max_steps;
	KeyFrame[playIndex].movAvion_yInc = (KeyFrame[playIndex + 1].movAvion_y - KeyFrame[playIndex].movAvion_y) / i_max_steps;
	KeyFrame[playIndex].giroAvionInc = (KeyFrame[playIndex + 1].giroAvion - KeyFrame[playIndex].giroAvion) / i_max_steps;

}


void animate(void)
{
	//Movimiento del objeto
	if (play)
	{
		if (i_curr_steps >= i_max_steps) //end of animation between frames?
		{
			playIndex++;
			printf("playindex : %d\n", playIndex);
			if (playIndex > FrameIndex - 2)	//end of total animation?
			{
				printf("Frame index= %d\n", FrameIndex);
				printf("termina anim\n");
				playIndex = 0;
				play = false;
			}
			else //Next frame interpolations
			{
				//printf("entro aqui\n");
				i_curr_steps = 0; //Reset counter
				//Interpolation
				interpolation();
			}
		}
		else
		{
			//printf("se quedo aqui\n");
			//printf("max steps: %f", i_max_steps);
			//Draw animation
			movAvion_x += KeyFrame[playIndex].movAvion_xInc;
			movAvion_y += KeyFrame[playIndex].movAvion_yInc;
			giroAvion += KeyFrame[playIndex].giroAvionInc;
			i_curr_steps++;
		}

	}
}


void leer_keyframes() {

	std::ifstream lectura;
	lectura.open(archivo_keyframes, std::ifstream::in);
	int i = 0;
	for (std::string linea; std::getline(lectura, linea); i++)
	{
		std::stringstream registro(linea);
		std::string dato;

		for (int columna = 0; std::getline(registro, dato, ','); ++columna)
		{
			switch (columna)
			{
			case 0: // X
				KeyFrame[i].movAvion_x = std::stof(dato) + posicion_delfin.x;
				printf("lei x:%lf\n", KeyFrame[i].movAvion_x);
				break;
			case 1: // Y
				KeyFrame[i].movAvion_y = std::stof(dato) + posicion_delfin.y;
				printf("lei y:%lf\n", KeyFrame[i].movAvion_y);
				break;
			case 2: // angulo
				KeyFrame[i].giroAvion = std::stof(dato);
				printf("lei y:%lf\n", KeyFrame[i].giroAvion);

				break;
			}
		}
	}
	lectura.close();
	printf("archivo leido\n");
}


void inputKeyframes(bool* keys)
{
	if (keys[GLFW_KEY_C])
	{
		if (reproduciranimacion < 1)
		{
			if (play == false && (FrameIndex > 1))
			{
				printf("animacion keyframe: ballena\n");
				resetElements();
				//First Interpolation				
				interpolation();
				play = true;
				playIndex = 0;
				i_curr_steps = 0;
				reproduciranimacion++;
				printf("presiona 0 para habilitar reproducir de nuevo la animación'\n");
				habilitaranimacion = 0;

			}
			else
			{
				play = false;
			}
		}
	}
	if (keys[GLFW_KEY_G]) {
		leer_keyframes();
	}
	if (keys[GLFW_KEY_0])
	{
		if (habilitaranimacion < 1)
		{
			reproduciranimacion = 0;
		}
	}
	if (keys[GLFW_KEY_2])
	{
		if (ciclo2 < 1)
		{
			ciclo = 0;
		}
	}

}

//
//END KEYFRAMES
//

void CreateShaders()
{
	Shader *shader1 = new Shader();
	shader1->CreateFromFiles(vShader, fShader);
	shaderList.push_back(*shader1);
}

int main()
{
	mainWindow = Window(800, 600); // 1280, 1024 or 1024, 768
	//mainWindow = Window(2560, 1440);
	mainWindow.Initialise();

	CreateShaders();

	camera = Camera(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), -60.0f, 0.0f, 5.0f, 0.5f);

	//tiempo
	int tiempo = 0.f;
	float f = 1500.f;

	//barco
	float barco_x = 150, barco_y = -2.0f, barco_z = 150;  // condiciones inciciales del barco
	float angulo = -90, ang_temp = 0;
	const float f_barco = 1000.f;
	const float movx = 3.5f * barco_x / (f);
	const float movz = 3.5f * barco_z / (f);
	float escala = 1.f;
	const float altura_lego = .96f;
	const float largo_lego = .8f;


	//personaje
	glm::vec3 posicion_avatar = glm::vec3(0.f,2.f,0.f);

	//importante para acomodar objetos, luces, y animaciones
	glm::vec3 posicion_alameda = glm::vec3(0.f, 0.f, 0.f);
	glm::vec3 posicion_arbol_animacion = glm::vec3(-10 * largo_lego, 0.f, -10 * largo_lego);
	glm::vec3 posicion_quiosko = glm::vec3(4 * largo_lego, 0.f, 0.f);
	glm::vec3 posicion_lamparas[] = { glm::vec3(-50.5f, 0.0f, -47.f), glm::vec3(-15.5f, 0.0f, -47.f), glm::vec3(14.5f, 0.0f, -47.f), glm::vec3(49.5f, 0.0f, -47.f),
										glm::vec3(-41.f, 0.0f, -37.f), glm::vec3(-25.f, 0.0f, -37.f), glm::vec3(24.f, 0.0f, -37.f), glm::vec3(40.f, 0.0f, -37.f),
										glm::vec3(-50.5f, 0.0f, 46.f),glm::vec3(-15.5f, 0.0f, 46.f),glm::vec3(15.5f, 0.0f, 46.f),glm::vec3(50.5f, 0.0f, 46.f),
										glm::vec3(-41.f, 0.0f, 36.f),glm::vec3(-25.f, 0.0f, 36.f),glm::vec3(25.f, 0.0f, 36.f),glm::vec3(41.f, 0.0f, 36.f),
										glm::vec3(-66.5f, 0.0f, -38.5f),glm::vec3(-66.5f, 0.0f, 38.5f),glm::vec3(-54.5f, 0.0f, -27.f),glm::vec3(-54.5f, 0.0f, 27.f),
										glm::vec3(-54.5f, 0.0f, -9.f),glm::vec3(-54.5f, 0.0f, 9.f),glm::vec3(0.f, 0.0f, -40.5f),glm::vec3(-13.f, 0.0f, -27.f),
										glm::vec3(-13.f, 0.0f, 27.f),glm::vec3(-14.5f, 0.0f, -9.f),glm::vec3(-14.5f, 0.0f, 9.f),glm::vec3(13.f, 0.0f, -27.f),
										glm::vec3(13.f, 0.0f, 27.f),glm::vec3(14.5f, 0.0f, -9.f),glm::vec3(14.5f, 0.0f, 9.f),glm::vec3(65.5f, 0.0f, -38.5f),
										glm::vec3(65.5f, 0.0f, 38.5f),glm::vec3(0.f, 0.0f, 40.5f),glm::vec3(53.5f, 0.0f, -27.f),glm::vec3(53.5f, 0.0f, 27.f),
										glm::vec3(53.5f, 0.0f, -9.f),glm::vec3(53.5f, 0.0f, 9.f) };

	//UTILES PARA ANIMACION COMPLEJA
	glm::vec3 posicion_manzana = glm::vec3(posicion_arbol_animacion.x + 11.f, posicion_arbol_animacion.y + 6.5f * altura_lego, posicion_arbol_animacion.z + 1.5f);
	const float g = 9.8;
	const float altura_manzana_inicial = posicion_manzana.y;
	float vi;
	float tiempo1;
	const float f_manzana = 100.f;

	//Materiales
	Material_brillante = Material(4.0f, 256);
	Material_opaco = Material(0.3f, 4);

	//luz direccional, sólo 1 y siempre debe de existir
	DirectionalLight sol = mainLight = DirectionalLight(1.0f, 1.0f, 1.0f,
		0.7f, 0.7f,
		0.0f, 10.0f, -1.0f);

	DirectionalLight luna = DirectionalLight(.2f, .2f, .2f,
		0.7f, 0.7f,
		0.0f, 10.0f, -1.0f);

	//Luminarias (6)
	unsigned int pointLightCount = 0;
	for (int i = 0; i < MAX_POINT_LIGHTS; i++) {
		pointLights[i] = PointLight(1.0f, 1.0f, 1.0f,  //Blancas
			.0f, 1.0f,
			posicion_lamparas[i].x, posicion_lamparas[i].y+8.5f, posicion_lamparas[i].z,//Posicion
			0.3f, 0.2f, 0.1f);
	}

	//Quiosko (3)  
	glm::vec3 colores[] = { glm::vec3(1.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f), glm::vec3(0.f, 0.f, 1.f) };
	SpotLight luces[3][MAX_SPOT_LIGHTS];

	
	float luces_quiosko_x = -34.5f;
	float luces_quiosko_y = 7.5f;
	float luces_quiosko_z = -1.f;
	unsigned int spotLightCount = 0;
	
	for (int i = 0; i < MAX_SPOT_LIGHTS; i++) {
		int num1 = i;
		int num2 = i+1;
		if (num2 >= 3) num2 -= 3;
		int num3 = i+2;
		if (num3 >= 3) num3 -= 3;

		luces[i][0] = SpotLight(colores[num1].x, colores[num1].y, colores[num1].z,
			0.0f, 2.0f,
			luces_quiosko_x + 2.f, luces_quiosko_y, luces_quiosko_z + 1.f,
			0.0f, -1.0f, 0.0f,  //Vector Direccion
			1.0f, 0.0f, 0.0f,
			15.0f);

		luces[i][1] = SpotLight(colores[num2].x, colores[num2].y, colores[num2].z,
			0.0f, 2.0f,
			luces_quiosko_x - 2.f, luces_quiosko_y, luces_quiosko_z + 1.f,
			0.0f, -1.0f, 0.0f,  //Vector Direccion
			1.0f, 0.0f, 0.0f,
			15.0f);

		luces[i][2] = SpotLight(colores[num3].x, colores[num3].y, colores[num3].z,
			0.0f, 2.0f,
			luces_quiosko_x, luces_quiosko_y, luces_quiosko_z - 1.f,
			0.0f, -1.0f, 0.0f,  //Vector Direccion
			1.0f, 0.0f, 0.0f,
			15.0f);
	}
	spotLights = luces[0];

	//SKYBOX

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

	skybox = dia1;
	//Modelos

	alameda = Model();
	alameda.LoadModel("models/alameda.obj");

	arbol_manzano = Model();
	//arbol_manzano.LoadModel("models/arbol_manzano.obj");

	barco = Model();
	//barco.LoadModel("Models/PirateShip.obj");

	ballena = Model();
	ballena.LoadModel("models/Ballena.obj");

	banca = Model();
	banca.LoadModel("models/banca.obj");

	bath = Model();
	bath.LoadModel("models/bath.obj");

	boteBasura = Model();
	boteBasura.LoadModel("models/boteBasura.obj");

	fuente = Model();
	//fuente.LoadModel("Models/Fountain.obj");

	lampara = Model();
	lampara.LoadModel("Models/Lamp.obj");

	pino = Model();
	//pino.LoadModel("Models/Pine.obj");

	manzana = Model();
	manzana.LoadModel("models/Manzana.obj");

	rejaChica = Model();
	rejaChica.LoadModel("models/rejaChica.obj");

	rejaGrande = Model();
	rejaGrande.LoadModel("models/rejaGrande.obj");

	//Quiosko
	Model brick_1x4 = Model();
	brick_1x4.LoadModel("models/brick_1x4.dae.obj");
	Model brick_2x2 = Model();
	brick_2x2.LoadModel("models/brick_2x2.obj");
	Model brick_round_corner_4x4 = Model();
	brick_round_corner_4x4.LoadModel("models/brick_round_corner_4x4.obj");
	Model cone_1_5 = Model();
	cone_1_5.LoadModel("models/cone_1_.5.obj");
	Model cone_1x1 = Model();
	cone_1x1.LoadModel("models/cone_1x1.obj");
	Model arriba = Model();
	arriba.LoadModel("models/espada.obj");
	Model frence_1x4x1 = Model();
	frence_1x4x1.LoadModel("models/frence_1_4_1.obj");
	Model plate_1x1 = Model();
	plate_1x1.LoadModel("models/plate_1x1.obj");
	Model plate_2x2_corner = Model();
	plate_2x2_corner.LoadModel("models/plate_2x2_corner.obj");
	Model palte_2x2_modified = Model();
	palte_2x2_modified.LoadModel("models/plate_2x2_modified.obj");
	Model plate_2x4 = Model();
	plate_2x4.LoadModel("models/plate_2x4.dae.obj");
	Model plate_round_corner_6x6 = Model();
	plate_round_corner_6x6.LoadModel("models/plate_round_corner_6x6.obj");
	Model slope_18_4x2 = Model();
	slope_18_4x2.LoadModel("models/slope_18_4x2.obj");
	Model slop_inverted_45_2x2 = Model();
	slop_inverted_45_2x2.LoadModel("models/slope_inverted_45_2x2.obj");
	Model title_1x1 = Model();
	title_1x1.LoadModel("models/tile_1x1.obj");
	Model title_1x2 = Model();
	title_1x2.LoadModel("models/tile_1x2.obj");
	Model title_1x4 = Model();
	title_1x4.LoadModel("models/tile_1x4.obj");
	Model title_2x2 = Model();
	title_2x2.LoadModel("models/tile_2x2.obj");
	Model title_round_corner_2x2 = Model();
	title_round_corner_2x2.LoadModel("models/tile_round_corner_2x2.obj");
	Model title_round_corner_4x4 = Model();
	title_round_corner_4x4.LoadModel("models/tile_round_corner_4x4.obj");
	Model arco = Model();
	arco.LoadModel("models/arco.obj");
	Model plate_modified_1x4 = Model();
	plate_modified_1x4.LoadModel("models/plate_modified_1x4.obj");
	Model wedge_4x4 = Model();
	wedge_4x4.LoadModel("models/wedge_4x4.obj");
	Model plate_1x2 = Model();
	plate_1x2.LoadModel("models/plate_1x2.obj");
	Model cilindro = Model();
	cilindro.LoadModel("models/cilindro.obj");
	Model telescopio = Model();
	telescopio.LoadModel("models/telescopio.obj");
	Model title_2x2_corner = Model();
	title_2x2_corner.LoadModel("models/tile_2x2_corner.obj");
	Model plate_2x2_modified = Model();
	plate_2x2_modified.LoadModel("models/plate_2x2_modified.obj");
	//Fin Quiosko
	
	//INICIA Avatar
	Model torso_plain = Model();
	torso_plain.LoadModel("models/torso_plain.obj");

	Model arm_left = Model();
	arm_left.LoadModel("models/arm_left.obj");

	Model arm_right = Model();
	arm_right.LoadModel("models/arm_right.obj");

	Model hips = Model();
	hips.LoadModel("models/hips.obj");

	Model leg_left = Model();
	leg_left.LoadModel("models/leg_left.obj");

	Model leg_right = Model();
	leg_right.LoadModel("models/leg_right.obj");

	Model head = Model();
	head.LoadModel("models/head.obj");

	Model hat = Model();
	hat.LoadModel("models/hat.obj");

	Model hand = Model();
	hand.LoadModel("models/hand.obj");
	//TERMINA Avatar
	
	GLuint uniformProjection = 0, uniformModel = 0, uniformView = 0, uniformEyePosition = 0,
		uniformSpecularIntensity = 0, uniformShininess = 0;
	glm::mat4 projection = glm::perspective(45.0f, (GLfloat)mainWindow.getBufferWidth() / mainWindow.getBufferHeight(), 0.1f, 300.0f);

	//para keyframes
	leer_keyframes();

	//para la camara
	glm::mat4 viewMatrix;
	glm::vec3 position;
	glm::vec3 up = glm::vec3(0.f, 0.f, -1.f);
	glm::vec3 des = glm::vec3(0.f, 0.f, 0.f);

	//para la fiesta
	int tiempo_fiesta=0;
	int estado_fiesta = 0;

	//para caminar
	glm::vec3 posicion_avatar_original = posicion_avatar;
	int tiempo_caminar = 0;
	const int tiempo_maximo = 40.f;
	const float velocidad_avatar = .5f;
	float angulo_avatar = 0.f;
	bool subiendo_angulo = true;
	int caminando = 0.f;

	//Loop mientras no se cierra la ventana
	while (!mainWindow.getShouldClose())
	{
		//keyframe 
		inputKeyframes(mainWindow.getsKeys());
		animate();

		//configuracion dia - noche
		int intervalo = (int)(tiempo / f) % 6;
		if (intervalo == 0 && false) {
			skybox = dia1;
			pointLightCount = 0;
			mainLight = sol;
		}
		else if (intervalo == 1 && false) {
			skybox = dia2;
		}
		else if (intervalo == 2 && false) {
			skybox = dia3;
		}
		else {  //noche
			skybox = noche;
			mainLight = luna;
			pointLightCount = MAX_POINT_LIGHTS;
		}

		//luces quiosko
		if (!mainWindow.getLucesPrendidasQuiosko()) {
			spotLightCount = 0;
		}
		else {
			if (mainWindow.getFiesta()) {
				if (estado_fiesta == 0) {
					tiempo_fiesta = tiempo;
					estado_fiesta++;
				}
				int actual = tiempo_fiesta - tiempo;
				int nivel_fiesta = (int)(actual / 5.f) % 6;
				if (nivel_fiesta % 2 == 0) {
					spotLightCount = MAX_SPOT_LIGHTS;
					int indice = (int)(nivel_fiesta / 2);
					spotLights = luces[indice];
				}
				else {
					spotLightCount = 0;
				}
			}
			else {
				spotLightCount = MAX_SPOT_LIGHTS;
				spotLights = luces[0];
				estado_fiesta = 0;
			}
			
		}

		//ANIMACION MANZANA
		float tiempo_manzana_actual = tiempo / f_manzana;
		if (mainWindow.getEstadoManzana() == 0) {  //esta arriba sin moverse
			posicion_manzana.y = altura_manzana_inicial;
			tiempo1 = tiempo_manzana_actual;
		}
		else if (mainWindow.getEstadoManzana() == 1) {  //esta en caida libre
			tiempo_manzana_actual -= tiempo1;
			posicion_manzana.y = altura_manzana_inicial - g * tiempo_manzana_actual * tiempo_manzana_actual;
			if (posicion_manzana.y <= 0) {
				vi = g * tiempo_manzana_actual;
				mainWindow.siguienteEstadoManzana();
				posicion_manzana.y = 0.f;
				tiempo1 = tiempo_manzana_actual;
			}
		}
		else if (mainWindow.getEstadoManzana() == 2) {  //esta en tiro vertical
			tiempo_manzana_actual -= tiempo1;
			posicion_manzana.y = (vi * tiempo_manzana_actual - g * tiempo_manzana_actual * tiempo_manzana_actual / 2);
			if (posicion_manzana.y <= 0) {
				vi = vi * .7;
				posicion_manzana.y = 0.f;
				tiempo1 = tiempo / f_manzana;
			}
		}

		//para las camaras, se usa m para cambiar de camara
		position = camera.getCameraPosition();
		if (mainWindow.getCamera() == 0) {
			position.y = 7.f;
			des.x = position.x;
			des.z = position.z;
			viewMatrix = glm::lookAt(position, des, up);
			posicion_avatar = position;
			posicion_avatar.x = posicion_avatar.x + 1.f;
			posicion_avatar.z = posicion_avatar.z - 1.f;
			posicion_avatar.y = altura_lego * 2;
		}
		else if (mainWindow.getCamera() == 1) {
			position.y = 20.f;
			des.x = position.x;
			des.z = position.z;
			viewMatrix = glm::lookAt(position, des, up);
		}
		else {
			viewMatrix = camera.calculateViewMatrix();
		}

		//animacion avatar caminando
		if (mainWindow.getCaminando()) {
			if (caminando) {
				caminando = 0.f;
				tiempo_caminar = tiempo;
			}
			int tiempo_actual = tiempo - tiempo_caminar;
			if (tiempo_actual > tiempo_maximo) {
				mainWindow.terminarCaminar();
			}
			else {
				float d = velocidad_avatar * (float)tiempo_actual;
				posicion_avatar = posicion_avatar_original + glm::vec3(0.f, 0.f, -d);
				if (subiendo_angulo) {
					if (angulo_avatar == 90) {
						subiendo_angulo = false;
					}
					angulo_avatar += 10.f;
				}
				else {
					if (angulo_avatar == -90) {
						subiendo_angulo = true;
					}
					angulo_avatar -= 10.f;
				}
			}
			
		}
		else {
			caminando = 1.f;
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

		GLfloat now = glfwGetTime();
		deltaTime = now - lastTime;
		lastTime = now;

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

		shaderList[0].SetDirectionalLight(&mainLight);
		shaderList[0].SetPointLights(pointLights, pointLightCount);
		shaderList[0].SetSpotLights(spotLights, spotLightCount);
		
		glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(viewMatrix));
		glUniform3f(uniformEyePosition, position.x, position.y, position.z);

		glm::mat4 model(1.0);
		glm::mat4 aux(1.0);

		glm::mat4 aux1(1.0);
		glm::mat4 aux2(1.0);
		glm::mat4 aux3(1.0);
		glm::mat4 aux4(1.0);

		// ALAMEDA
		model = glm::mat4(1.0);
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		alameda.RenderModel();

		// REJAS
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(-33.0f, 0.0f, -49.62f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		rejaChica.RenderModel();

		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(32.0f, 0.0f, -49.62f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		rejaChica.RenderModel();

		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(-33.0f, 0.0f, 48.47f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		rejaChica.RenderModel();

		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(32.0f, 0.0f, 48.47f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		rejaChica.RenderModel();

		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(-69.0f, 0.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		rejaGrande.RenderModel();

		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(69.0f, 0.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		rejaGrande.RenderModel();

		// LAMPARAS		
		for (int i = 0; i < MAX_POINT_LIGHTS; i++) {
			model = aux;
			model = glm::translate(model, posicion_lamparas[i]);
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			lampara.RenderModel();
		}

		// BANCAS
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(-33.f, 0.0f, -37.f));
		model = glm::rotate(model, 180 * toRadians, glm::vec3(0.f, 1.0f, 0.f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		banca.RenderModel();

		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(32.f, 0.0f, -37.f));
		model = glm::rotate(model, 180 * toRadians, glm::vec3(0.f, 1.0f, 0.f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		banca.RenderModel();

		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(-33.f, 0.0f, 36.f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		banca.RenderModel();

		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(33.f, 0.0f, 36.f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		banca.RenderModel();

		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(-54.5f, 0.0f, 18.f));
		model = glm::rotate(model, -90 * toRadians, glm::vec3(0.f, 1.0f, 0.f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		banca.RenderModel();

		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(-54.5f, 0.0f, -18.f));
		model = glm::rotate(model, -90 * toRadians, glm::vec3(0.f, 1.0f, 0.f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		banca.RenderModel();

		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(53.5f, 0.0f, 18.f));
		model = glm::rotate(model, 90 * toRadians, glm::vec3(0.f, 1.0f, 0.f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		banca.RenderModel();

		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(53.5f, 0.0f, -18.f));
		model = glm::rotate(model, 90 * toRadians, glm::vec3(0.f, 1.0f, 0.f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		banca.RenderModel();

		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(14.5f, 0.0f, 18.f));
		model = glm::rotate(model, -90 * toRadians, glm::vec3(0.f, 1.0f, 0.f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		banca.RenderModel();

		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(14.5f, 0.0f, -18.f));
		model = glm::rotate(model, -90 * toRadians, glm::vec3(0.f, 1.0f, 0.f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		banca.RenderModel();

		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(-14.5f, 0.0f, 18.f));
		model = glm::rotate(model, 90 * toRadians, glm::vec3(0.f, 1.0f, 0.f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		banca.RenderModel();

		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(-14.5f, 0.0f, -18.f));
		model = glm::rotate(model, 90 * toRadians, glm::vec3(0.f, 1.0f, 0.f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		banca.RenderModel();

		// BOTE BASURA
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(-14.5f, 0.0f, 7.f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		boteBasura.RenderModel();

		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(-14.5f, 0.0f, -7.f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		boteBasura.RenderModel();

		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(14.5f, 0.0f, 7.f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		boteBasura.RenderModel();

		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(14.5f, 0.0f, -7.f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		boteBasura.RenderModel();

		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(-54.5f, 0.0f, 7.f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		boteBasura.RenderModel();

		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(-54.5f, 0.0f, -7.f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		boteBasura.RenderModel();

		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(53.5f, 0.0f, 7.f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		boteBasura.RenderModel();

		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(53.5f, 0.0f, -7.f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		boteBasura.RenderModel();

		// Bath
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(72.f, 0.0f, 0.f));
		model = glm::rotate(model, 90 * toRadians, glm::vec3(0.f, 1.f, 0.f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		bath.RenderModel();

		//EMPIEZA QUIOSKO
		//
		//gris claro
		model = aux;
		model = glm::rotate(model, 180 * toRadians, glm::vec3(1.f, 0.f, 0.f));
		model = glm::translate(model, glm::vec3(-44.0f, 2.5f, -9.f));
		model = aux1 = glm::scale(model, glm::vec3(2.f, 2.f, 2.f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		plate_round_corner_6x6.RenderModel();

		//gris claro
		model = aux1;
		model = glm::translate(model, glm::vec3(0.f, 0.f, 12 * largo_lego));
		model = aux2 = glm::rotate(model, 90 * toRadians, glm::vec3(0.f, 1.f, 0.f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		plate_round_corner_6x6.RenderModel();

		//gris claro
		model = aux1;
		model = glm::translate(model, glm::vec3(12 * largo_lego, 0.f, 0));
		model = aux3 = glm::rotate(model, -90 * toRadians, glm::vec3(0.f, 1.f, 0.f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		plate_round_corner_6x6.RenderModel();

		//gris claro
		model = aux1;
		model = glm::translate(model, glm::vec3(12 * largo_lego, 0.f, 12 * largo_lego));
		model = aux4 = glm::rotate(model, 180 * toRadians, glm::vec3(0.f, 1.f, 0.f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		plate_round_corner_6x6.RenderModel();

		glm::mat4 arr[] = { aux1, aux2, aux3, aux4 };

		for (int i = 0; i < 4; i++) {
			//gris claro
			model = arr[i];
			model = arr[i] = glm::translate(model, glm::vec3(4 * largo_lego, -altura_lego / 3, 0));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			brick_1x4.RenderModel();
		}

		//escaleras

		for (int i = 0; i < 4; i++) {
			//gris claro
			model = arr[i];
			model = glm::translate(model, glm::vec3(8 * largo_lego, 0.f, 0.f));
			model = glm::rotate(model, -90 * toRadians, glm::vec3(0.f, 1.f, 0.f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			brick_round_corner_4x4.RenderModel();

			//blanco
			model = arr[i];
			model = arr[i] = glm::translate(model, glm::vec3(-largo_lego * 4, -altura_lego, 0.f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			plate_round_corner_6x6.RenderModel();
		}

		//piso

		for (int i = 0; i < 4; i++) {
			//blanco
			model = arr[i];
			model = glm::translate(model, glm::vec3(largo_lego * 3, -altura_lego / 3, 0.f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			plate_1x1.RenderModel();

			//blanco
			model = arr[i];
			model = arr[i] = glm::translate(model, glm::vec3(8 * largo_lego, -altura_lego / 3, 0.f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			plate_1x1.RenderModel();

			//blanco
			model = arr[i];
			model = arr[i] = glm::translate(model, glm::vec3(-5 * largo_lego, -altura_lego / 3, 0.f));
			model = glm::rotate(model, 270 * toRadians, glm::vec3(0.f, 1.f, 0.f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			arco.RenderModel();

			//blanco
			model = arr[i];
			model = arr[i] = glm::translate(model, glm::vec3(largo_lego, -5 * altura_lego, largo_lego));
			model = glm::rotate(model, 90 * toRadians, glm::vec3(0.f, 1.f, 0.f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			plate_modified_1x4.RenderModel();

			//blanco
			model = arr[i];
			model = glm::translate(model, glm::vec3(0.f, 0.f, 3 * largo_lego));
			model = glm::rotate(model, 180 * toRadians, glm::vec3(0.f, 1.f, 0.f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			title_round_corner_4x4.RenderModel();

			//rojo vino
			model = arr[i];
			model = arr[i] = glm::translate(model, glm::vec3(0.f, -altura_lego / 3, 3 * largo_lego));
			model = glm::rotate(model, 90 * toRadians, glm::vec3(0.f, 1.f, 0.f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			slope_18_4x2.RenderModel();

			//rojo vino
			model = arr[i];
			model = arr[i] = glm::translate(model, glm::vec3(2 * largo_lego, 0.f, 0.f));
			model = glm::rotate(model, 90 * toRadians, glm::vec3(0.f, 1.f, 0.f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			slope_18_4x2.RenderModel();

			//rojo vino
			model = arr[i];
			model = glm::translate(model, glm::vec3(2 * largo_lego, 0.f, 0.f));
			model = glm::rotate(model, 90 * toRadians, glm::vec3(0.f, 1.f, 0.f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			wedge_4x4.RenderModel();

			//rojo
			model = arr[i];
			model = glm::translate(model, glm::vec3(largo_lego, altura_lego / 3, -largo_lego));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			plate_1x2.RenderModel();

			//blanco
			model = arr[i];
			model = arr[i] = glm::translate(model, glm::vec3(largo_lego, -altura_lego, -largo_lego));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			plate_1x2.RenderModel();

			//rojo vino
			model = arr[i];
			model = glm::translate(model, glm::vec3(2 * largo_lego, 4 * altura_lego / 3, 0.f));
			model = glm::rotate(model, -90 * toRadians, glm::vec3(0.f, 1.f, 0.f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			plate_2x2_corner.RenderModel();

			//blanco
			model = arr[i];
			model = glm::translate(model, glm::vec3(2 * largo_lego, 0.f, 0.f));
			model = glm::rotate(model, -90 * toRadians, glm::vec3(0.f, 1.f, 0.f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			plate_2x2_corner.RenderModel();

			//blanco
			model = arr[i];
			model = arr[i] = glm::translate(model, glm::vec3(-3.f * largo_lego, -altura_lego / 3, 0.f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			frence_1x4x1.RenderModel();

			//blanco
			model = arr[i];
			model = arr[i] = glm::translate(model, glm::vec3(0.f, 0.f, 1 * largo_lego));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			cilindro.RenderModel();

			//blanco
			model = arr[i];
			//model = glm::scale(model, glm::vec3(100.f, 100.0f, 100.f));
			//model = glm::rotate(model, 90 * toRadians, glm::vec3(0.f, 1.f, 0.f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			slop_inverted_45_2x2.RenderModel();

			//blanco
			model = arr[i];
			model = glm::translate(model, glm::vec3(0.f, 2 * altura_lego, largo_lego));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			telescopio.RenderModel();

			//rojo vino
			model = arr[i];
			model = glm::translate(model, glm::vec3(largo_lego, -altura_lego, largo_lego));
			model = glm::rotate(model, 90 * toRadians, glm::vec3(0.f, 1.f, 0.f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			slope_18_4x2.RenderModel();

			//rojo vino
			model = arr[i];
			model = glm::translate(model, glm::vec3(3 * largo_lego, -altura_lego, largo_lego));
			model = glm::rotate(model, 90 * toRadians, glm::vec3(0.f, 1.f, 0.f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			wedge_4x4.RenderModel();

			//blanco
			model = arr[i];
			model = glm::translate(model, glm::vec3(largo_lego, -altura_lego, largo_lego));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			brick_2x2.RenderModel();

			//blanco
			model = arr[i];
			model = aux2 = glm::translate(model, glm::vec3(largo_lego, -2 * altura_lego, largo_lego));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			cone_1x1.RenderModel();

			//blanco
			model = arr[i];
			model = glm::translate(model, glm::vec3(2 * largo_lego, -2 * altura_lego, 0.f));
			model = glm::rotate(model, 180 * toRadians, glm::vec3(0.f, 1.f, 0.f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			title_2x2_corner.RenderModel();

			arr[i] = aux2;
		}

		//rojo vino
		aux2 = arr[0];
		model = glm::translate(model, glm::vec3(-largo_lego, -altura_lego, -3 * largo_lego));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		plate_2x2_modified.RenderModel();

		//rojo vino
		model = glm::translate(model, glm::vec3(largo_lego / 2, -altura_lego / 3, largo_lego / 2));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		arriba.RenderModel();

		//
		//TERIMNA QUIOSKO
		//

		//INICIO AVATAR
		//

		//torso
		
		model = glm::mat4(1.0);
		model = glm::translate(model, posicion_avatar);
		model = glm::rotate(model, 180 * toRadians, glm::vec3(1.f, 0.f, 0.f));
		//model = glm::rotate(model, 90 * toRadians, glm::vec3(0.f, 1.f, 0.f));
		model = aux2 = glm::translate(model, glm::vec3(-1.f,-1.f,-1.f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		torso_plain.RenderModel();

		//brazo izquierdo
		model = aux2;
		model = glm::translate(model, glm::vec3(2 * largo_lego / 3, altura_lego / 3, 0.f));
		model = glm::rotate(model, angulo_avatar * toRadians, glm::vec3(1.f, 0.f, 0.f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		arm_left.RenderModel();

		//mano izquierda
		model = glm::translate(model, glm::vec3(1.25 * largo_lego / 3, 2 * altura_lego / 3, largo_lego / 3));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		hand.RenderModel();

		//brazo derecho
		model = aux2;
		model = glm::translate(model, glm::vec3(-2 * largo_lego / 3, altura_lego / 3, 0.f));
		model = glm::rotate(model, -angulo_avatar * toRadians, glm::vec3(1.f, 0.f, 0.f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		arm_right.RenderModel();

		//mano derecha
		model = glm::translate(model, glm::vec3(-1.25 * largo_lego / 3, 2 * altura_lego / 3, largo_lego / 3));
		model = glm::rotate(model, -19 * toRadians, glm::vec3(0.f, 1.f, 0.f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		hand.RenderModel();

		//hips
		model = aux2;
		model = aux3 = glm::translate(model, glm::vec3(0.f, 4 * altura_lego / 3, 0.f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		hips.RenderModel();

		//pie izquierdo
		model = aux3;
		model = glm::translate(model, glm::vec3(0.f, 1.5f * altura_lego / 3, 0.f));
		model = glm::rotate(model, -angulo_avatar * toRadians, glm::vec3(1.f, 0.f, 0.f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		leg_left.RenderModel();

		//pie derecho
		model = aux3;
		model = glm::translate(model, glm::vec3(0.f, 1.5f * altura_lego / 3, 0.f));
		model = glm::rotate(model, angulo_avatar * toRadians, glm::vec3(1.f, 0.f, 0.f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		leg_right.RenderModel();

		//cabeza
		model = aux2;
		model = glm::translate(model, glm::vec3(0.f, -altura_lego, 0.f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		head.RenderModel();

		//sombrero
		model = glm::translate(model, glm::vec3(0.f, 0.f, 0.f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		hat.RenderModel();

		//
		//
		//FIN AVATAR
		//

		//arbol manzano
		model = aux;
		model = glm::translate(model, posicion_arbol_animacion);
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		arbol_manzano.RenderModel();

		//Manzana
		escala = 0.3125;  //DEBE ESTAR ABAJO DE ARBOL MANZANO
		model = glm::translate(model, posicion_manzana);
		model = glm::scale(model, glm::vec3(escala, escala, escala));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		manzana.RenderModel();

		//BALLENA
		model = aux;
		model = glm::translate(model, glm::vec3(posicion_delfin.x + movAvion_x, posicion_delfin.y +
			movAvion_y, posicion_delfin.z));
		//model = glm::rotate(model, 180 * toRadians, glm::vec3(1.f, 0.f, 0.f));
		model = glm::scale(model, glm::vec3(3.0f, 3.0f, 3.0f));
		model = glm::rotate(model, giroAvion * toRadians, glm::vec3(0.f, 0.f, 1.f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		ballena.RenderModel();

		//barquito
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(barco_x, barco_y, barco_z));
		model = glm::rotate(model, 90 * toRadians, glm::vec3(0.f, 1.f, 0.f));
		model = glm::rotate(model, -angulo * toRadians, glm::vec3(0.f, 1.f, 0.f));
		model = glm::scale(model, glm::vec3(.1f, 0.1f, 0.1f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
		barco.RenderModel();

		mainWindow.swapBuffers();

		tiempo++;
	}

	return 0;
}