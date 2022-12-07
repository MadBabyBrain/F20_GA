// 3D Graphics and Animation - Main Template
// This uses Visual Studio Code - https://code.visualstudio.com/docs/cpp/introvideos-cpp
// Two versions available -  Win64 and Apple MacOS - please see notes
// Last changed August 2022

//#pragma comment(linker, "/NODEFAULTLIB:MSVCRT")

//#define STB_IMAGE_IMPLEMENTATION

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include <map>
using namespace std;

#define GL_SILENCE_DEPRECATION
#include <glad/glad.h> // Extend OpenGL Specfication to version 4.5 for WIN64 and versions 4.1 for Apple (note: two different files).

#include <imgui/imgui.h>			  // Load GUI library - Dear ImGui - https://github.com/ocornut/imgui
#include <imgui/imgui_impl_glfw.h>	  // Platform ImGui using GLFW
#include <imgui/imgui_impl_opengl3.h> // Platform new OpenGL - aka better than 3.3 core version.

#include <GLFW/glfw3.h> // Add GLFW - library to launch a window and callback mouse and keyboard - https://www.glfw.org

#define GLM_ENABLE_EXPERIMENTAL	 // Enable all functions include non LTS
#include <glm/glm.hpp>			 // Add helper maths library - GLM 0.9.9.9 - https://github.com/g-truc/glm - for example variables vec3, mat and operators.
#include <glm/gtx/transform.hpp> // Help us with transforms
using namespace glm;
 
#include <tinygltf/tiny_gltf.h> // Model loading library - tiny gltf - https://github.com/syoyo/tinygltf
// #include "src/stb_image.hpp" // Add library to load images for textures

// #include "src/Mesh.hpp" // Simplest mesh holder and OBJ loader - can update more - from https://github.com/BennyQBD/ModernOpenGLTutorial

#include "src/Pipeline.hpp"		// Setup pipeline and load shaders.
#include "src/Content.hpp"		// Setup content loader and drawing functions - https://github.com/KhronosGroup/glTF - https://github.com/syoyo/tinygltf 
#include "src/Debugger.hpp"		// Setup debugger functions.

# define M_PIl 3.141592653589793238462643383279502884L

// Main fuctions
void startup();
void update();
void render();
void ui();
void endProgram();

// HELPER FUNCTIONS OPENGL
void hintsGLFW();
//string readShader(string name);
//void checkErrorShader(GLuint shader);
inline void errorCallbackGLFW(int error, const char *description){cout << "Error GLFW: " << description << "\n";};
void debugGL();

void APIENTRY openGLDebugCallback(GLenum source,
								  GLenum type,
								  GLuint id,
								  GLenum severity,
								  GLsizei length,
								  const GLchar *message,
								  const GLvoid *userParam);
GLenum glCheckError_(const char *file, int line);
#define glCheckError() glCheckError_(__FILE__, __LINE__)

// Setup all the message loop callbacks - do this before Dear ImGui
// Callback functions for the window and interface devices
void onResizeCallback(GLFWwindow *window, int w, int h);
void onKeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
void onMouseButtonCallback(GLFWwindow *window, int button, int action, int mods);
void onMouseMoveCallback(GLFWwindow *window, double x, double y);
void onMouseWheelCallback(GLFWwindow *window, double xoffset, double yoffset);

// VARIABLES
GLFWwindow *window; 								// Keep track of the window
auto windowWidth = 1280;							// Window width					
auto windowHeight =720;							// Window height
auto running(true);							  		// Are we still running our main loop
mat4 projMatrix;							 		// Our Projection Matrix
vec3 cameraPosition = vec3(2.0f, 0.75f, 1.0f);		// Where is our camera
vec3 cameraFront = vec3(0.0f, 0.0f, -1.0f);			// Camera front vector
vec3 cameraUp = vec3(0.0f, 1.0f, 0.0f);				// Camera up vector
auto aspect = (float)windowWidth / (float)windowHeight;	// Window aspect ration
auto fovy = 45.0f;									// Field of view (y axis)
bool keyStatus[1024];								// Track key strokes
auto currentTime = 0.0f;							// Framerate
auto deltaTime = 0.0f;								// time passed
auto lastTime = 0.0f;								// Used to calculate Frame rate

float yaw = -90.0f;
float pitch = 0.0f;
auto lastX = 0.0f;
auto lastY = 0.0f;
bool firstMouse = true;

// pair<GLuint, std::map<int, GLuint>> vaoAndEbos;
// Model model;

Pipeline pipeline;									// Add one pipeline plus some shaders.
map<string, Content> content;						// Add one content loader (+drawing).
// map<string, vector<Texture>> textures;
Debugger debugger;									// Add one debugger to use for callbacks ( Win64 - openGLDebugCallback() ) or manual calls ( Apple - glCheckError() ) 

// map<string, vector<vec3>> modelPosition;			// Model position
// map<string, vector<vec3>> modelOffset;				// Model offset
// map<string, vector<vec3>> modelRotation;			// Model rotation;
// map<string, vector<vec3>> modelScale;				// Model scale

