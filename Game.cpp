/* 
OpenGL Template for INM376 / IN3005
City University London, School of Mathematics, Computer Science and Engineering
Source code drawn from a number of sources and examples, including contributions from
 - Ben Humphrey (gametutorials.com), Michal Bubner (mbsoftworks.sk), Christophe Riccio (glm.g-truc.net)
 - Christy Quinn, Sam Kellett and others

 For educational use by Department of Computer Science, City University London UK.

 This template contains a skybox, simple terrain, camera, lighting, shaders, texturing

 Potential ways to modify the code:  Add new geometry types, shaders, change the terrain, load new meshes, change the lighting, 
 different camera controls, different shaders, etc.
 
 Template version 5.0a 29/01/2017
 Dr Greg Slabaugh (gregory.slabaugh.1@city.ac.uk) 

 version 6.0a 29/01/2019
 Dr Eddie Edwards (Philip.Edwards@city.ac.uk)
*/


#include "game.h"


// Setup includes
#include "HighResolutionTimer.h"
#include "GameWindow.h"
#include "CatmullRom.h"

// Game includes
#include "CatmullRom.h"
#include "Camera.h"
#include "Skybox.h"
#include "Plane.h"
#include "Shaders.h"
#include "FreeTypeFont.h"
#include "Sphere.h"
#include "MatrixStack.h"
#include "OpenAssetImportMesh.h"
#include "Audio.h"
#include "SoundSource.h"

// Constructor
Game::Game()
{
	m_pPath = NULL;
	m_pSkybox = NULL;
	m_pCamera = NULL;
	m_pShaderPrograms = NULL;
	m_pPlanarTerrain = NULL;
	m_pFtFont = NULL;
	m_pBarrelMesh = NULL;
	m_pHorseMesh = NULL;
	m_pSphere = NULL;
	m_pModule = NULL;
	m_pSubmarine = NULL;
	m_pHighResolutionTimer = NULL;
	m_pAudio = NULL;
	m_pSoundSource = NULL;
	m_pSubmarineSoundSource = NULL;

	m_dt = 0.0;
	m_framesPerSecond = 0;
	m_frameCount = 0;
	m_elapsedTime = 0.0f;
	m_currentDistance = 0.0f;
	m_cameraSpeed = 0.01f;
	m_cameraRotation = 0.0f;
	m_filterControl = 0.0f;
	m_t = 0.0f;
	m_submarinePosition = glm::vec3(0.0f, 0.0f, 0.0f);
	m_submarineOrientation = glm::mat4(1.0f);
	m_submarineVel = 0.0001f;
}

// Destructor
Game::~Game() 
{ 
	//game objects
	delete m_pPath;
	delete m_pCamera;
	delete m_pSkybox;
	delete m_pPlanarTerrain;
	delete m_pFtFont;
	delete m_pBarrelMesh;
	delete m_pHorseMesh;
	delete m_pSphere;
	delete m_pModule;
	delete m_pSubmarine;
	delete m_pAudio;
	delete m_pSoundSource;
	delete m_pSubmarineSoundSource;

	if (m_pShaderPrograms != NULL) {
		for (unsigned int i = 0; i < m_pShaderPrograms->size(); i++)
			delete (*m_pShaderPrograms)[i];
	}
	delete m_pShaderPrograms;

	//setup objects
	delete m_pHighResolutionTimer;
}

