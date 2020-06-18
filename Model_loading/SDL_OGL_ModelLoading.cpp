
#include <GL\glew.h>

#include <SDL.h>
#include <SDL_opengl.h>
#include <stdio.h>
#include <gl\GLU.h>

#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>

#include <iostream>

#include "Shader.h"
#include "Camera.h"
#include "Model.h"

bool init();
bool initGL();
void render();
void close();

//The window we'll be rendering to
SDL_Window* gWindow = NULL;

//OpenGL context
SDL_GLContext gContext;

Shader gShader;
Model gModel;
Model cModel;
Model eModel;
Model bModel;



// camera
Camera camera(glm::vec3(0.0f, 6.0f, 13.0f));
float lastX = -1;
float lastY = -1;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;
float Speed = 0;

// lighting
glm::vec3 lightPos(1.2f, 1.0f, 2.0f);

//event handlers
void HandleKeyDown(const SDL_KeyboardEvent& key);
void HandleMouseMotion(const SDL_MouseMotionEvent& motion);
void HandleMouseWheel(const SDL_MouseWheelEvent& wheel);

int main(int argc, char* args[])
{
	init();

	SDL_Event e;
	//While application is running
	bool quit = false;
	while (!quit)
	{
		// per-frame time logic
		// --------------------
		float currentFrame = SDL_GetTicks() / 1000.0f;
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		//Handle events on queue
		while (SDL_PollEvent(&e) != 0)
		{
			//User requests quit
			if (e.type == SDL_QUIT)
			{
				quit = true;
			}
			switch (e.type)
			{
			case SDL_QUIT:
				quit = true;
				break;
			case SDL_KEYDOWN:
				if (e.key.keysym.sym == SDLK_ESCAPE)
				{
					quit = true;
				}
				else
				{
					HandleKeyDown(e.key);
				}
				break;
			case SDL_MOUSEMOTION:
				HandleMouseMotion(e.motion);
				break;
			case SDL_MOUSEWHEEL:
				HandleMouseWheel(e.wheel);
				break;
			}
		}

		//Render
		render();

		//Update screen
		SDL_GL_SwapWindow(gWindow);
	}

	close();

	return 0;
}

void HandleKeyDown(const SDL_KeyboardEvent& key)
{
	switch (key.keysym.sym)
	{
	case SDLK_w:
		camera.ProcessKeyboard(FORWARD, deltaTime);
		break;
	case SDLK_s:
		camera.ProcessKeyboard(BACKWARD, deltaTime);
		break;
	case SDLK_a:
		camera.ProcessKeyboard(LEFT, deltaTime);
		break;
	case SDLK_d:
		camera.ProcessKeyboard(RIGHT, deltaTime);
		break;
	}
}

void HandleMouseMotion(const SDL_MouseMotionEvent& motion)
{
	if (firstMouse)
	{
		lastX = motion.x;
		lastY = motion.y;
		firstMouse = false;
	}
	else
	{
		camera.ProcessMouseMovement(motion.x - lastX, lastY - motion.y);
		lastX = motion.x;
		lastY = motion.y;
	}
}

void HandleMouseWheel(const SDL_MouseWheelEvent& wheel)
{
	camera.ProcessMouseScroll(wheel.y);
}
bool init()
{
	//Initialization flag
	bool success = true;

	//Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
		success = false;
	}
	else
	{
		//Use OpenGL 3.3
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

		//Create window
		gWindow = SDL_CreateWindow("GTA VI", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1024, 768,
			SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN /*| SDL_WINDOW_FULLSCREEN*/);
		if (gWindow == NULL)
		{
			printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
			success = false;
		}
		else
		{
			//Create context
			gContext = SDL_GL_CreateContext(gWindow);
			if (gContext == NULL)
			{
				printf("OpenGL context could not be created! SDL Error: %s\n", SDL_GetError());
				success = false;
			}
			else
			{
				//Use Vsync
				if (SDL_GL_SetSwapInterval(1) < 0)
				{
					printf("Warning: Unable to set VSync! SDL Error: %s\n", SDL_GetError());
				}
				//Initialize OpenGL
				if (!initGL())
				{
					printf("Unable to initialize OpenGL!\n");
					success = false;
				}
			}
		}
	}

	return success;
}

bool initGL()
{
	bool success = true;
	GLenum error = GL_NO_ERROR;

	glewInit();

	error = glGetError();
	if (error != GL_NO_ERROR)
	{
		success = false;
		printf("Error initializing OpenGL! %s\n", gluErrorString(error));
	}

	glClearColor(0.0f, 0.5f, 0.6f, 1.0f);
	glEnable(GL_DEPTH_TEST);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	gShader.Load("./shaders/vertex.vert", "./shaders/fragment.frag");

	gModel.LoadModel("./models/car/car.obj");
	cModel.LoadModel("./models/copcar/untitled.obj");
	eModel.LoadModel("./models/env/env.obj");
	bModel.LoadModel("./models/bg/bg.obj");

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); //other modes GL_FILL, GL_POINT

	return success;
}

