// Include standard headers
#define _CRT_SECURE_NO_WARNINGS

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <stdio.h>
#include <stdlib.h>
#include <vector>

#include <string>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <sstream>
using namespace std;

#include <stdlib.h>
#include <string.h>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
using namespace glm;

#include "controls.hpp"
#include "objloader.hpp"

GLuint lastPressedKeycode = 0;
void characterCallback(GLFWwindow* window, unsigned int keycode)
{
	lastPressedKeycode = keycode;
}

typedef struct Texture {
	GLuint textureID;
	GLuint uniformID;

	Texture(GLuint TextureID, GLuint UniformID){
		textureID = TextureID;
		uniformID = UniformID;
	}
} Texture;

enum GameObjectType
{
	SUN,
	PLANET,
	METEOR
};

typedef struct ObjectData {
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec2> uvs;
	GLuint vertexbuffer;
	//GLuint normalbuffer;
	GLuint uvbuffer;

	ObjectData(std::vector<glm::vec3> Vertices,
		std::vector<glm::vec3> Normals,
		std::vector<glm::vec2> Uvs,
		GLuint Vertexbuffer,
		//GLuint Normalbuffer,
		GLuint Uvbuffer
	) 
	{
		vertices = Vertices;
		normals = Normals;
		uvs = Uvs;
		vertexbuffer = Vertexbuffer;
		//normalbuffer = Normalbuffer;
		uvbuffer = Uvbuffer;
	}
} ObjectData;

typedef struct GameObject {
	ObjectData* objectData;
	Texture* texture;
	glm::mat4 modelMatrix;
	enum GameObjectType type;

	GameObject(ObjectData* ObjectData,
		Texture* _texture,
		glm::mat4 ModelMatrix,
		enum GameObjectType Type)
	{
		objectData = ObjectData;
		texture = _texture;
		modelMatrix = ModelMatrix;
		type = Type;
	}
}GameObject;

GLuint LoadShaders(const char* vertex_file_path, const char* fragment_file_path) {

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if (VertexShaderStream.is_open()) {
		std::stringstream sstr;
		sstr << VertexShaderStream.rdbuf();
		VertexShaderCode = sstr.str();
		VertexShaderStream.close();
	}
	else {
		printf("Impossible to open %s. Are you in the right directory ? Don't forget to read the FAQ !\n", vertex_file_path);
		getchar();
		return 0;
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if (FragmentShaderStream.is_open()) {
		std::stringstream sstr;
		sstr << FragmentShaderStream.rdbuf();
		FragmentShaderCode = sstr.str();
		FragmentShaderStream.close();
	}
	else {
		printf("Impossible to open %s. Are you in the right directory ? Don't forget to read the FAQ !\n", fragment_file_path);
		getchar();
		return 0;
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;


	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const* VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer, NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> VertexShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
		printf("%s\n", &VertexShaderErrorMessage[0]);
	}



	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const* FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer, NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> FragmentShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
		printf("%s\n", &FragmentShaderErrorMessage[0]);
	}



	// Link the program
	printf("Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
		glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
		printf("%s\n", &ProgramErrorMessage[0]);
	}


	glDetachShader(ProgramID, VertexShaderID);
	glDetachShader(ProgramID, FragmentShaderID);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}

// Very, VERY simple OBJ loader.
// Here is a short list of features a real function would provide : 
// - Binary files. Reading a model should be just a few memcpy's away, not parsing a file at runtime. In short : OBJ is not very great.
// - Animations & bones (includes bones weights)
// - Multiple UVs
// - All attributes should be optional, not "forced"
// - More stable. Change a line in the OBJ file and it crashes.
// - More secure. Change another line and you can inject code.
// - Loading from memory, stream, etc