// Initialisation:  This method only runs once at startup
void Game::Initialise() 
{
	// Set the clear colour and depth
	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClearDepth(1.0f);

	/// Create objects
	m_pPath = new CCatmullRom;
	m_pCamera = new CCamera;
	m_pSkybox = new CSkybox;
	m_pShaderPrograms = new vector <CShaderProgram *>;
	m_pPlanarTerrain = new CPlane;
	m_pFtFont = new CFreeTypeFont;
	m_pBarrelMesh = new COpenAssetImportMesh;
	m_pHorseMesh = new COpenAssetImportMesh;
	m_pSphere = new CSphere;
	m_pModule = new COpenAssetImportMesh;
	m_pSubmarine = new COpenAssetImportMesh;
	m_pAudio = new CAudio;
	m_pSoundSource = new CSoundSource;
	m_pSubmarineSoundSource = new CSoundSource;

	RECT dimensions = m_gameWindow.GetDimensions();

	int width = dimensions.right - dimensions.left;
	int height = dimensions.bottom - dimensions.top;

	// Set the orthographic and perspective projection matrices based on the image size
	m_pCamera->SetOrthographicProjectionMatrix(width, height); 
	m_pCamera->SetPerspectiveProjectionMatrix(45.0f, (float) width / (float) height, 0.5f, 5000.0f);

	// Load shaders
	vector<CShader> shShaders;
	vector<string> sShaderFileNames;
	sShaderFileNames.push_back("mainShader.vert");
	sShaderFileNames.push_back("mainShader.frag");
	sShaderFileNames.push_back("textShader.vert");
	sShaderFileNames.push_back("textShader.frag");

	for (int i = 0; i < (int) sShaderFileNames.size(); i++) {
		string sExt = sShaderFileNames[i].substr((int) sShaderFileNames[i].size()-4, 4);
		int iShaderType;
		if (sExt == "vert") iShaderType = GL_VERTEX_SHADER;
		else if (sExt == "frag") iShaderType = GL_FRAGMENT_SHADER;
		else if (sExt == "geom") iShaderType = GL_GEOMETRY_SHADER;
		else if (sExt == "tcnl") iShaderType = GL_TESS_CONTROL_SHADER;
		else iShaderType = GL_TESS_EVALUATION_SHADER;
		CShader shader;
		shader.LoadShader("resources\\shaders\\"+sShaderFileNames[i], iShaderType);
		shShaders.push_back(shader);
	}

	// Create the main shader program
	CShaderProgram *pMainProgram = new CShaderProgram;
	pMainProgram->CreateProgram();
	pMainProgram->AddShaderToProgram(&shShaders[0]);
	pMainProgram->AddShaderToProgram(&shShaders[1]);
	pMainProgram->LinkProgram();
	m_pShaderPrograms->push_back(pMainProgram);

	// Create a shader program for fonts
	CShaderProgram *pFontProgram = new CShaderProgram;
	pFontProgram->CreateProgram();
	pFontProgram->AddShaderToProgram(&shShaders[2]);
	pFontProgram->AddShaderToProgram(&shShaders[3]);
	pFontProgram->LinkProgram();
	m_pShaderPrograms->push_back(pFontProgram);

	// You can follow this pattern to load additional shaders

	// Create the skybox
	// Skybox downloaded from http://www.akimbo.in/forum/viewtopic.php?f=10&t=9
	m_pSkybox->Create(2500.0f);
	
	// Create the planar terrain
	m_pPlanarTerrain->Create("resources\\textures\\", "grassfloor01.jpg", 2000.0f, 2000.0f, 50.0f); // Texture downloaded from http://www.psionicgames.com/?page_id=26 on 24 Jan 2013

	m_pFtFont->LoadSystemFont("arial.ttf", 32);
	m_pFtFont->SetShaderProgram(pFontProgram);

	// Load some meshes in OBJ format
	m_pBarrelMesh->Load("resources\\models\\Barrel\\Barrel02.obj");  // Downloaded from http://www.psionicgames.com/?page_id=24 on 24 Jan 2013
	m_pHorseMesh->Load("resources\\models\\Horse\\Horse2.obj");  // Downloaded from http://opengameart.org/content/horse-lowpoly on 24 Jan 2013
	m_pModule->Load("resources\\models\\Module\\14011_Underwater_Colony_Science_Module_v3_L1.obj"); //Downloaded from https://free3d.com/es/modelo-3d/underwater-colony-science-module-v3--212160.html
	m_pSubmarine->Load("resources\\models\\Submarine\\submarine.obj"); //Downloaded from https://free3d.com/es/modelo-3d/mini-submarine-725226.html

	// Create a sphere
	m_pSphere->Create("resources\\textures\\", "dirtpile01.jpg", 50, 50);  // Texture downloaded from http://www.psionicgames.com/?page_id=26 on 24 Jan 2013
	glEnable(GL_CULL_FACE);

	// Initialise audio and play background music
	m_pAudio->Initialise();
	m_pAudio->LoadObjectSound("resources\\Audio\\submarine-rotor.wav");					// Royalty free sound from freesound.org https://freesound.org/people/urbanmatter/sounds/269705/
	m_pAudio->LoadSoundSource("resources\\Audio\\sonar-sweep-beep.wav");					// Royalty free sound from freesound.org https://freesound.org/people/SamsterBirdies/sounds/371178/
	m_pAudio->LoadMusicStream("resources\\Audio\\DST-BlueMist.mp3");	// Royalty free music from http://www.nosoapradio.us/ (https://drive.google.com/open?id=0Bw4MH6EXU9vTQzlNQk9qU2dTc0U)
	//m_pAudio->PlayMusicStream();
	//m_pAudio->PlaySoundSource();
	//m_pAudio->PlayObjectSound();
	
	// Create path 
	m_pPath->CreatePath("resources\\textures\\", "Tile41a.jpg");
	vector<glm::vec3> points;
	points.push_back(glm::vec3(-500, 10, -200));
	points.push_back(glm::vec3(-250, 10, -200));
	points.push_back(glm::vec3(0, 10, 200));
	points.push_back(glm::vec3(0, 10, 0));
	points.push_back(glm::vec3(0, 10, -200));
	//OutputDebugString("");
	m_pPath->CreateCentreline();
	m_pPath->CreateOffsetCurves();
	m_pPath->CreateTrack();
}