// map<string, vector<int>> animations;				// holds model_name -> list of model_position_indexes
// map<string, vector<vec3>> velocities;				// model velocity

struct object {
	string type;
	string name;
	vec3 position;
	vec3 offset;
	vec3 rotation;
	vec3 scale;
	bool animation;
	vec3 velocity;
};

// map<string, modelholder> models;
map<string, vector<object>> objects;	// array of all objects


glm::mat4 lightPos; 	// position of light in world
glm::vec4 ia; 			// ambiant light colour
glm::mat4 id; 			// diffuse light colour
glm::mat4 is; 			// specular light colour

bool Ls = false;
bool Lp = false;
bool Lc = false;

float ka = 0.5f;
float kd = 0.8f;
float ks = 1.0f;
float shininess = 32.0f;

int frame = 0;

int main()
{
	cout << endl << "===" << endl << "3D Graphics and Animation - Running..." << endl;

	if (!glfwInit()) // Check if GLFW is working, if not then stop all
	{
		cout << "Could not initialise GLFW...";
		return -1;
	} 

	glfwSetErrorCallback(errorCallbackGLFW); // Setup a function callback to catch and display all GLFW errors.
	hintsGLFW();							 // Setup glfw with various hints.

	const GLFWvidmode *mode = glfwGetVideoMode(glfwGetPrimaryMonitor()); // Grab reference to monitor
	// windowWidth = mode->width; windowHeight = mode->height; //fullscreen
	// window = glfwCreateWindow(windowWidth, windowHeight, title.c_str(), glfwGetPrimaryMonitor(), NULL); // fullscreen

	// Create our Window
	const auto windowTitle = "My 3D Graphics and Animation OpenGL Application"s;
	window = glfwCreateWindow(windowWidth, windowHeight, windowTitle.c_str(), NULL, NULL);
	if (!window) // Test if Window or OpenGL context creation failed
	{
		cout << "Could not initialise GLFW...";
		glfwTerminate();
		return -1;
	} 

	glfwSetWindowPos(window, 100, 100); // Place it in top corner for easy debugging.
	glfwMakeContextCurrent(window);	  // making the OpenGL context current

	// GLAD: Load OpenGL function pointers - aka update specs to newest versions - plus test for errors.
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD...";
		glfwMakeContextCurrent(NULL);
		glfwTerminate();
		return -1;
	}

	glfwSetWindowSizeCallback(window, onResizeCallback);	   // Set callback for resize
	glfwSetKeyCallback(window, onKeyCallback);				   // Set Callback for keys
	glfwSetMouseButtonCallback(window, onMouseButtonCallback); // Set callback for mouse click
	glfwSetCursorPosCallback(window, onMouseMoveCallback);	   // Set callback for mouse move
	glfwSetScrollCallback(window, onMouseWheelCallback);	   // Set callback for mouse wheel.
	// glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);	// Set mouse cursor Fullscreen
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);	// Set mouse cursor FPS fullscreen.

	// Setup Dear ImGui and add context	-	https://blog.conan.io/2019/06/26/An-introduction-to-the-Dear-ImGui-library.html
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO &io = ImGui::GetIO(); //(void)io;
								  // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	// io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	ImGui::StyleColorsLight(); // ImGui::StyleColorsDark(); 		// Setup Dear ImGui style

	// Setup Platform/Renderer ImGui backends
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	const auto glsl_version = "#version 410";
	ImGui_ImplOpenGL3_Init(glsl_version);

	#if defined(__WIN64__)
		debugGL(); // Setup callback to catch openGL errors.	V4.2 or newer
	#elif(__APPLE__)
		glCheckError(); // Old way of checking for errors. Newest not implemented by Apple. Manually call function anywhere in code to check for errors.
	#endif

	glfwSwapInterval(1);			 // Ony render when synced (V SYNC) - https://www.tomsguide.com/features/what-is-vsync-and-should-you-turn-it-on-or-off
	glfwWindowHint(GLFW_SAMPLES, 2); // Multisampling - covered in lectures - https://www.khronos.org/opengl/wiki/Multisampling

	startup(); // Setup all necessary information for startup (aka. load texture, shaders, models, etc).

	cout << endl << "Starting main loop and rendering..." << endl;

	do{											 // run until the window is closed
		auto currentTime = (float)glfwGetTime(); // retrieve timelapse
		deltaTime = currentTime - lastTime;		 // Calculate delta time
		lastTime = currentTime;					 // Save for next frame calculations.

		glfwPollEvents(); 						// poll callbacks

		update(); 								// update (physics, animation, structures, etc)
		render(); 								// call render function.
		ui();									// call function to render ui.

		#if defined(__APPLE__)
			glCheckError();				// Manually checking for errors for MacOS, Windows has a callback.
		#endif

		glfwSwapBuffers(window); 		// swap buffers (avoid flickering and tearing)

		running &= (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_RELEASE); // exit if escape key pressed
		running &= (glfwWindowShouldClose(window) != GL_TRUE);
	} while (running);

	endProgram(); // Close and clean everything up...

	// cout << "\nPress any key to continue...\n";
	// cin.ignore(); cin.get(); // delay closing console to read debugging errors.

	return 0;
}