bool loadOBJ(
	const char* path,
	std::vector<glm::vec3>& out_vertices,
	std::vector<glm::vec2>& out_uvs,
	std::vector<glm::vec3>& out_normals
) {
	printf("Loading OBJ file %s...\n", path);

	std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
	std::vector<glm::vec3> temp_vertices;
	std::vector<glm::vec2> temp_uvs;
	std::vector<glm::vec3> temp_normals;


	FILE* file = fopen(path, "r");
	if (file == NULL) {
		printf("Impossible to open the file ! Are you in the right path ? See Tutorial 1 for details\n");
		getchar();
		return false;
	}

	while (1) {

		char lineHeader[128];
		// read the first word of the line
		int res = fscanf(file, "%s", lineHeader);
		if (res == EOF)
			break; // EOF = End Of File. Quit the loop.

		// else : parse lineHeader

		if (strcmp(lineHeader, "v") == 0) {
			glm::vec3 vertex;
			fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
			temp_vertices.push_back(vertex);
		}
		else if (strcmp(lineHeader, "vt") == 0) {
			glm::vec2 uv;
			fscanf(file, "%f %f\n", &uv.x, &uv.y);
			uv.y = -uv.y; // Invert V coordinate since we will only use DDS texture, which are inverted. Remove if you want to use TGA or BMP loaders.
			temp_uvs.push_back(uv);
		}
		else if (strcmp(lineHeader, "vn") == 0) {
			glm::vec3 normal;
			fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
			temp_normals.push_back(normal);
		}
		else if (strcmp(lineHeader, "f") == 0) {
			std::string vertex1, vertex2, vertex3;
			unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
			int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2]);
			if (matches != 9) {
				printf("File can't be read by our simple parser :-( Try exporting with other options\n");
				fclose(file);
				return false;
			}
			vertexIndices.push_back(vertexIndex[0]);
			vertexIndices.push_back(vertexIndex[1]);
			vertexIndices.push_back(vertexIndex[2]);
			uvIndices.push_back(uvIndex[0]);
			uvIndices.push_back(uvIndex[1]);
			uvIndices.push_back(uvIndex[2]);
			normalIndices.push_back(normalIndex[0]);
			normalIndices.push_back(normalIndex[1]);
			normalIndices.push_back(normalIndex[2]);
		}
		else {
			// Probably a comment, eat up the rest of the line
			char stupidBuffer[1000];
			fgets(stupidBuffer, 1000, file);
		}

	}

	// For each vertex of each triangle
	for (unsigned int i = 0; i < vertexIndices.size(); i++) {

		// Get the indices of its attributes
		unsigned int vertexIndex = vertexIndices[i];
		unsigned int uvIndex = uvIndices[i];
		unsigned int normalIndex = normalIndices[i];

		// Get the attributes thanks to the index
		glm::vec3 vertex = temp_vertices[vertexIndex - 1];
		glm::vec2 uv = temp_uvs[uvIndex - 1];
		glm::vec3 normal = temp_normals[normalIndex - 1];

		// Put the attributes in buffers
		out_vertices.push_back(vertex);
		out_uvs.push_back(uv);
		out_normals.push_back(normal);

	}
	fclose(file);
	return true;
}


Texture loadTexture(const char *imageName, GLuint programID) {
	// Load the texture
	int width, height, nrChannels;
	unsigned char* data = stbi_load(imageName, &width, &height, &nrChannels, 0);

	if (data)
	{

	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}

	GLuint textureID;
	glGenTextures(1, &textureID);

	// "Bind" the newly created texture : all future texture functions will modify this texture
	glBindTexture(GL_TEXTURE_2D, textureID);

	// Give the image to OpenGL
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);

	// Get a handle for our "myTextureSampler" uniform
	GLuint TextureID = glGetUniformLocation(programID, "myTextureSampler");

	Texture texture(textureID, TextureID);
	return texture;
}

ObjectData initializeObject(const char * objectFilename) {
	// Read our .obj file
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec2> uvs;
	bool result1 = loadOBJ(objectFilename, vertices, uvs, normals);

	// Load it into a VBO
	GLuint vertexbuffer;
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);

	GLuint uvbuffer;
	glGenBuffers(1, &uvbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);

	ObjectData object(vertices, normals, uvs, vertexbuffer, uvbuffer);
	return object;
}

