#include "View.h"

//using namespace std;

View::View()
{
	//TODO: Initialize member variables to 0 or w/e
	this->model = NULL;
	
	this->programID_waveform = 0;
	//WINDOWWORLDCOMMENT this->programID_world = 0;

	this->sampler_2Dfield = 0;
	this->texture_2Dfield = 0;
	this->texture_2Dfielddata = NULL;
	
	this->vertexbuffer_2Dfield = 0;
	this->vertexbuffer_waveform = 0;

	this->vertexbuffer_3Dverts = 0;
	this->vertexbuffer_3Dcolors = 0;
	
	this->vertexbuffer_waveformdata = NULL;

	this->windowWavefield = NULL;
	//WINDOWWORLDCOMMENT this->windowWorld = NULL;

	//WINDOWWORLDCOMMENT Mat4ZeroOut(&this->cameraProjection);
	//WINDOWWORLDCOMMENT Mat4ZeroOut(&this->cameraModelview);
	//WINDOWWORLDCOMMENT this->cameraSpinParam = 0.0f;

	this->initialized = false;
	
	
	
	
	
	//TODO: get this font stuff to work
	//FTGLPixmapFont font("C:/Windows/Fonts/Arial.tff");





}

View::~View()
{
	if (this->model == NULL || this->initialized == false)
	{
#ifdef VERBOSE
		cout << "View was never initialized, so no View cleanup is necessary." << endl;
#endif
	}
	else
	{
#ifdef VERBOSE
		cout << "Performing full View cleanup." << endl;
#endif
		
		if (this->windowWavefield!=NULL)
		{
			glfwDestroyWindow(windowWavefield);
		//WINDOWWORLDCOMMENT glfwDestroyWindow(windowWorld);

			glfwTerminate();
		}
		
		glDeleteBuffers(1, &vertexbuffer_waveform);
		glDeleteBuffers(1, &vertexbuffer_2Dfield);

		glDeleteBuffers(1, &vertexbuffer_3Dverts);
		glDeleteBuffers(1, &vertexbuffer_3Dcolors);

		glDeleteVertexArrays(1, &vao1);
		glDeleteVertexArrays(1, &vao2);

		glDeleteTextures(1, &texture_2Dfield);
		glDeleteSamplers(1, &sampler_2Dfield);

		glDeleteShader(programID_waveform);

		//clear new'd items
		delete[] vertexbuffer_waveformdata;
		delete[] texture_2Dfielddata;

		delete[] this->highlightThese;
	}
}


void View::setModel(FDTD* _model)
{
	if (this->model != NULL)
	{
		//Clean up old stuff
		cout << "Closing old View structures and re-initializing." << endl;
		this->model = NULL;
		this->initialized = false;
		delete[] this->texture_2Dfielddata;
		delete[] this->vertexbuffer_waveformdata;
		delete[] this->highlightThese;
		
		//WINDOWWORLDCOMMENT for (int i = 0; i < this->scene.size(); i++)
		//WINDOWWORLDCOMMENT 	delete this->scene[i];
		//WINDOWWORLDCOMMENT this->scene.clear();
		//cout << "You cannot reuse a View with a new Model at this time. Please delete this View and create a new one." << endl;
		this->model = _model;
		this->initViewAllocateData();
		this->initialized = true;
	}
	else
	{
		if (_model == NULL)
		{
			cout << "You are trying to set a NULL model to this view. This is not allowed." << endl;
		}
		else
		{
			this->model = _model;
			this->initView();
		}
	}
}