void close()
{
	//delete GL programs, buffers and objects
	glDeleteProgram(gShader.ID);
	//glDeleteVertexArrays(1, &gVAO);
	//glDeleteBuffers(1, &gVBO);

	//Delete OGL context
	SDL_GL_DeleteContext(gContext);
	//Destroy window	
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;

	//Quit SDL subsystems
	SDL_Quit();
}

void render()
{
	//Clear color buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	Speed += deltaTime*4;
	if (Speed > 70.0f)Speed = 0; //reset cars to starting position

	//  car
	glm::mat4 car = glm::mat4(1.0f);
	car = glm::rotate(car, glm::radians(0.0f), glm::vec3(0, 1, 0));
	car = glm::translate(car, glm::vec3(40.0f, 0.0f, -0.3f)); //initial position
	car = glm::translate(car, glm::vec3(-Speed, -1.0f, 0.0f)); // moving position
	car = glm::scale(car, glm::vec3(0.4f, 0.4f, 0.4f));

	gModel.Draw(gShader);

	// copcar

	glm::mat4 copcar = glm::mat4(1.0f);
	copcar = glm::rotate(copcar, glm::radians(0.0f), glm::vec3(0, 1, 0));
	copcar = glm::translate(copcar, glm::vec3(43.0f, -1.0f, 0.2f)); // Initial position

	copcar = glm::translate(copcar, glm::vec3(-Speed, 0.0f, 0.0f)); // change position per second

	copcar = glm::scale(copcar, glm::vec3(0.4f, 0.4f, 0.4f));
	gShader.setMat4("model", copcar);

	cModel.Draw(gShader);

	// env

	glm::mat4 env = glm::mat4(1.0f);
	env = glm::rotate(env, glm::radians(-270.0f), glm::vec3(0, 1, 0));
	env = glm::translate(env, glm::vec3(0.0f, -1.0f, 0.0f));
	env = glm::scale(env, glm::vec3(0.25f, 0.25f, 0.25f));

	gShader.setMat4("model", env);

	eModel.Draw(gShader);

	//env copy 
	glm::mat4 envc = glm::mat4(1.0f);
	envc = glm::rotate(envc, glm::radians(-270.0f), glm::vec3(0, 1, 0));
	envc = glm::translate(envc, glm::vec3(0.0f, -1.0f, 23.5f));
	envc = glm::scale(envc, glm::vec3(0.25f, 0.25f, 0.25f));

	gShader.setMat4("model", envc);

	eModel.Draw(gShader);
	//env copy 2
	glm::mat4 envct = glm::mat4(1.0f);
	envct = glm::rotate(envct, glm::radians(-270.0f), glm::vec3(0, 1, 0));
	envct = glm::translate(envct, glm::vec3(0.0f, -1.0f, -23.0f));
	envct = glm::scale(envct, glm::vec3(0.25f, 0.25f, 0.25f));

	gShader.setMat4("model", envct);

	eModel.Draw(gShader);
	//env copy 3
	glm::mat4 envctz = glm::mat4(1.0f);
	envctz = glm::rotate(envctz, glm::radians(-270.0f), glm::vec3(0, 1, 0));
	envctz = glm::translate(envctz, glm::vec3(23.0f, -1.0f, 0.0f));
	envctz = glm::scale(envctz, glm::vec3(0.25f, 0.25f, 0.25f));

	gShader.setMat4("model", envctz);

	eModel.Draw(gShader);

	//background mountains
	glm::mat4 bg = glm::mat4(1.0f);
	bg = glm::rotate(bg, glm::radians(-270.0f), glm::vec3(0, 1, 0));
	bg = glm::translate(bg, glm::vec3(20.0f, -4.0f, 0.0f));
	bg = glm::scale(bg, glm::vec3(1.5f, 1.5f, 1.5f));

	gShader.setMat4("model", bg);

	bModel.Draw(gShader);






	//lighting
	gShader.setVec3("light.diffuse", 1.0f, 1.0f, 1.0f);
	gShader.setVec3("light.position", lightPos);
	gShader.setVec3("viewPos", camera.Position);





	glm::mat4 view = camera.GetViewMatrix();
	glm::mat4 proj = glm::perspective(glm::radians(camera.Zoom), 4.0f / 3.0f, 0.1f, 100.0f);
	glm::mat3 normalMat = glm::transpose(glm::inverse(car));

	glUseProgram(gShader.ID);
	gShader.setMat4("model", car);
	gShader.setMat4("view", view);
	gShader.setMat4("proj", proj);
	gShader.setMat3("normalMat", normalMat);
}