void hintsGLFW(){
	
	auto majorVersion = 3; auto minorVersion = 3; // define OpenGL version - at least 3.3 for bare basic NEW OpenGL

	#if defined(__WIN64__)	
		majorVersion = 4; minorVersion = 5;					// Recommended for Windows 4.5, but latest is 4.6 (not all drivers support 4.6 version).
		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE); // Create context in debug mode - for debug message callback
	#elif(__APPLE__)
		majorVersion = 4; minorVersion = 1; 				// Max support for OpenGL in MacOS
	#endif

	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); 	// https://www.glfw.org/docs/3.3/window_guide.html
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, majorVersion); 
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, minorVersion);
}

void endProgram()
{
	// Clean ImGui
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwMakeContextCurrent(NULL); 	// destroys window handler
	glfwTerminate();				// destroys all windows and releases resources.
}

void addModel(string filename, string name) {
	Content c;
	c.LoadGLTF(filename);
	content[name] = c;
	// modelholder mod;
	// mod.model = c.model;
	// mod.vaoAndEbos = c.vaoAndEbos;

	// models.push_back(mod);

	// vector<Texture> v;
	// for (Texture t : c.model.textures) {
	// 	v.push_back(t);
	// }
	// textures.insert(pair<string, vector<Texture>>(name, v));
	// content.insert(pair<string, Content>(name, c));
	// // textures.push_back(v);
	// // content.push_back(c);

	// vector<vec3> vp;
	// vector<vec3> vo;
	// vector<vec3> vr;
	// vector<vec3> vs;

	// modelPosition.insert(pair<string, vector<vec3>>(name, vp));
	// modelOffset.insert(pair<string, vector<vec3>>(name, vo));
	// modelRotation.insert(pair<string, vector<vec3>>(name, vr));
	// modelScale.insert(pair<string, vector<vec3>>(name, vs));
}

void setPositons(string type, string name, vec3 pos, vec3 off, vec3 rot, vec3 scale, bool useAnim, vec3 vel) {
	object obj;
	obj.type = type;
	obj.name = name;

	obj.position = pos;
	obj.offset = off;
	obj.rotation = rot;
	obj.scale = scale;

	obj.animation = useAnim;
	obj.velocity = vel;

	objects[name].push_back(obj);
	// modelPosition[name].push_back(pos);
	// modelOffset[name].push_back(off);
	// modelRotation[name].push_back(rot);
	// modelScale[name].push_back(scale);
}

void initLights(vec4 v1, vec4 v2, vec4 v3, vec4 v4, vec4 c1, vec4 c2, vec4 c3, vec4 c4) {
	lightPos[0] = v1;
	lightPos[1] = v2;
	lightPos[2] = v3;
	lightPos[3] = v4;

	ia = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

	id[0] = c1;
	id[1] = c2;
	id[2] = c3;
	id[3] = c4;

	is[0] = c1;
	is[1] = c2;
	is[2] = c3;
	is[3] = c4;

}