// Render method runs repeatedly in a loop
void Game::Render() 
{
	
	// Clear the buffers and enable depth testing (z-buffering)
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	// Set up a matrix stack
	glutil::MatrixStack modelViewMatrixStack;
	modelViewMatrixStack.SetIdentity();

	// Use the main shader program 
	CShaderProgram *pMainProgram = (*m_pShaderPrograms)[0];
	pMainProgram->UseProgram();
	pMainProgram->SetUniform("bUseTexture", true);
	pMainProgram->SetUniform("sampler0", 0);
	// Note: cubemap and non-cubemap textures should not be mixed in the same texture unit.  Setting unit 10 to be a cubemap texture.
	int cubeMapTextureUnit = 10; 
	pMainProgram->SetUniform("CubeMapTex", cubeMapTextureUnit);
	

	// Set the projection matrix
	pMainProgram->SetUniform("matrices.projMatrix", m_pCamera->GetPerspectiveProjectionMatrix());

	// Call LookAt to create the view matrix and put this on the modelViewMatrix stack. 
	// Store the view matrix and the normal matrix associated with the view matrix for later (they're useful for lighting -- since lighting is done in eye coordinates)
	modelViewMatrixStack.LookAt(m_pCamera->GetPosition(), m_pCamera->GetView(), m_pCamera->GetUpVector());
	glm::mat4 viewMatrix = modelViewMatrixStack.Top();
	glm::mat3 viewNormalMatrix = m_pCamera->ComputeNormalMatrix(viewMatrix);

	
	// Set light and materials in main shader program
	glm::vec4 lightPosition1 = glm::vec4(-100, 100, -100, 1); // Position of light source *in world coordinates*
	pMainProgram->SetUniform("light1.position", viewMatrix*lightPosition1); // Position of light source *in eye coordinates*
	pMainProgram->SetUniform("light1.La", glm::vec3(1.0f));		// Ambient colour of light
	pMainProgram->SetUniform("light1.Ld", glm::vec3(1.0f));		// Diffuse colour of light
	pMainProgram->SetUniform("light1.Ls", glm::vec3(1.0f));		// Specular colour of light
	pMainProgram->SetUniform("material1.Ma", glm::vec3(1.0f));	// Ambient material reflectance
	pMainProgram->SetUniform("material1.Md", glm::vec3(0.0f));	// Diffuse material reflectance
	pMainProgram->SetUniform("material1.Ms", glm::vec3(0.0f));	// Specular material reflectance
	pMainProgram->SetUniform("material1.shininess", 15.0f);		// Shininess material property
		

	// Render the skybox and terrain with full ambient reflectance 
	modelViewMatrixStack.Push();
		pMainProgram->SetUniform("renderSkybox", true);
		// Translate the modelview matrix to the camera eye point so skybox stays centred around camera
		glm::vec3 vEye = m_pCamera->GetPosition();
		modelViewMatrixStack.Translate(vEye);
		pMainProgram->SetUniform("matrices.modelViewMatrix", modelViewMatrixStack.Top());
		pMainProgram->SetUniform("matrices.normalMatrix", m_pCamera->ComputeNormalMatrix(modelViewMatrixStack.Top()));
		m_pSkybox->Render(cubeMapTextureUnit);
		pMainProgram->SetUniform("renderSkybox", false);
	modelViewMatrixStack.Pop();

	// Render the planar terrain
	modelViewMatrixStack.Push();
		pMainProgram->SetUniform("matrices.modelViewMatrix", modelViewMatrixStack.Top());
		pMainProgram->SetUniform("matrices.normalMatrix", m_pCamera->ComputeNormalMatrix(modelViewMatrixStack.Top()));
		//m_pPlanarTerrain->Render();
	modelViewMatrixStack.Pop();


	// Turn on diffuse + specular materials
	pMainProgram->SetUniform("material1.Ma", glm::vec3(0.5f));	// Ambient material reflectance
	pMainProgram->SetUniform("material1.Md", glm::vec3(0.5f));	// Diffuse material reflectance
	pMainProgram->SetUniform("material1.Ms", glm::vec3(1.0f));	// Specular material reflectance	

	/**
	// Render the horse 
	modelViewMatrixStack.Push();
		modelViewMatrixStack.Translate(glm::vec3(0.0f, 0.0f, 0.0f));
		modelViewMatrixStack.Rotate(glm::vec3(0.0f, 1.0f, 0.0f), 180.0f);
		modelViewMatrixStack.Scale(2.5f);
		pMainProgram->SetUniform("matrices.modelViewMatrix", modelViewMatrixStack.Top());
		pMainProgram->SetUniform("matrices.normalMatrix", m_pCamera->ComputeNormalMatrix(modelViewMatrixStack.Top()));
		m_pHorseMesh->Render();
	modelViewMatrixStack.Pop();

	*/

	/**
	//Render the barrel 
	modelViewMatrixStack.Push();
		modelViewMatrixStack.Translate(glm::vec3(100.0f, 0.0f, 0.0f));
		modelViewMatrixStack.Scale(5.0f);
		pMainProgram->SetUniform("matrices.modelViewMatrix", modelViewMatrixStack.Top());
		pMainProgram->SetUniform("matrices.normalMatrix", m_pCamera->ComputeNormalMatrix(modelViewMatrixStack.Top()));
		m_pBarrelMesh->Render();
	modelViewMatrixStack.Pop();
	*/

	// Render the submarine 
	modelViewMatrixStack.Push();
		modelViewMatrixStack.Translate(m_submarinePosition);
		modelViewMatrixStack *= m_submarineOrientation;
		//modelViewMatrixStack.Rotate(glm::vec3(0.0f, 1.0f, 0.0f), 180.0f);
		modelViewMatrixStack.Scale(0.5f);
		pMainProgram->SetUniform("matrices.modelViewMatrix", modelViewMatrixStack.Top());
		pMainProgram->SetUniform("matrices.normalMatrix", m_pCamera->ComputeNormalMatrix(modelViewMatrixStack.Top()));
		pMainProgram->SetUniform("bUseTexture", true);
		m_pSubmarine->Render();
	modelViewMatrixStack.Pop();

	// Render the module
	modelViewMatrixStack.Push();
		modelViewMatrixStack.Translate(m_pSoundSource->GetPosition());
		modelViewMatrixStack.Rotate(glm::vec3(1.0f, 0.0f, 0.0f), 80.0f);
		modelViewMatrixStack.Scale(0.005f);
		pMainProgram->SetUniform("matrices.modelViewMatrix", modelViewMatrixStack.Top());
		pMainProgram->SetUniform("matrices.normalMatrix", m_pCamera->ComputeNormalMatrix(modelViewMatrixStack.Top()));
		// To turn off texture mapping and use the sphere colour only (currently white material), uncomment the next line
		//pMainProgram->SetUniform("bUseTexture", false);
		m_pModule->Render();
	modelViewMatrixStack.Pop();

	//Render the path
	modelViewMatrixStack.Push();
		//pMainProgram->SetUniform("bUseTexture", false);
		// turn off texturing
		pMainProgram->SetUniform("matrices.modelViewMatrix", modelViewMatrixStack.Top());
		pMainProgram->SetUniform("matrices.normalMatrix", m_pCamera->ComputeNormalMatrix(modelViewMatrixStack.Top()));
		pMainProgram->SetUniform("bUseTexture", true);
		m_pPath->RenderTrack();
	modelViewMatrixStack.Pop();

	//Render the spline
	modelViewMatrixStack.Push();
		pMainProgram->SetUniform("bUseTexture", false);
		// turn off texturing
		pMainProgram->SetUniform("matrices.modelViewMatrix", modelViewMatrixStack.Top());
		pMainProgram->SetUniform("matrices.normalMatrix", m_pCamera->ComputeNormalMatrix(modelViewMatrixStack.Top()));
		//m_pPath->RenderCentreline();
	modelViewMatrixStack.Pop();

	//Render the offset splines
	modelViewMatrixStack.Push();
		pMainProgram->SetUniform("bUseTexture", false);
		// turn off texturing
		pMainProgram->SetUniform("matrices.modelViewMatrix", modelViewMatrixStack.Top());
		pMainProgram->SetUniform("matrices.normalMatrix", m_pCamera->ComputeNormalMatrix(modelViewMatrixStack.Top()));
		//m_pPath->RenderOffsetCurves();
	modelViewMatrixStack.Pop();

	// Draw the 2D graphics after the 3D graphics
	DisplayFrameRate();

	// Swap buffers to show the rendered image
	SwapBuffers(m_gameWindow.Hdc());		

}