void changeSpeed(GLfloat* speed, GLuint keycode) {
	// Increase speed, 'p' = 112
	if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS && keycode == 112)
		*speed += 1;
	// Decrease speed, 'u' = 117
	if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS && keycode == 117) {
		if (*speed == 0)
			return;
		*speed -= 1;
	}
}

void rotateAroundSun(std::vector<GameObject> &objects, GLfloat speed, GLfloat deltaTime) {
	// Model matrix indices
	//	0  4  8  12
	//	1  5  9  13
	//	2  6 10  14
	//	3  7 11  15
	// 
	// 12,13,14 -> x y z

	// make calculations
	for (int i = 0; i < objects.size(); i++) {
		if (objects[i].type != GameObjectType::PLANET)
			continue;

		glm::vec3 position = glm::vec3(objects[i].modelMatrix[3][0],
				objects[i].modelMatrix[3][1], objects[i].modelMatrix[3][2]);

		float angleX = glm::atan(position.z, position.x); // Get angle in radians
		angleX = angleX * 180 / glm::pi<float>();		  // Rads to Degrees
		angleX += speed * deltaTime;					  // Increase angle
		angleX = angleX * glm::pi<float>() / 180;		  // back to Radians

		// Standard circle equation: r^2 = x^2 + y^2
		float radius = sqrtf(pow(position.x, 2) + pow(position.z, 2));
		// update positions
		position.x = radius * glm::cos(angleX);
		position.z = radius * glm::sin(angleX);
		objects[i].modelMatrix[3][0] = position.x;
		objects[i].modelMatrix[3][2] = position.z;
	}
}

void spawnMeteors(std::vector<GameObject>& objects, GameObject dummyMeteor) {
	if (!(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS))
		return; 
	
	static double previousSpawn = glfwGetTime();
	double currentSpawnTimer = glfwGetTime();
	double meteorSpawnInterval = 0.5; // seconds

	if (currentSpawnTimer - previousSpawn <= meteorSpawnInterval)
		return;
	
	previousSpawn = currentSpawnTimer;

	GameObject meteor = dummyMeteor;
	meteor.modelMatrix = glm::translate(glm::mat4(1.0), getCameraPosition());
	objects.push_back(meteor);
}

void moveMeteors(std::vector<GameObject>& objects, GLfloat speed, GLfloat deltaTime) {
	for (int i = 0; i < objects.size(); i++) {
		if (objects[i].type != GameObjectType::METEOR)
			continue;

		glm::vec3 position = glm::vec3(objects[i].modelMatrix[3][0],
			objects[i].modelMatrix[3][1], objects[i].modelMatrix[3][2]);
		glm::vec3 diff = glm::vec3(0, 0, 0) - position;
		float length = sqrt(pow(diff.x, 2) + pow(diff.y, 2) + pow(diff.z, 2));
		glm::vec3 normalized = glm::vec3(diff.x / length, diff.y / length, diff.z / length);
		position += normalized * speed * deltaTime;
		objects[i].modelMatrix[3][0] = position.x;
		objects[i].modelMatrix[3][1] = position.y;
		objects[i].modelMatrix[3][2] = position.z;
	}
}