void startup()
{
	// Output some debugging information
	cout << "VENDOR: " << (char *)glGetString(GL_VENDOR) << endl;
	cout << "VERSION: " << (char *)glGetString(GL_VERSION) << endl;
	cout << "RENDERER: " << (char *)glGetString(GL_RENDERER) << endl;	

	cout << endl << "Loading content..." << endl;

	addModel("assets/Tree_V2.2_Trunk.gltf", "Trunk");
	addModel("assets/Tree_V2.2_Leaves.gltf", "Leaves");
	addModel("assets/dog.gltf", "Dog");
	addModel("assets/C_Belt_V1.1.gltf", "Belt");
	addModel("assets/Plane.gltf", "Plane");
	addModel("assets/Cube.gltf", "Cube");
	addModel("assets/C_C_Belt_V1.gltf", "Corner");

	vec4 l1 = vec4(10.0f, 	10.0f, 		10.0f, 	1.0f);
	vec4 l2 = vec4(0.0f, 	100.0f, 	0.0f, 	1.0f);
	vec4 l3 = vec4(0.0f, 	0.0f, 		0.0f, 	1.0f);
	vec4 l4 = vec4(0.0f, 	0.0f, 		0.0f, 	1.0f);

	vec4 c1 = vec4(1.0f, 	1.0f, 	1.0f, 	1.0f);
	vec4 c2 = vec4(1.0f, 	1.0f, 	1.0f, 	1.0f);
	vec4 c3 = vec4(0.0f, 	0.0f, 	0.0f, 	0.0f);
	vec4 c4 = vec4(0.0f, 	0.0f, 	0.0f, 	0.0f);

	initLights(l1, l2, l3, l4, c1, c2, c3, c4);
	

	// add Trees to world
	int max = 360;
	int min = 	0;

	float randNum = rand() % (max - min + 1) + min;
	setPositons("Trunk", "T1", vec3( 4.0f,  0.0f,  4.0f), 	vec3(0.0f, -0.2f, 0.0f), 		vec3(0.0f, randNum * (M_PIl / 180), 0.0f), 	vec3( 1.0f,  1.0f,  1.0f), false, vec3(0.0f, 0.0f, 0.0f));
	setPositons("Leaves", "L1", vec3( 4.0f,  0.0f,  4.0f), 	vec3(0.0f,  2.4f, 0.0f), 		vec3(0.0f, randNum * (M_PIl / 180), 0.0f), 	vec3( 1.8f,  1.8f,  1.8f), false, vec3(0.0f, 0.0f, 0.0f));

	randNum = rand() % (max - min + 1) + min;
	setPositons("Trunk", "T2", vec3( 4.0f,  0.0f,  0.0f), 	vec3(0.0f, -0.2f, 0.0f), 		vec3(0.0f, randNum * (M_PIl / 180), 0.0f), 	vec3( 1.0f,  1.0f,  1.0f), false, vec3(0.0f, 0.0f, 0.0f));
	setPositons("Leaves", "L2", vec3( 4.0f,  0.0f,  0.0f), 	vec3(0.0f,  2.4f, 0.0f), 		vec3(0.0f, randNum * (M_PIl / 180), 0.0f), 	vec3( 1.8f,  1.8f,  1.8f), false, vec3(0.0f, 0.0f, 0.0f));

	randNum = rand() % (max - min + 1) + min;
	setPositons("Trunk", "T3", vec3( 4.0f,  0.0f, -4.0f), 	vec3(0.0f, -0.2f, 0.0f), 		vec3(0.0f, randNum * (M_PIl / 180), 0.0f), 	vec3( 1.0f,  1.0f,  1.0f), false, vec3(0.0f, 0.0f, 0.0f));
	setPositons("Leaves", "L3", vec3( 4.0f,  0.0f, -4.0f), 	vec3(0.0f,  2.4f, 0.0f), 		vec3(0.0f, randNum * (M_PIl / 180), 0.0f), 	vec3( 1.8f,  1.8f,  1.8f), false, vec3(0.0f, 0.0f, 0.0f));

	randNum = rand() % (max - min + 1) + min;
	setPositons("Trunk", "T4", vec3( 0.0f,  0.0f,  4.0f), 	vec3(0.0f, -0.2f, 0.0f), 		vec3(0.0f, randNum * (M_PIl / 180), 0.0f), 	vec3( 1.0f,  1.0f,  1.0f), false, vec3(0.0f, 0.0f, 0.0f));
	setPositons("Leaves", "L4", vec3( 0.0f,  0.0f,  4.0f), 	vec3(0.0f,  2.4f, 0.0f), 		vec3(0.0f, randNum * (M_PIl / 180), 0.0f), 	vec3( 1.8f,  1.8f,  1.8f), false, vec3(0.0f, 0.0f, 0.0f));

	randNum = rand() % (max - min + 1) + min;
	setPositons("Trunk", "T5", vec3( 0.0f,  0.0f, -4.0f), 	vec3(0.0f, -0.2f, 0.0f), 		vec3(0.0f, randNum * (M_PIl / 180), 0.0f), 	vec3( 1.0f,  1.0f,  1.0f), false, vec3(0.0f, 0.0f, 0.0f));
	setPositons("Leaves", "L5", vec3( 0.0f,  0.0f, -4.0f), 	vec3(0.0f,  2.4f, 0.0f), 		vec3(0.0f, randNum * (M_PIl / 180), 0.0f), 	vec3( 1.8f,  1.8f,  1.8f), false, vec3(0.0f, 0.0f, 0.0f));

	randNum = rand() % (max - min + 1) + min;
	setPositons("Trunk", "T6", vec3(-4.0f,  0.0f,  4.0f), 	vec3(0.0f, -0.2f, 0.0f), 		vec3(0.0f, randNum * (M_PIl / 180), 0.0f), 	vec3( 1.0f,  1.0f,  1.0f), false, vec3(0.0f, 0.0f, 0.0f));
	setPositons("Leaves", "L6", vec3(-4.0f,  0.0f,  4.0f), 	vec3(0.0f,  2.4f, 0.0f), 		vec3(0.0f, randNum * (M_PIl / 180), 0.0f), 	vec3( 1.8f,  1.8f,  1.8f), false, vec3(0.0f, 0.0f, 0.0f));

	randNum = rand() % (max - min + 1) + min;
	setPositons("Trunk", "T7", vec3(-4.0f,  0.0f,  0.0f), 	vec3(0.0f, -0.2f, 0.0f), 		vec3(0.0f, randNum * (M_PIl / 180), 0.0f), 	vec3( 1.0f,  1.0f,  1.0f), false, vec3(0.0f, 0.0f, 0.0f));
	setPositons("Leaves", "L7", vec3(-4.0f,  0.0f,  0.0f), 	vec3(0.0f,  2.4f, 0.0f), 		vec3(0.0f, randNum * (M_PIl / 180), 0.0f), 	vec3( 1.8f,  1.8f,  1.8f), false, vec3(0.0f, 0.0f, 0.0f));

	randNum = rand() % (max - min + 1) + min;
	setPositons("Trunk", "T8", vec3(-4.0f,  0.0f, -4.0f), 	vec3(0.0f, -0.2f, 0.0f), 		vec3(0.0f, randNum * (M_PIl / 180), 0.0f), 	vec3( 1.0f,  1.0f,  1.0f), false, vec3(0.0f, 0.0f, 0.0f));
	setPositons("Leaves", "L8", vec3(-4.0f,  0.0f, -4.0f), 	vec3(0.0f,  2.4f, 0.0f), 		vec3(0.0f, randNum * (M_PIl / 180), 0.0f), 	vec3( 1.8f,  1.8f,  1.8f), false, vec3(0.0f, 0.0f, 0.0f));

	// add Dog to world
	setPositons("Dog", "D1", vec3( 0.55f,  0.5f,  0.55f), 	vec3(0.0f, 0.0f, 0.0f), 		vec3(0.0f, 45 * (M_PIl / 180), 0.0f), 		vec3( 0.1f,  0.1f,  0.1f), false, vec3(0.0f, 0.0f, 0.0f));

	// add belts to world
	setPositons("Belt", "B1", vec3( 1.0f,  0.0f,  0.0f), 	vec3(0.0f, 0.0f, 0.0f), 		vec3(0.0f, 90 * (M_PIl / 180), 0.0f), 		vec3( 1.0f,  1.0f,  1.0f), false, vec3(0.0f, 0.0f, 0.0f));
	setPositons("Belt", "B2", vec3(-1.0f,  0.0f,  0.0f), 	vec3(0.0f, 0.0f, 0.0f), 		vec3(0.0f, 90 * (M_PIl / 180), 0.0f), 		vec3( 1.0f,  1.0f,  1.0f), false, vec3(0.0f, 0.0f, 0.0f));
	setPositons("Belt", "B3", vec3( 0.0f,  0.0f,  1.0f), 	vec3(0.0f, 0.0f, 0.0f), 		vec3(0.0f,  0 * (M_PIl / 180), 0.0f), 		vec3( 1.0f,  1.0f,  1.0f), false, vec3(0.0f, 0.0f, 0.0f));
	setPositons("Belt", "B4", vec3( 0.0f,  0.0f, -1.0f), 	vec3(0.0f, 0.0f, 0.0f), 		vec3(0.0f,  0 * (M_PIl / 180), 0.0f), 		vec3( 1.0f,  1.0f,  1.0f), false, vec3(0.0f, 0.0f, 0.0f));

	setPositons("Corner", "C1", vec3( 1.0f,  0.0f,  1.0f), 	vec3(0.0f, 0.0f, 0.0f), 		vec3(0.0f,  0 * (M_PIl / 180), 0.0f), 		vec3( 1.0f,  1.0f,  1.0f), false, vec3(0.0f, 0.0f, 0.0f));
	setPositons("Corner", "C2", vec3( 1.0f,  0.0f, -1.0f), 	vec3(0.0f, 0.0f, 0.0f), 		vec3(0.0f,  90 * (M_PIl / 180), 0.0f), 		vec3( 1.0f,  1.0f,  1.0f), false, vec3(0.0f, 0.0f, 0.0f));
	setPositons("Corner", "C3", vec3(-1.0f,  0.0f,  1.0f), 	vec3(0.0f, 0.0f, 0.0f), 		vec3(0.0f,  270 * (M_PIl / 180), 0.0f), 	vec3( 1.0f,  1.0f,  1.0f), false, vec3(0.0f, 0.0f, 0.0f));
	setPositons("Corner", "C4", vec3(-1.0f,  0.0f, -1.0f), 	vec3(0.0f, 0.0f, 0.0f), 		vec3(0.0f,  180 * (M_PIl / 180), 0.0f), 	vec3( 1.0f,  1.0f,  1.0f), false, vec3(0.0f, 0.0f, 0.0f));
	
	// add ground to world
	setPositons("Plane", "P1", vec3( 0.0f,  0.0f,  0.0f), 	vec3(0.0f, 0.0f, 0.0f), 		vec3(0.0f, 0.0f, 0.0f), 					vec3(10.0f,  1.0f, 10.0f), false, vec3(0.0f, 0.0f, 0.0f));

	// add cube to world
	setPositons("Cube", "I1", vec3(1.0f, 0.5f, 1.0f),		vec3(0.0f, 0.0f, 0.0f), 		vec3(0.0f, 0.0f, 0.0f), 					vec3( 0.1f,  0.1f,  0.1f), true,  vec3(0.0f, 0.0f, 0.0f));
	
	pipeline.CreatePipeline();
	pipeline.LoadShaders("shaders/vs_model.glsl", "shaders/fs_model.glsl");

	// Start from the centre

	// A few optimizations.
	glFrontFace(GL_CCW);
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	// Get the correct size in pixels - E.g. if retina display or monitor scaling
	glfwGetFramebufferSize(window, &windowWidth, &windowHeight);

	// Calculate proj_matrix for the first time.
	aspect = (float)windowWidth / (float)windowHeight;
	projMatrix = glm::perspective(glm::radians(fovy), aspect, 0.1f, 1000.0f);
}