// Update method runs repeatedly with the Render method
void Game::Update() 
{
	// Update the camera using the amount of time that has elapsed to avoid framerate dependent motion
	m_pCamera->Update(m_dt);

	//Direction of the camera
	glm::vec3 pNext;
	m_pPath->Sample(m_currentDistance + 1.0f, pNext);

	//Moving the camera 
	m_currentDistance += m_dt * m_cameraSpeed;
	glm::vec3 p;
	m_pPath->Sample(m_currentDistance, p);
	
	//Normalised tangent vector T that points from p to pNext
	glm::vec3 t = glm::normalize(glm::vec3(pNext.x - p.x, pNext.y - p.y, pNext.z - p.z));
	//N = T x y. Replace y for interpolated up vector
	glm::vec3 n = glm::normalize(glm::cross(t, glm::vec3(0, 1, 0)));
	//B = N x T
	glm::vec3 b = glm::normalize(glm::cross(n, t));

	glm::vec3 m_position = p + b*5.0f;
	glm::vec3 m_view = p + 10.0f * t;
	glm::vec3 m_upVector = glm::rotate(glm::vec3(0, 1, 0), m_cameraRotation, t);
	//m_pCamera->Set(m_position, m_view , m_upVector);

	//DSP and audio programming coursework:

	//Submarine position and orientation
	m_t += m_submarineVel * (float)m_dt;
	float r = 100.0f;  
	glm::vec3 x = glm::vec3(1, 0, 0);  
	glm::vec3 y = glm::vec3(0, 1, 0);  
	glm::vec3 z = glm::vec3(0, 0, 1); 
	//Sets submarine position
	m_submarinePosition = r * cos(m_t) * x + 15.0f * y + r * sin(m_t) * z;
	glm::vec3 T = glm::normalize(-r * sin(m_t) * x + r * cos(m_t) * z); 
	glm::vec3 N = glm::normalize(glm::cross(T, y)); 
	glm::vec3 B = glm::normalize(glm::cross(N, T));
	//Sets submarine orientation
	m_submarineOrientation = glm::mat4(glm::mat3(T, B, N));
	//Sets position and velocity vectors for the submarine sound source
	m_pSubmarineSoundSource->SetPosition(m_submarinePosition);
	m_pSubmarineSoundSource->SetVelocity(glm::vec3(m_t, 0, m_t));

	//Updates the module sound source 
	m_pSoundSource->Update(m_dt);

	//Updates the audio object
	m_pAudio->Update(m_filterControl, m_pSoundSource, m_pSubmarineSoundSource, m_submarineVel, m_pCamera);
}