void detectMeteorCollisions(std::vector<GameObject>& objects) {
	for (int i = 0; i < objects.size(); i++) {
		// skip all except meteors
		if (objects[i].type != GameObjectType::METEOR)
			continue;

		// here we store the colliding objects and textures
		std::vector<GameObject> collidedObjects;
		// meteor position
		glm::vec3 meteor = glm::vec3(objects[i].modelMatrix[3][0],
			objects[i].modelMatrix[3][1], objects[i].modelMatrix[3][2]);

		// loop all elements for every meteor, see who collided
		for (int otherObj = 0; otherObj < objects.size(); otherObj++) {
			// dont calculate itself
			if (i == otherObj)
				continue;

			// get other object position
			glm::vec3 obj = glm::vec3(objects[otherObj].modelMatrix[3][0],
				objects[otherObj].modelMatrix[3][1], objects[otherObj].modelMatrix[3][2]);
			// calculate center to center distance
			GLfloat distance = sqrt(pow(meteor.x - obj.x, 2) + pow(meteor.y - obj.y, 2) + pow(meteor.z - obj.z, 2));

			// sun radius = 15
			// meteor radius = 2
			// total distance == 15 + 2
			if (objects[otherObj].type == GameObjectType::SUN && distance <= 15 + 2) 
			{
				// delete this meteor
				collidedObjects.push_back(objects[i]);
			}
			// planet radius = 5
			// meteor radius = 2
			// total distance between planet and meteor = 5 + 2
			// total distance between meteors = 2 + 2
			else if ((objects[otherObj].type == GameObjectType::PLANET && distance <= 5 + 2) ||
				(objects[otherObj].type == GameObjectType::METEOR && distance <= 2 + 2)) 
			{
				// delete this meteor
				collidedObjects.push_back(objects[i]);
				// delete this collided obj
				collidedObjects.push_back(objects[otherObj]);
			}
		}

		// delete all objects it collided with
		for (int j = 0; j < collidedObjects.size(); j++) {
			objects.erase(
				std::remove_if(objects.begin(), objects.end(), [&](GameObject const& obj) {
					return obj.modelMatrix == collidedObjects[j].modelMatrix;
					}),
				objects.end());
		}
	}
}

void drawObject(GameObject object, GLuint MatrixID, glm::mat4 ProjectionMatrix, glm::mat4 ViewMatrix) {
	// calculate MVP for each object, based on the explicit Model Matrix of its
	glm::mat4 MVP = ProjectionMatrix * ViewMatrix * object.modelMatrix;
	glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

	// Bind our texture in Texture Unit 0
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, object.texture->textureID);
	// Set our "myTextureSampler" sampler to use Texture Unit 0
	glUniform1i(object.texture->uniformID, 0);

	// Draw the triangle !
	glDrawArrays(GL_TRIANGLES, 0, object.objectData->vertices.size());
}