//Allocate all view-instance-specific memory here.
//call this when new models are linked
void View::initViewAllocateData(void)
{
	//assume model is good
	
	//Make the texture and waveform buffers first so the simulation can run over RDP without a window
	texture_2Dfielddata = new GLubyte[this->model->iSize*this->model->iSize*4];
#define texture_2DfielddataJSIZE this->model->iSize
#define texture_2DfielddataKSIZE 4
	for (unsigned int i = 0; i < this->model->iSize; i++)
	{
		for (unsigned int j = 0; j < this->model->iSize; j++)
		{
			i3(texture_2Dfielddata,i,j,0) = 128;
			i3(texture_2Dfielddata,i,j,1) = 128;
			i3(texture_2Dfielddata,i,j,2) = 128;
			i3(texture_2Dfielddata,i,j,3) = 255;
		}
	}

	vertexbuffer_waveformdata = new GLfloat[this->model->iSize*2];
#define vertexbuffer_waveformdataJSIZE 2
	for (unsigned int i = 0; i < this->model->iSize; i++)
	{
		float param = ((float)i / ((float)this->model->iSize-1.0f));
		i2(vertexbuffer_waveformdata,i,0) = param;
		i2(vertexbuffer_waveformdata,i,1) = 0.5f+0.5f*(float)sin(param * 2.0f * PI);
		//cout << vertexbuffer_waveformdata[i*2] << endl;
	}


	//Init geometry
	//WINDOWWORLDCOMMENT this->quadForPN = new Mesh();
	//WINDOWWORLDCOMMENT this->quadForPN->color = Vec3GenVec3(.5,0,1);
	//WINDOWWORLDCOMMENT this->quadForPN->pos = Vec3GenVec3(0,0,0);
	//WINDOWWORLDCOMMENT this->quadForPN->verts.push_back(Vec3GenVec3(0,0,this->model->zDim/2.0f));
	//WINDOWWORLDCOMMENT this->quadForPN->verts.push_back(Vec3GenVec3(0,this->model->yDim,this->model->zDim/2.0f));
	//WINDOWWORLDCOMMENT this->quadForPN->verts.push_back(Vec3GenVec3(this->model->xDim,this->model->yDim,this->model->zDim/2.0f));
	//WINDOWWORLDCOMMENT this->quadForPN->verts.push_back(Vec3GenVec3(this->model->xDim,0,this->model->zDim/2.0f));
	//WINDOWWORLDCOMMENT this->quadForPN->uvs.push_back(Vec2GenVec2(0,0));
	//WINDOWWORLDCOMMENT this->quadForPN->uvs.push_back(Vec2GenVec2(0,1));
	//WINDOWWORLDCOMMENT this->quadForPN->uvs.push_back(Vec2GenVec2(1,1));
	//WINDOWWORLDCOMMENT this->quadForPN->uvs.push_back(Vec2GenVec2(1,0));
	//WINDOWWORLDCOMMENT this->scene.push_back(this->quadForPN);


	//Mesh* cube = new Mesh();
	//cube->color = Vec3GenVec3(1,0,0);
	//cube->pos = Vec3GenVec3(0,0,1);
	//cube->verts.push_back(Vec3GenVec3(0,0,0));
	//cube->verts.push_back(Vec3GenVec3(0,1,0));
	//cube->verts.push_back(Vec3GenVec3(1,1,0));
	//cube->verts.push_back(Vec3GenVec3(1,0,0));

	//cube->verts.push_back(Vec3GenVec3(0,0,1));
	//cube->verts.push_back(Vec3GenVec3(0,1,1));
	//cube->verts.push_back(Vec3GenVec3(1,1,1));
	//cube->verts.push_back(Vec3GenVec3(1,0,1));

	//cube->verts.push_back(Vec3GenVec3(0,0,0));
	//cube->verts.push_back(Vec3GenVec3(0,0,1));
	//cube->verts.push_back(Vec3GenVec3(1,0,1));
	//cube->verts.push_back(Vec3GenVec3(1,0,0));

	//cube->verts.push_back(Vec3GenVec3(0,1,0));
	//cube->verts.push_back(Vec3GenVec3(0,1,1));
	//cube->verts.push_back(Vec3GenVec3(1,1,1));
	//cube->verts.push_back(Vec3GenVec3(1,1,0));

	//cube->verts.push_back(Vec3GenVec3(0,0,0));
	//cube->verts.push_back(Vec3GenVec3(0,0,1));
	//cube->verts.push_back(Vec3GenVec3(0,1,1));
	//cube->verts.push_back(Vec3GenVec3(0,1,0));

	//cube->verts.push_back(Vec3GenVec3(1,0,0));
	//cube->verts.push_back(Vec3GenVec3(1,0,1));
	//cube->verts.push_back(Vec3GenVec3(1,1,1));
	//cube->verts.push_back(Vec3GenVec3(1,1,0));

	//this->scene.push_back(cube);

	//debugging highlights
	this->highlightThese = new bool[this->model->iSize * this->model->jSize];
	this->highlightTheseJSIZE = this->model->iSize;
	memset(this->highlightThese,0,this->model->iSize*this->model->jSize*sizeof(bool));
}