void Game::DisplayFrameRate()
{

	CShaderProgram *fontProgram = (*m_pShaderPrograms)[1];

	RECT dimensions = m_gameWindow.GetDimensions();
	int height = dimensions.bottom - dimensions.top;

	// Increase the elapsed time and frame counter
	m_elapsedTime += m_dt;
	m_frameCount++;

	// Now we want to subtract the current time by the last time that was stored
	// to see if the time elapsed has been over a second, which means we found our FPS.
	if (m_elapsedTime > 1000)
    {
		m_elapsedTime = 0;
		m_framesPerSecond = m_frameCount;

		// Reset the frames per second
		m_frameCount = 0;
    }

	if (m_framesPerSecond > 0) {
		// Use the font shader program and render the text
		fontProgram->UseProgram();
		glDisable(GL_DEPTH_TEST);
		fontProgram->SetUniform("matrices.modelViewMatrix", glm::mat4(1));
		fontProgram->SetUniform("matrices.projMatrix", m_pCamera->GetOrthographicProjectionMatrix());
		fontProgram->SetUniform("vColour", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
		m_pFtFont->Render(20, height - 20, 20, "FPS: %d", m_framesPerSecond);
	}
}

// The game loop runs repeatedly until game over
void Game::GameLoop()
{
	/*
	// Fixed timer
	dDt = pHighResolutionTimer->Elapsed();
	if (dDt > 1000.0 / (double) Game::FPS) {
		pHighResolutionTimer->Start();
		Update();
		Render();
	}
	*/
	
	
	// Variable timer
	m_pHighResolutionTimer->Start();
	Update();
	Render();
	m_dt = m_pHighResolutionTimer->Elapsed();
	

}


WPARAM Game::Execute() 
{
	m_pHighResolutionTimer = new CHighResolutionTimer;
	m_gameWindow.Init(m_hInstance);

	if(!m_gameWindow.Hdc()) {
		return 1;
	}

	Initialise();

	m_pHighResolutionTimer->Start();

	
	MSG msg;

	while(1) {													
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) { 
			if(msg.message == WM_QUIT) {
				break;
			}

			TranslateMessage(&msg);	
			DispatchMessage(&msg);
		} else if (m_appActive) {
			GameLoop();
		} 
		else Sleep(200); // Do not consume processor power if application isn't active
	}

	m_gameWindow.Deinit();

	return(msg.wParam);
}