int main(void)
{
	// Initialise GLFW
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a window and create its OpenGL context
	window = glfwCreateWindow(800, 800, u8"Ηλιακό σύστημα", NULL, NULL);
	if (window == NULL) {
		fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		glfwTerminate();
		return -1;
	}

	// Pass the window to the controls 
	setControlsWindow(window);

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	// Hide the mouse and enable unlimited mouvement
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	// Callbacks
	glfwSetCharCallback(window, characterCallback);

	// Set the mouse at the center of the screen
	glfwPollEvents();
	
	// Black background
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);

	// Cull triangles which normal is not towards the camera
	glEnable(GL_CULL_FACE);

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	// Create and compile our GLSL program from the shaders
	GLuint programID = LoadShaders(
		"Shaders\\TransformVertexShader.vertexshader", 
		"Shaders\\TextureFragmentShader.fragmentshader"
	);

	// Get a handle for our "MVP" uniform
	GLuint MatrixID = glGetUniformLocation(programID, "MVP");
	
	// Load textures, in order of the solar system (except meteor at [1])
	std::vector<const char*> textureFilenames = { 
		"Textures\\sun.jpg", 
		"Textures\\meteor.jpg", 
		"Textures\\mercury.jpg", 
		"Textures\\venus.jpg",
		"Textures\\mars.jpg", 
		"Textures\\jupiter.jpg", 
		"Textures\\saturn.jpg", 
		"Textures\\uranus.jpg", 
		"Textures\\neptune.jpg" 
	};
	std::vector<Texture> textures;

	for (int i = 0; i < textureFilenames.size(); i++) {
		Texture texture = loadTexture(textureFilenames[i], programID);
		textures.push_back(texture);
	}
	
	// Load Objects
	std::vector<const char*> objectsFilenames = { 
		"Objects\\sun.obj", 
		"Objects\\meteor.obj", 
		"Objects\\planet.obj" 
	};
	std::vector<ObjectData> objectTypes;

	for (int i = 0; i < objectsFilenames.size(); i++) {
		ObjectData object = initializeObject(objectsFilenames[i]);
		objectTypes.push_back(object);
	}

	// dont need these vectors any more
	textureFilenames.clear();
	textureFilenames.shrink_to_fit();
	objectsFilenames.clear();
	objectsFilenames.shrink_to_fit();

	// Create position vectors of each planet
	std::vector<glm::vec3> planetPositions;
	planetPositions.push_back(glm::vec3(25, 0, 25));
	planetPositions.push_back(glm::vec3(0, 0, -50));
	planetPositions.push_back(glm::vec3(0, 0, 75));
	planetPositions.push_back(glm::vec3(100, 0, 0));
	planetPositions.push_back(glm::vec3(-125, 0, 0));
	planetPositions.push_back(glm::vec3(-150, 0, 100));
	planetPositions.push_back(glm::vec3(90, 0, 175));

	// Create objects
	std::vector<GameObject> objects;

	// sun
	objects.push_back(GameObject(&objectTypes[0], &textures[0], glm::mat4(1.0), GameObjectType::SUN));

	// dummy meteor object, not inserted in object's vector, used to make new meteors
	GameObject meteor = GameObject(&objectTypes[1], &textures[1], glm::mat4(1.0), GameObjectType::METEOR);

	for (int i = 0; i < planetPositions.size(); i++) {	
		glm::mat4 model = glm::translate(glm::mat4(1.0), planetPositions[i]);
		// +2, because [0] = sun, [1] = meteor
		objects.push_back(GameObject(&objectTypes[2], &textures[i+2], model, GameObjectType::PLANET));
	}
	

	// speed 
	GLfloat speed = 20.0f;
	do {
		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Use our shader
		glUseProgram(programID);

		// Calculate deltaTime
		static double lastTime = glfwGetTime();
		double currentTime = glfwGetTime();
		float deltaTime = float(currentTime - lastTime);

		// Compute the MVP matrix from keyboard and mouse input
		computeMatricesFromInputs(lastPressedKeycode, deltaTime);
		glm::mat4 ProjectionMatrix = getProjectionMatrix();
		glm::mat4 ViewMatrix = getViewMatrix();

		changeSpeed(&speed, lastPressedKeycode);
		rotateAroundSun(objects, speed, deltaTime);

		spawnMeteors(objects, meteor);
		moveMeteors(objects, speed, deltaTime);
		detectMeteorCollisions(objects);

		// draw for each type
		for (int i = 0; i < objectTypes.size(); i++)
		{
			// 1rst attribute buffer : vertices
			glEnableVertexAttribArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, objectTypes[i].vertexbuffer);
			glVertexAttribPointer(
				0,                  // attribute
				3,                  // size
				GL_FLOAT,           // type
				GL_FALSE,           // normalized?
				0,                  // stride
				(void*)0            // array buffer offset
			);

			// 2nd attribute buffer : UVs
			glEnableVertexAttribArray(1);
			glBindBuffer(GL_ARRAY_BUFFER, objectTypes[i].uvbuffer);
			glVertexAttribPointer(
				1,                                // attribute
				2,                                // size
				GL_FLOAT,                         // type
				GL_FALSE,                         // normalized?
				0,                                // stride
				(void*)0                          // array buffer offset
			);

			// draw all of each type, etc all suns or all planets
			for (int o = 0; o < objects.size(); o++)
				if (objectTypes[i].vertexbuffer == objects[o].objectData->vertexbuffer)
					drawObject(objects[o], MatrixID, ProjectionMatrix, ViewMatrix);

			glDisableVertexAttribArray(0);
			glDisableVertexAttribArray(1);
		}

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

		// For the next frame, the "last time" will be "now"
		lastTime = currentTime;

	} // Check if the Q key was pressed or the window was closed
	while (lastPressedKeycode != 81 && glfwWindowShouldClose(window) == 0);
	


	// Clear object data
	for (int i = 0; i < objectTypes.size(); i++)
	{
		glDeleteBuffers(1, &objectTypes[i].vertexbuffer);
		glDeleteBuffers(1, &objectTypes[i].uvbuffer);
	}
	// Clear textures
	for (int i = 0; i < textures.size(); i++)
	{
		glDeleteTextures(1, &textures[i].textureID);
	}
	// clear etc
	glDeleteProgram(programID);
	glDeleteVertexArrays(1, &VertexArrayID);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}