//Init windows... once per View
void View::initViewVisuals(void)
{
	//GLFW init was here

	//Create the windows
	//Note that the second window (context) shares the first one's stuff
	//WINDOWWORLDCOMMENT windowWorld = glfwCreateWindow(500,500,"FDTD Simulation World", NULL, NULL);
	//windowWavefield = glfwCreateWindow(500,500,"FDTD Simulation Wavefield", NULL, windowWorld);
	
	
	
	
	
	windowWavefield = glfwCreateWindow(500,500,"FDTD Simulation Wavefield", NULL, NULL);
	//TODO REMOVE THIS
	//windowWavefield = NULL;







	if (!windowWavefield
		//WINDOWWORLDCOMMENT  || !windowWorld
		)
	{
		cout << "Failed to open visualization windowWavefield!" << endl;
		//glfwTerminate();	//not needed
		this->initialized = false;
		return;
	}

	//Keyboard callback initialization was here

//SETUP WINDOW 1
	glfwMakeContextCurrent(windowWavefield);
	glewExperimental = GL_TRUE; 
	if (glewInit() != GLEW_OK)
	{
		cout << "Failed to initialize GLEW successfully!" << endl;
		this->initialized = false;
		return;
	}

	//Set clear color
	glClearColor(0.5f, 0.5f, 0.4f, 0.0f);
	
	//Set program
	programID_waveform = LoadShaders( "2D.vertexshader", "2D.fragmentshader" );

	//Gen Buffers
	glGenBuffers(1, &vertexbuffer_waveform);	
	glGenBuffers(1, &vertexbuffer_2Dfield);
	const GLfloat vertexbuffer_2Dfielddata[] =
	{
		0.0f, 0.0f, 0, 0,
		0.0f,  1.0f, 0, 1,
	 	 1.0f,  1.0f, 1, 1,
	 	 1.0f, 0.0f, 1, 0
	};
#define vertexbuffer_2DfielddataJSIZE 4

	//Upload initial buffer data for waveform and 2d field quad
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_waveform);
	glBufferData(GL_ARRAY_BUFFER, this->model->iSize*2*sizeof(GLfloat), vertexbuffer_waveformdata, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_2Dfield);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertexbuffer_2Dfielddata), vertexbuffer_2Dfielddata, GL_STATIC_DRAW);

	//Texture
	//generate texture
	glGenTextures(1, &texture_2Dfield);
	glBindTexture(GL_TEXTURE_2D, texture_2Dfield);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, this->model->iSize, this->model->iSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture_2Dfielddata);

	//Sampler
	//generate sampler
	glGenSamplers(1,&sampler_2Dfield);
	glBindSampler(0,sampler_2Dfield);
	glSamplerParameteri(sampler_2Dfield, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glSamplerParameteri(sampler_2Dfield, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	//Create vao1 for context1 waveform
	glGenVertexArrays(1, &vao1);

	//Configure VAO1
	glBindVertexArray(vao1);
		
		//TODO: necessary?
		//bind texture
		//glBindTexture(GL_TEXTURE_2D, texture_2Dfield);
		//bind sampler
		//glBindSampler(0,sampler_2Dfield);

		//set attributes
		//bind arraybuffer and enable attrib array

		//TODO: necessary?
		//Bind vbo
		//glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_2Dfield);	//note that this does not become VAO state but rather the next line associates vertexbuffer_2Dfield with the VAO. This is why we can unbind this later (see below, before glBindVertexArray(0))

		glEnableVertexAttribArray(0);
		//set attrib pointer
		glVertexAttribPointer(
		0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
		4,                  // element size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		0,                  // stride
		(void*)0            // array buffer offset
		);
		//unbind array buffer
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		//set texture uniform
			//GLint texLoc = glGetUniformLocation(programID_waveform, "tex");
			//glUniform1i(texLoc, 0);
	//Unbind VAO to protect it
	glBindVertexArray(0);



//WINDOWWORLDCOMMENT 

////SETUP WINDOW 2
//	glfwMakeContextCurrent(windowWorld);
//	glewExperimental = GL_TRUE; 
//	if (glewInit() != GLEW_OK)
//	{
//		cout << "Failed to initialize GLEW successfully!" << endl;
//		return;
//	}
//
//	//Set clear color
//	glClearColor(.9f, .9f, 1.0f, 0.0f);
//
//	//Set program
//	programID_world = LoadShaders( "3D.vertexshader", "3D.fragmentshader" );
//
//	//Gen Buffers
//	glGenBuffers(1, &vertexbuffer_3Dverts);
//	glGenBuffers(1, &vertexbuffer_3Dcolors);
//
//	//Create vao2 for context2 world
//	glGenVertexArrays(1, &vao2);
//	glBindVertexArray(vao2);
//
//		//TODO: is this necessary?
//		//glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_3Dverts);
//		//glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_3Dcolors);
//		
//		//TODO: is this necessary or even useful?
//		//bind texture
//		//glBindTexture(GL_TEXTURE_2D, texture_2Dfield);
//		//bind sampler
//		//glBindSampler(0,sampler_2Dfield);
//
//		glEnableVertexAttribArray(0);
//		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_3Dverts);
//		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)0);
//
//		glEnableVertexAttribArray(1);
//		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_3Dverts);
//		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(3*sizeof(float)));
//
//		glEnableVertexAttribArray(2);
//		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_3Dverts);
//		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(5*sizeof(float)));
//
//		//glEnableVertexAttribArray(2);
//		//glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_3Dcolors);
//		//glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);
//
//	glBindVertexArray(0);


	//glEnable(GL_DEPTH_TEST);
}


//Initialize the view.

void View::initView(void)
{
#ifdef VERBOSE
	cout << "Init visualization..." << endl;
#endif

#ifdef SHOWVIEW
	initialized = true;

	this->initViewVisuals();
#endif

	this->initViewAllocateData();

	this->shouldstop = false;
}

GLuint View::LoadShaders(const char * vertex_file_path,const char * fragment_file_path)
{
	//cout << "In LoadShaders..." << endl;
	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
 
	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if(VertexShaderStream.is_open())
	{
		std::string Line = "";
		while(getline(VertexShaderStream, Line))
			VertexShaderCode += "\n" + Line;
		VertexShaderStream.close();
	}
 
	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if(FragmentShaderStream.is_open()){
		std::string Line = "";
		while(getline(FragmentShaderStream, Line))
			FragmentShaderCode += "\n" + Line;
		FragmentShaderStream.close();
	}
 
	GLint Result = GL_FALSE;
	int InfoLogLength;
 
	// Compile Vertex Shader
	//printf("Compiling shader : %s\n", vertex_file_path);
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
	glCompileShader(VertexShaderID);
 
	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> VertexShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
	if (VertexShaderErrorMessage[0] != '\0')
		fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);
 
	// Compile Fragment Shader
	//printf("Compiling shader : %s\n", fragment_file_path);
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
	glCompileShader(FragmentShaderID);
 
	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
	if (FragmentShaderErrorMessage[0] != '\0')
		fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);
 
	// Link the program
	//fprintf(stdout, "Linking program\n");
	GLuint programID = glCreateProgram();
	glAttachShader(programID, VertexShaderID);
	glAttachShader(programID, FragmentShaderID);
	//glBindFragDataLocation(programID_waveform, 1, "fragmentcolor");
	glLinkProgram(programID);
	//cout << "fragmentcolor location: " << glGetFragDataLocation(programID_waveform, "fragmentcolor") << endl;
 
	// Check the program
	glGetProgramiv(programID, GL_LINK_STATUS, &Result);
	glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> ProgramErrorMessage( max(InfoLogLength, int(1)) );
	glGetProgramInfoLog(programID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
	if (ProgramErrorMessage[0] != '\0')
		fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);
 
	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);
 
	return programID;
}