void animationsCalc() {
	for (const auto& [name, objs] : objects) {
		for (size_t i = 0; i < objs.size(); i++) {
			if (objects[name][i].animation) {
				if (objects[name][i].position.x >= 1.0f && objects[name][i].position.z >= 1.0f)
					objects[name][i].velocity = vec3(-0.01f, 0.0f, 0.0f);
				if (objects[name][i].position.x >= 1.0f && objects[name][i].position.z <= -1.0f)
					objects[name][i].velocity = vec3(0.0f, 0.0f, 0.01f);
				if (objects[name][i].position.x <= -1.0f && objects[name][i].position.z >= 1.0f)
					objects[name][i].velocity = vec3(0.0f, 0.0f, -0.01f);
				if (objects[name][i].position.x <= -1.0f && objects[name][i].position.z <= -1.0f)
					objects[name][i].velocity = vec3(0.01f, 0.0f, 0.0f);
				
				vec3 nPos = objects[name][i].position + objects[name][i].velocity;
				objects[name][i].position = nPos;
			}
		}
	}
}

void update()
{
	float speed = std::max(0.01f, deltaTime) * 2.5;

	animationsCalc();

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	if (keyStatus[GLFW_KEY_TAB]) {
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}

	if (keyStatus[GLFW_KEY_LEFT_SHIFT]) speed *= 2;

	vec3 forward = vec3(cameraFront.x, 0.0f, cameraFront.z);
	if (keyStatus[GLFW_KEY_W]) cameraPosition += speed * forward;
	if (keyStatus[GLFW_KEY_S]) cameraPosition -= speed * forward;

	if (keyStatus[GLFW_KEY_A]) cameraPosition -= glm::normalize(glm::cross(cameraFront, cameraUp)) * speed;
	if (keyStatus[GLFW_KEY_D]) cameraPosition += glm::normalize(glm::cross(cameraFront, cameraUp)) * speed;

	if (keyStatus[GLFW_KEY_SPACE]) cameraPosition += speed * cameraUp;
	if (keyStatus[GLFW_KEY_LEFT_CONTROL]) cameraPosition -= speed * cameraUp;

	glm::mat4 view = glm::lookAt(cameraPosition, cameraPosition + cameraFront, cameraUp);

	if (keyStatus[GLFW_KEY_R]) pipeline.ReloadShaders();

	// Start the Dear ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

void render()
{
	glViewport(0, 0, windowWidth, windowHeight);

	// Clear colour buffer
	glm::vec4 inchyraBlue = glm::vec4(0.345f, 0.404f, 0.408f, 1.0f);
	glm::vec4 backgroundColor = inchyraBlue;
	glClearBufferfv(GL_COLOR, 0, &backgroundColor[0]);

	// Clear deep buffer
	static const GLfloat one = 1.0f;
	glClearBufferfv(GL_DEPTH, 0, &one);

	// Enable blend
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Use our shader programs
	glUseProgram(pipeline.pipe.program);

	// Setup camera
	
	glm::mat4 viewMatrix = glm::lookAt(cameraPosition,				 // eye
									   cameraPosition + cameraFront, // centre
									   cameraUp);					 // up


	// Do some translations, rotations and scaling
	for (const auto& [name, objs] : objects) {
		vector<glm::mat4> modelMatrix;
		// string s[]{"assets/Tree_V2.2_Trunk.gltf", "assets/Tree_V2.2_Leaves.gltf"};
		for (object obj: objs) {

			if (content[obj.type].model.textures.size() > 0) {
				if (content[obj.type].model.textures[0].source > -1) {
					GLuint texid;

					glGenTextures(1, &texid);

					tinygltf::Image &image = content[obj.type].model.images[content[obj.type].model.textures[0].source];

					glBindTexture(GL_TEXTURE_2D, texid);
					glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
					glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
					glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

					GLenum format = GL_RGBA;

					GLenum type = GL_UNSIGNED_BYTE;

					glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.width, image.height, 0, format, type, &image.image.at(0));

					modelMatrix.push_back(glm::translate(glm::mat4(1.0f), glm::vec3(obj.position.x + obj.offset.x, obj.position.y + obj.offset.y, obj.position.z + obj.offset.z)));
					// glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(modelPosition[i].x + modelOffset[i].x, modelPosition[i].y + modelOffset[i].y, modelPosition[i].z + modelOffset[i].z));
					// glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
					modelMatrix[modelMatrix.size() - 1] = glm::rotate(modelMatrix[modelMatrix.size() - 1], obj.rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
					modelMatrix[modelMatrix.size() - 1] = glm::rotate(modelMatrix[modelMatrix.size() - 1], obj.rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
					modelMatrix[modelMatrix.size() - 1] = glm::scale(modelMatrix[modelMatrix.size() - 1], obj.scale);

					glm::mat4 mv_matrix = viewMatrix * modelMatrix[modelMatrix.size() - 1];

					projMatrix = glm::perspective(glm::radians(fovy), aspect, 0.1f, 1000.0f);
					
					glUniformMatrix4fv(glGetUniformLocation(pipeline.pipe.program, "model_matrix"), 1, GL_FALSE, &modelMatrix[modelMatrix.size() - 1][0][0]);
					glUniformMatrix4fv(glGetUniformLocation(pipeline.pipe.program, "view_matrix"), 1, GL_FALSE, &viewMatrix[0][0]);
					glUniformMatrix4fv(glGetUniformLocation(pipeline.pipe.program, "proj_matrix"), 1, GL_FALSE, &projMatrix[0][0]);

					glUniform4f(glGetUniformLocation(pipeline.pipe.program, "viewPosition"), cameraPosition.x, cameraPosition.y, cameraPosition.z, 1.0);
					glUniformMatrix4fv(glGetUniformLocation(pipeline.pipe.program, "lightPosition"), 1, GL_FALSE, &lightPos[0][0]);

					glUniform4f(glGetUniformLocation(pipeline.pipe.program, "ia"), ia.r, ia.g, ia.b, 1.0);
					glUniform1f(glGetUniformLocation(pipeline.pipe.program, "ka"), ka);
					glUniformMatrix4fv(glGetUniformLocation(pipeline.pipe.program, "id"), 1, GL_FALSE, &id[0][0]);
					glUniform1f(glGetUniformLocation(pipeline.pipe.program, "kd"), kd);
					glUniformMatrix4fv(glGetUniformLocation(pipeline.pipe.program, "is"), 1, GL_FALSE, &is[0][0]);
					glUniform1f(glGetUniformLocation(pipeline.pipe.program, "ks"), ks);

					glUniform1f(glGetUniformLocation(pipeline.pipe.program, "shininess"), shininess);

					
					content[obj.type].DrawModel(content[obj.type].vaoAndEbos, content[obj.type].model);
					glFlush();
					
					glDeleteTextures(1, &texid);
				}
			}
		}
	}

		
	#if defined(__APPLE__)
		glCheckError();
	#endif
}

void ui()
{
	ImGuiIO &io = ImGui::GetIO();
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration; 
	window_flags |= ImGuiWindowFlags_AlwaysAutoResize;
	window_flags |= ImGuiWindowFlags_NoSavedSettings; 
	window_flags |= ImGuiWindowFlags_NoFocusOnAppearing; 
	window_flags |= ImGuiWindowFlags_NoNav;

	const auto PAD = 10.0f;
	const ImGuiViewport *viewport = ImGui::GetMainViewport();
	ImVec2 work_pos = viewport->WorkPos; // Use work area to avoid menu-bar/task-bar, if any!
	ImVec2 work_size = viewport->WorkSize;
	ImVec2 window_pos, window_pos_pivot;
	window_pos.x = work_pos.x + work_size.x - PAD;
	window_pos.y = work_pos.y + work_size.y - PAD;
	window_pos_pivot.x = 1.0f;
	window_pos_pivot.y = 1.0f;

	ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
	window_flags |= ImGuiWindowFlags_NoMove;

	ImGui::SetNextWindowBgAlpha(0.35f); // Transparent background
	bool *p_open = NULL;
	if (ImGui::Begin("Info", nullptr, window_flags)) {
		ImGui::Text("About: 3D Graphics and Animation 2022"); // ImGui::Separator();
		ImGui::Text("Performance: %.3fms/Frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::Text("Pipeline: %s", pipeline.pipe.error?"ERROR":"OK");

		ImGui::Text("Movement:\nw : forward\na : left\ns : back\nd : right\nspace : move up\nleft control : move down\nleft shift : increase move speed\n\nmouse : look around and zoom\n\ntab : free move mouse\n");

		ImGui::Text("Camera position: %f, %f, %f", cameraPosition.x, cameraPosition.y, cameraPosition.z);
		
		bool *showLs = &Ls;
		ImGui::Checkbox("Show Lighting scales", showLs);
		if (*showLs) {
			float *a = &ka;
			ImGui::SliderFloat(" : Ambient lighting scale", a, 0.0f, 10.0f, "%.3f", 1.0f);
			float *d = &kd;
			ImGui::SliderFloat(" : Diffuse lighting scale", d, 0.0f, 10.0f, "%.3f", 1.0f);
			float *s = &ks;
			ImGui::SliderFloat(" : Specular lighting scale", s, 0.0f, 10.0f, "%.3f", 1.0f);
		}
		
		bool *showLp = &Lp;
		ImGui::Checkbox("Show Light Positions", showLp);
		if (*showLp) {
			float *p1 = &lightPos[0][0];
			ImGui::SliderFloat4(" : Light 1 Pos", p1, -10.0f, 10.0f, "%.3f", 1.0f);
			float *p2 = &lightPos[1][0];
			ImGui::SliderFloat4(" : Light 2 Pos", p2, -10.0f, 10.0f, "%.3f", 1.0f);
			float *p3 = &lightPos[2][0];
			ImGui::SliderFloat4(" : Light 3 Pos", p3, -10.0f, 10.0f, "%.3f", 1.0f);
			float *p4 = &lightPos[3][0];
			ImGui::SliderFloat4(" : Light 4 Pos", p4, -10.0f, 10.0f, "%.3f", 1.0f);
		}

		
		bool *showLc = &Lc;
		ImGui::Checkbox("Show Light Colours", showLc);
		if (*showLc) {
			float *ca = &ia[0];
			ImGui::SliderFloat4(" : Ambiant Light Colour", ca, 0.0f, 1.0f, "%.3f", 1.0f);
			float *cd1 = &id[0][0];
			ImGui::SliderFloat4(" : Light 1 Colour", cd1, 0.0f, 1.0f, "%.3f", 1.0f);
			float *cd2 = &id[1][0];
			ImGui::SliderFloat4(" : Light 2 Colour", cd2, 0.0f, 1.0f, "%.3f", 1.0f);
			float *cd3 = &id[2][0];
			ImGui::SliderFloat4(" : Light 3 Colour", cd3, 0.0f, 1.0f, "%.3f", 1.0f);
			float *cd4 = &id[3][0];
			ImGui::SliderFloat4(" : Light 4 Colour", cd4, 0.0f, 1.0f, "%.3f", 1.0f);
		}
	}
	ImGui::End();

	// Rendering imgui
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void onResizeCallback(GLFWwindow *window, int w, int h)
{
	windowWidth = w;
	windowHeight = h;

	// Get the correct size in pixels
	glfwGetFramebufferSize(window, &windowWidth, &windowHeight);

	if (windowWidth > 0 && windowHeight > 0)
	{ // Avoid issues when minimising window - it gives size of 0 which fails division.
		aspect = (float)w / (float)h;
		projMatrix = glm::perspective(glm::radians(fovy), aspect, 0.1f, 1000.0f);
	}
}

void onKeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS)
		keyStatus[key] = true;
	else if (action == GLFW_RELEASE)
		keyStatus[key] = false;

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GLFW_TRUE);
}

void onMouseButtonCallback(GLFWwindow *window, int button, int action, int mods)
{
}

void onMouseMoveCallback(GLFWwindow *window, double x, double y)
{
	if (keyStatus[GLFW_KEY_TAB]) return;

	int mouseX = static_cast<int>(x);
	int mouseY = static_cast<int>(y);

	if (firstMouse) {
		lastX = mouseX;
		lastY = mouseY;
		firstMouse = false;
	}

	float xoff = mouseX - lastX;
	float yoff = mouseY - lastY;

	lastX = mouseX;
	lastY = mouseY;
	

	float sens = 0.05f;

	xoff *= sens;
	yoff *= sens;

	yaw += xoff;
	pitch -= yoff;

	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	glm::vec3 front;

	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = sin(glm::radians(pitch));
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	cameraFront = glm::normalize(front);
}

void onMouseWheelCallback(GLFWwindow *window, double xoffset, double yoffset)
{
	if (keyStatus[GLFW_KEY_TAB]) return;

	int yoffsetInt = static_cast<int>(yoffset);

	if (fovy >= 1.0f && fovy <= 45.0f)
		fovy -= yoffset;
	if (fovy <= 1.0f)
		fovy = 1.0f;
	if (fovy >= 45.0f)
		fovy = 45.0f;
}

void APIENTRY openGLDebugCallback(GLenum source,
								  GLenum type,
								  GLuint id,
								  GLenum severity,
								  GLsizei length,
								  const GLchar *message,
								  const GLvoid *userParam)  // Debugger callback for Win64 - OpenGL versions 4.2 or better.
{
	if(id == 131169 || id == 131185 || id == 131218 || id == 131204) return; 
	debugger.OpenGLDebugCallback(source, type, id, severity, length, message, userParam);
}

void debugGL() // Debugger function for Win64 - OpenGL versions 4.2 or better.
{
	// Enable Opengl Debug
	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback((GLDEBUGPROC)openGLDebugCallback, nullptr);
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, true);
}

GLenum glCheckError_(const char *file, int line) // Debugger manual function for Apple - OpenGL versions 3.3 to 4.1.
{
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR) debugger.GlGetError(errorCode, file, line);

	return errorCode;
}