LRESULT Game::ProcessEvents(HWND window,UINT message, WPARAM w_param, LPARAM l_param) 
{
	LRESULT result = 0;

	switch (message) {


	case WM_ACTIVATE:
	{
		switch(LOWORD(w_param))
		{
			case WA_ACTIVE:
			case WA_CLICKACTIVE:
				m_appActive = true;
				m_pHighResolutionTimer->Start();
				break;
			case WA_INACTIVE:
				m_appActive = false;
				break;
		}
		break;
		}

	case WM_SIZE:
			RECT dimensions;
			GetClientRect(window, &dimensions);
			m_gameWindow.SetDimensions(dimensions);
		break;

	case WM_PAINT:
		PAINTSTRUCT ps;
		BeginPaint(window, &ps);
		EndPaint(window, &ps);
		break;

	case WM_KEYDOWN:
		switch(w_param) {
		case VK_ESCAPE:
			PostQuitMessage(0);
			break;
		case '1': //Plays the music stream - Task 1
			m_pAudio->PlayMusicStream();
			break;
		case '2': //Plays the sound od the module - Task 2a Part 1
			m_pAudio->PlaySoundSource();
			break;
		case '3': //Plays the sound od the submarine - Task 2a Part 2
			m_pAudio->PlayObjectSound();
			break;
		//Controls for moving the module
		case 'Y':
			m_pSoundSource->SetVelocity(glm::vec3(0, 0, 0.1f));
			break;
		case 'H':
			m_pSoundSource->SetVelocity(glm::vec3(0, 0, -0.1f));
			break;
		case 'G':
			m_pSoundSource->SetVelocity(glm::vec3(-0.1f, 0, 0));
			break;
		case 'J':
			m_pSoundSource->SetVelocity(glm::vec3(0.1f, 0, 0));
			break;
		case VK_SPACE: //Stops the module
			m_pSoundSource->SetVelocity(glm::vec3(0, 0, 0));
			break;
		//Changes the value of the filter control applied to the music
		case VK_F1:
			if (m_filterControl > 0.0) {
				m_filterControl = m_filterControl-0.1;
			}
			break;
		case VK_F2:
			if (m_filterControl < 1.0) {
				m_filterControl = m_filterControl+0.1;
			}
			break;
		//Changes the submarine speed
		case VK_F3:
			if (m_submarineVel > 0.0002) {
				m_submarineVel = m_submarineVel - 0.0001;
			}
			break;
		case VK_F4:
			if (m_submarineVel < 0.001) {
				m_submarineVel = m_submarineVel + 0.0001;
			}
			break;
		case VK_RIGHT:
			m_cameraRotation = m_cameraRotation + m_dt * 0.01f;
			break;
		case VK_LEFT:
			m_cameraRotation = m_cameraRotation - m_dt * 0.01f;
			break;
		}
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		result = DefWindowProc(window, message, w_param, l_param);
		break;
	}

	return result;
}

Game& Game::GetInstance() 
{
	static Game instance;

	return instance;
}

void Game::SetHinstance(HINSTANCE hinstance) 
{
	m_hInstance = hinstance;
}

LRESULT CALLBACK WinProc(HWND window, UINT message, WPARAM w_param, LPARAM l_param)
{
	return Game::GetInstance().ProcessEvents(window, message, w_param, l_param);
}

int WINAPI WinMain(HINSTANCE hinstance, HINSTANCE, PSTR, int) 
{
	Game &game = Game::GetInstance();
	game.SetHinstance(hinstance);

	return game.Execute();
}