void View::updateView()
{
	//return;

	if (this->model == NULL)
	{
		cout << "This View's Model is not configured. Make sure you call ->setModel(__) before using a View." << endl;
		return;
	}

	if (initialized == false)
		return;

	const float CONTRAST = 0.2f;

	unsigned int i, j, k;
	//Loop over pn, generating new view data (image texture and waveform points)
	//TODO: change to i1 i2 j1 j2 stuff here ????

	for (i = 0; i < this->model->iSize; i++)
	{
		for (j = 0; j < this->model->jSize; j++)
		{
			//TODO: Remember about this. You might want it to be mick later.
			k = this->model->mick;//kSize/2;
			//Calculate a good value for the i,j pixel colors based on pn
			//greyscale
			float thispn = i3(this->model->pn,i,j,k);
			float candidate_value = 255.0f * (CONTRAST * thispn + 0.5f);
			unsigned char r = (unsigned char)(max(min(candidate_value,255.0f), 0.0f));

			//r = 255;

			//Update the RGB color channels of the texture data
			i3(texture_2Dfielddata,i,j,0) = r;
			i3(texture_2Dfielddata,i,j,1) = r;
			i3(texture_2Dfielddata,i,j,2) = r;
			//leave 4th channel, alpha, alone
		}

		//Fix j and k for reading off this waveform data
		//TODO: Remember about this. You might want it to be micj/mick later.
		//j = this->model->jSize/2;
		//k = this->model->kSize/2;
		//Fill waveform buffer with new vertical positions based on pn
		//i2(vertexbuffer_waveformdata,i,1) = i3(this->model->pn,i,j,k) / 100.0f + 0.5f;
	}
	//i3(texture_2Dfielddata,this->model->iSize-1,this->model->jSize-1,0)=255;
	//i3(texture_2Dfielddata,this->model->iSize-1,this->model->jSize-1,1)=255;
	//i3(texture_2Dfielddata,this->model->iSize-1,this->model->jSize-1,2)=255;

	const GLubyte PMLtickshade = 0;

	i3(texture_2Dfielddata,this->model->na-1,0,0)=PMLtickshade;
	i3(texture_2Dfielddata,this->model->na-1,0,1)=PMLtickshade;
	i3(texture_2Dfielddata,this->model->na-1,0,2)=PMLtickshade;

	i3(texture_2Dfielddata,this->model->iSize-this->model->na,0,0)=PMLtickshade;
	i3(texture_2Dfielddata,this->model->iSize-this->model->na,0,1)=PMLtickshade;
	i3(texture_2Dfielddata,this->model->iSize-this->model->na,0,2)=PMLtickshade;

	i3(texture_2Dfielddata,0,this->model->na-1,0)=PMLtickshade;
	i3(texture_2Dfielddata,0,this->model->na-1,1)=PMLtickshade;
	i3(texture_2Dfielddata,0,this->model->na-1,2)=PMLtickshade;

	i3(texture_2Dfielddata,0,this->model->jSize-this->model->na,0)=PMLtickshade;
	i3(texture_2Dfielddata,0,this->model->jSize-this->model->na,1)=PMLtickshade;
	i3(texture_2Dfielddata,0,this->model->jSize-this->model->na,2)=PMLtickshade;

	i3(texture_2Dfielddata,this->model->mici,this->model->micj,0) = 0;
	i3(texture_2Dfielddata,this->model->mici,this->model->micj,1) = 0;
	i3(texture_2Dfielddata,this->model->mici,this->model->micj,2) = 255;

	i3(texture_2Dfielddata,this->model->sourcei,this->model->sourcej,0) = 0;
	i3(texture_2Dfielddata,this->model->sourcei,this->model->sourcej,1) = 255;
	i3(texture_2Dfielddata,this->model->sourcei,this->model->sourcej,2) = 0;

	//Highlighting
	for (i = 0; i < this->model->iSize; i++)
	{
		for (j = 0; j < this->model->jSize; j++)
		{
			//OBSTACLES
			if ((i3(this->model->obstaclemap,i,j,this->model->mick)&1u) == 0)
			{
				i3(texture_2Dfielddata, i, j, 0) = min(i3(texture_2Dfielddata, i, j, 0)+50,255);
				i3(texture_2Dfielddata, i, j, 0) = 255;
				i3(texture_2Dfielddata, i, j, 1) = 0;
				i3(texture_2Dfielddata, i, j, 2) = 0;
			}

			//ABSORBERS
			//if (i3(this->model->absorbmap,i,j,this->model->mick) > 0)
			//{
			//	i3(texture_2Dfielddata, i, j, 1) = min(i3(texture_2Dfielddata, i, j, 1)+50,255);
			//}

			//HIGHLIGHTED CELLS
			if (i2(highlightThese,i,j))
			{
				i3(texture_2Dfielddata, i, j, 2) = min(i3(texture_2Dfielddata, i, j, 2)+50,255);
				//i3(texture_2Dfielddata, i, j, 2) = 255;
				//i3(texture_2Dfielddata, i, j, 1) = 0;
				//i3(texture_2Dfielddata, i, j, 0) = 0;
			}

			//preview image (note that it does not scale the image)
			//i3(texture_2Dfielddata, i, j, 0) = i3(this->model->obstaclemap_heightmap,i,j,0);
			//i3(texture_2Dfielddata, i, j, 1) = i3(this->model->obstaclemap_heightmap,i,j,1);
			//i3(texture_2Dfielddata, i, j, 2) = i3(this->model->obstaclemap_heightmap,i,j,2);

			if (!((i3(this->model->obstaclemap,i,j,this->model->mick)&1u) == 0) && !(i2(highlightThese,i,j))
				&& (i!=this->model->na && i!=this->model->iSize-this->model->na && j!=this->model->na && j!=this->model->jSize-this->model->na && k!=this->model->na && k!=this->model->kSize-this->model->na)
				&& !(i==this->model->mici && j==this->model->micj)
				&& !(i==this->model->sourcei && j==this->model->sourcej)
				)
			{
				i3(texture_2Dfielddata, i, j, 0) = 255;
				i3(texture_2Dfielddata, i, j, 1) = 255;
				i3(texture_2Dfielddata, i, j, 2) = 255;
			}
		}
	}







	//Update Wavefield window

	glfwMakeContextCurrent(windowWavefield);
		glClear(GL_COLOR_BUFFER_BIT);

		//upload new waveform to gpu
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_waveform);
		glBufferData(GL_ARRAY_BUFFER, this->model->iSize*2*sizeof(GLfloat), vertexbuffer_waveformdata, GL_DYNAMIC_DRAW);

		//upload new texture to gpu
		glBindTexture(GL_TEXTURE_2D, texture_2Dfield);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, this->model->iSize, this->model->jSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture_2Dfielddata);
		glGenerateMipmap(GL_TEXTURE_2D);

		//Use the appropriate shader program
		glUseProgram(programID_waveform);

		//Set the appropriate glUniforms
		GLint texLoc = glGetUniformLocation(programID_waveform, "tex");
		glUniform1i(texLoc, 0);

		//activate the appropriate texture
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture_2Dfield);
		glBindSampler(0, sampler_2Dfield);

		//Bind the VAO with the correct schematic for this VBO	
		glBindVertexArray(vao1);
			//Draw the array
			glDrawArrays(GL_QUADS, 0, 4);
		//unbind vao1 to protect it.
		glBindVertexArray(0);

		//glDisableVertexAttribArray(0);

	//Swap backbuffer
	glfwSwapBuffers(windowWavefield);









