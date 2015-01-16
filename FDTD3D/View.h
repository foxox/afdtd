#ifndef VIEWH
#define VIEWH

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <ftgl/ftgl.h>

#include <vector>
#include <math.h>
#include <iostream>
#include <string>
#include <fstream>

#include "common.h"

#include "foxmath3.h"
using namespace FM;

#include "Mesh.h"


//WINDOWWORLDCOMMENT#define SHOWVIEW

class FDTD;

class View
{
protected:
	void initView(void);
	GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path);

	FDTD* model;

	void initViewAllocateData(void);
	void initViewVisuals(void);

public:

	bool initialized;	//was private. should probably be private

	View(void);
	virtual ~View(void);

	//Windows
	GLFWwindow* windowWavefield;
	//WINDOWWORLDCOMMENT GLFWwindow* windowWorld;

	//VAOs
	GLuint vao1;
	GLuint vao2;

	//Shaders
	GLuint programID_waveform;
	//WINDOWWORLDCOMMENT GLuint programID_world;

	//Buffers
	GLuint vertexbuffer_waveform;
	GLuint vertexbuffer_2Dfield;
	//for 3d view
	GLuint vertexbuffer_3Dverts;
	GLuint vertexbuffer_3Dcolors;

	//Data buffers
	GLfloat* vertexbuffer_waveformdata;
	GLubyte* texture_2Dfielddata;

	//Texture
	GLuint texture_2Dfield;
	//Sampler
	GLuint sampler_2Dfield;


	//3D WORLD STUFF
	//WINDOWWORLDCOMMENT Mat4 cameraProjection;
	//WINDOWWORLDCOMMENT Mat4 cameraModelview;
	//WINDOWWORLDCOMMENT float cameraSpinParam;
	//WINDOWWORLDCOMMENT vector<Mesh*> scene;
	//WINDOWWORLDCOMMENT Mesh* quadForPN;


	//Debugging
	bool* highlightThese;
	size_t highlightTheseJSIZE;
	

	//view-free stopping
	bool shouldstop;

	void setModel(FDTD* _model);

	virtual void updateView();
};

//Inlined Methods:
#include "FDTD.h"

#endif