////WINDOWWORLDCOMMENT 
//
//	//Update World Window
//
//	//Update camera
//	this->cameraSpinParam += 0.01f;
//	//this->camera = Mat4Mat4Multiply(
//		//Mat4GenPerspectiveProjection(90.0f, 1.0f, 0.01f, 100.0f),
//		//Mat4GenLookAtTransform( Vec3GenVec3(10.0f * sin(this->cameraSpinParam),10,10), Vec3GenVec3(0,0,0), Vec3GenVec3(0,0,1) )
//		//);
//	//this->camera = Mat4GenTranslate(.5,.5,1);
//	this->cameraProjection = Mat4GenPerspectiveProjection(30.0f, 1.0f, 0.5f, 1000.0f);
//
//		//Mat4GenTranslate(-1.0f,-1.0f,-1+2.0f*sin(this->cameraSpinParam))
//		
//		//Mat4GenLookAtTransform( Vec3GenVec3(-10,-10,-10), Vec3GenVec3(0,0,0), Vec3GenVec3(0,0,1) )
//
//	this->cameraModelview =
//		//Mat4GenTranslate(-0.5f,-0.5f,-3+sin(this->cameraSpinParam));
//
//	//Mat4GenTranslate(sin(this->cameraSpinParam),cos(this->cameraSpinParam),-3+sin(this->cameraSpinParam));
//
//		Mat4GenLookAtTransform(
//			Vec3GenVec3(0.5f+3.0f*sin(this->cameraSpinParam),0.5f+3.0f*cos(this->cameraSpinParam),0.5f+3.0f),
//			Vec3GenVec3(0.5f,0.5f,0.5f),
//			Vec3GenVec3(0,0,1)
//			);
//
//		//Mat4GenLookAtTransform( Vec3GenVec3(10.0f * sin(this->cameraSpinParam),10.0f * sin(4.0f * this->cameraSpinParam),10), Vec3GenVec3(0,0,0), Vec3GenVec3(0,1,0) )
//
//	glfwMakeContextCurrent(windowWorld);
//		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//
//		//Combine scene objects
//		vector<Vec3> vertbuffer;
//		vector<Vec2> uvbuffer;
//		vector<Vec3> colorbuffer;
//		for (vector<Mesh*>::iterator i = this->scene.begin(); i < scene.end(); i++)
//		{
//			for (unsigned int j = 0; j < (*i)->verts.size(); j++)
//			{
//				//If this mesh has valid UVs, use them. Otherwise, use 0s and keep the same
//				//vert buffer format. alternatively, add another VAO format... but no thanx
//				if ( (*i)->uvs.size() == (*i)->verts.size() )
//				{
//					uvbuffer.push_back( (*i)->uvs[j] );
//				}
//				else
//				{
//					Vec2 zerouv; zerouv.u = 0.0f; zerouv.v = 0.0f;
//					uvbuffer.push_back( zerouv );
//				}
//				vertbuffer.push_back( Vec3Add((*i)->verts[j], (*i)->pos) );
//				colorbuffer.push_back( (*i)->color );
//			}
//		}
//		float* tempvertdata = new float[vertbuffer.size() * 8];	//3 pos 2 uv 3 color
//		for (unsigned int i = 0; i < vertbuffer.size(); i++)
//		{
//			tempvertdata[i*8 + 0] = vertbuffer[i].x;
//			tempvertdata[i*8 + 1] = vertbuffer[i].y;
//			tempvertdata[i*8 + 2] = vertbuffer[i].z;
//			tempvertdata[i*8 + 3] = uvbuffer[i].u;
//			tempvertdata[i*8 + 4] = uvbuffer[i].v;
//			tempvertdata[i*8 + 5] = colorbuffer[i].x;
//			tempvertdata[i*8 + 6] = colorbuffer[i].y;
//			tempvertdata[i*8 + 7] = colorbuffer[i].z;
//			//for (int j = 0; j < 8; j++)
//			//	cout << tempvertdata[i*8 + j] << "  ";
//			//cout << endl;
//		}
////		cout << endl;
//
//		//float* tempcolors = new float[colorbuffer.size() * 3];
//		//for (unsigned int i = 0; i < colorbuffer.size(); i++)
//		//{
//			//tempcolors[i*3 + 0] = colorbuffer[i].x;
//			//tempcolors[i*3 + 1] = colorbuffer[i].y;
//			//tempcolors[i*3 + 2] = colorbuffer[i].z;
//		//}
//		
//		//upload buffer data
//		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_3Dverts);
//		glBufferData(GL_ARRAY_BUFFER, vertbuffer.size()*8*sizeof(float), tempvertdata, GL_DYNAMIC_DRAW);
//		//glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_3Dcolors);
//		//glBufferData(GL_ARRAY_BUFFER, colorbuffer.size()*3*sizeof(float), tempcolors, GL_DYNAMIC_DRAW);
//
//		//delete temporary arrays
//		delete[] tempvertdata;
//		//delete[] tempcolors;
//
//		//Use the appropriate shader program
//		glUseProgram(this->programID_world);
//
//		//Set the appropriate glUniforms for texture
//		GLint texLoc2 = glGetUniformLocation(programID_world, "tex");
//		glUniform1i(texLoc2, 0);
//
//		//activate the appropriate texture
//		glActiveTexture(GL_TEXTURE0);
//		glBindTexture(GL_TEXTURE_2D, texture_2Dfield);
//		glBindSampler(0, sampler_2Dfield);
//
//		//Set uniforms
//		//GLfloat transform[16];
//		//for (int i = 0; i < 4; i++)
//			//for (int j = 0; j < 4; j++)
//		//		transform[i*4+j] = this->camera.mat[i][j];
//		GLint projectionLoc = glGetUniformLocation(programID_world, "projection");
//		GLint modelviewLoc = glGetUniformLocation(programID_world, "modelview");
//		glUniformMatrix4fv(projectionLoc, 1, GL_TRUE, &this->cameraProjection.mat[0][0]);
//		glUniformMatrix4fv(modelviewLoc, 1, GL_FALSE, &this->cameraModelview.mat[0][0]);
//		
//		//draw buffers
//		glBindVertexArray(vao2);
//		glDrawArrays(GL_QUADS, 0, static_cast<GLsizei>(vertbuffer.size()) );
//		glBindVertexArray(0);
//
//	//Swap backbuffer
//	glfwSwapBuffers(windowWorld);
}