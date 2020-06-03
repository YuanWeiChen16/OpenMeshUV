#include <Common.h>
#include <ViewManager.h>
#include <AntTweakBar/AntTweakBar.h>
#include <ResourcePath.h>

#include "OpenMesh.h"
#include "MeshObject.h"
#include "DrawModelShader.h"
#include "PickingShader.h"
#include "PickingTexture.h"
#include "DrawPickingFaceShader.h"
#include "DrawTextureShader.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../../Include/STB/stb_image_write.h"

using namespace glm;
using namespace std;

int windowWidth = 1200;
int windowHeight = 600;
const std::string ProjectName = "TextureParameterization";

GLuint			program;			//shader program
mat4			proj_matrix;		//projection matrix
float			aspect;

GLuint			textureID;
ViewManager		meshWindowCam, texCoordWindowCam;

bool isRightButtonPress = false;
bool drawTexture = false;
float uvRotateAngle = 0.0;
float prevUVRotateAngle = 0.0;
MeshObject model;

DrawTextureShader drawTextureShader;
DrawModelShader drawModelShader;
DrawPickingFaceShader drawPickingFaceShader;
PickingShader pickingShader;
PickingTexture pickingTexture;

int mainWindow, meshWindow, texCoordWindow;
TwBar *bar;

void TW_CALL ParameterizationBtn(void *clientData)
{
	model.Parameterization(uvRotateAngle);
	prevUVRotateAngle = uvRotateAngle;
}

void setupGUI()
{
#ifdef _MSC_VER
	TwInit(TW_OPENGL, NULL);
#else
	TwInit(TW_OPENGL_CORE, NULL);
#endif
	TwGLUTModifiersFunc(glutGetModifiers);
	bar = TwNewBar("Texture Parameter Setting");
	TwDefine(" 'Texture Parameter Setting' size='220 90' ");
	TwDefine(" 'Texture Parameter Setting' fontsize='3' color='96 216 224'");
	
	TwAddVarRW(bar, "Show Texture", TW_TYPE_BOOLCPP, &drawTexture, NULL);
	TwAddVarRW(bar, "UV Rotate", TW_TYPE_FLOAT, &uvRotateAngle, "min=0 max=360 step=0.1");
	TwAddButton(bar, "Parameterization", ParameterizationBtn, NULL, "label='Parameterization'");
}

void My_LoadModel()
{
	if (model.Init(ResourcePath::modelPath))
	{
		int id = 0;
		while (model.AddSelectedFace(id))
		{
			++id;
		}
		model.Parameterization();
		drawTexture = true;

		puts("Load Model");
	}
	else
	{
		puts("Load Model Failed");
	}
}

void My_LoadTextures()
{
	//Texture setting
	///////////////////////////	
	//Load texture data from file
	TextureData tdata = Common::Load_png((ResourcePath::imagePath + "checkerboard4.jpg").c_str());

	//Generate empty texture
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	//Do texture setting
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, tdata.width, tdata.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, tdata.data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);
	///////////////////////////	
}

void InitOpenGL()
{
	glEnable(GL_TEXTURE_2D);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void InitData()
{
	ResourcePath::shaderPath = "./Shader/" + ProjectName + "/";
	ResourcePath::imagePath = "./Imgs/" + ProjectName + "/";
	ResourcePath::modelPath = "./Model/UnionSphere.obj";

	//Initialize shaders
	///////////////////////////	
	drawTextureShader.Init();
	drawModelShader.Init();
	pickingShader.Init();
	pickingTexture.Init(windowWidth, windowHeight);
	drawPickingFaceShader.Init();

	//Load model to shader program
	My_LoadTextures();
	My_LoadModel();
}

void Reshape(int width, int height)
{
	windowWidth = width;
	windowHeight = height;

	TwWindowSize(width, height);

	int meshWindowWidth = width / 2;
	int meshWindowHeight = height;
	glutSetWindow(meshWindow);
	glutPositionWindow(0, 0);
	glutReshapeWindow(meshWindowWidth, meshWindowHeight);
	glViewport(0, 0, meshWindowWidth, meshWindowHeight);

	aspect = meshWindowWidth * 1.0f / meshWindowHeight;
	meshWindowCam.SetWindowSize(meshWindowWidth, meshWindowHeight);
	pickingTexture.Init(meshWindowWidth, meshWindowHeight);

	int texCoordWindowWidth = width / 2;
	int texCoordWindowHeight = height;
	glutSetWindow(texCoordWindow);
	glutPositionWindow(meshWindowWidth, 0);
	glutReshapeWindow(texCoordWindowWidth, texCoordWindowHeight);
	glViewport(0, 0, texCoordWindowWidth, texCoordWindowHeight);

	texCoordWindowCam.SetWindowSize(texCoordWindowWidth, texCoordWindowHeight);
	texCoordWindowCam.ToggleOrtho();
}

// GLUT callback. Called to draw the scene.
void RenderMeshWindow()
{
	glutSetWindow(meshWindow);

	//Update shaders' input variable
	///////////////////////////	

	glm::mat4 mvMat = meshWindowCam.GetViewMatrix() * meshWindowCam.GetModelMatrix();
	glm::mat4 pMat = meshWindowCam.GetProjectionMatrix(aspect);

	if (!drawTexture)
	{
		pickingTexture.Enable();

		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		pickingShader.Enable();
		pickingShader.SetMVMat(value_ptr(mvMat));
		pickingShader.SetPMat(value_ptr(pMat));

		model.Render();

		pickingShader.Disable();
		pickingTexture.Disable();
	}
	///////////////////////////	

	glClearColor(0.5f, 0.5f, 0.5f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	drawModelShader.Enable();
	glm::mat3 normalMat = glm::transpose(glm::inverse(glm::mat3(mvMat)));

	float radian = uvRotateAngle * M_PI / 180.0f;
	glm::mat4 uvRotMat = glm::rotate(radian, glm::vec3(0.0, 0.0, 1.0));

	drawModelShader.SetWireColor(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
	drawModelShader.SetFaceColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	drawModelShader.UseLighting(true);
	drawModelShader.DrawTexCoord(false);
	drawModelShader.DrawTexture(false);
	drawModelShader.DrawWireframe(!drawTexture);
	drawModelShader.SetNormalMat(normalMat);
	drawModelShader.SetMVMat(mvMat);
	drawModelShader.SetPMat(pMat);
	drawModelShader.SetUVRotMat(uvRotMat);
	
	model.Render();
	if (drawTexture)
	{
		drawModelShader.DrawTexture(true);
		
		glBindTexture(GL_TEXTURE_2D, textureID);
		model.RenderParameterized();
		glBindTexture(GL_TEXTURE_2D, 0);

	}

	drawModelShader.Disable();
	///////////////////////////	

	/*if (!drawTexture)
	{
		drawPickingFaceShader.Enable();
		drawPickingFaceShader.SetMVMat(value_ptr(mvMat));
		drawPickingFaceShader.SetPMat(value_ptr(pMat));
		model.RenderSelectedFace();
		drawPickingFaceShader.Disable();
	}*/


	glUseProgram(0);
	TwDraw();

	glutSwapBuffers();
}

void RenderTexCoordWindow()
{
	glutSetWindow(texCoordWindow);
	float radian = (uvRotateAngle - prevUVRotateAngle) * M_PI / 180.0f;
	glm::mat4 uvRotMat = glm::rotate(radian, glm::vec3(0.0, 0.0, 1.0));
	glm::mat4 mvMat = texCoordWindowCam.GetViewMatrix() * texCoordWindowCam.GetModelMatrix();
	glm::mat4 pMat = texCoordWindowCam.GetProjectionMatrix(aspect);
	
	glClearColor(0.3f, 0.3f, 0.3f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/*if (drawTexture)
	{
		drawTextureShader.Enable();
		drawTextureShader.SetMVMat(mvMat);
		drawTextureShader.SetPMat(pMat);

		glBindTexture(GL_TEXTURE_2D, textureID);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glBindTexture(GL_TEXTURE_2D, 0);

		drawTextureShader.Disable();
	}*/

	///////////////////////////////////

	drawModelShader.Enable();
	
	drawModelShader.SetFaceColor(glm::vec4(0.0f, 0.0f, 0.0f, 0.0f));
	drawModelShader.SetWireColor(glm::vec4(1.0f, 1.0f, 0.0f, 1.0f));
	drawModelShader.UseLighting(false);
	drawModelShader.DrawWireframe(true);
	drawModelShader.DrawTexCoord(true);
	drawModelShader.DrawTexture(false);
	drawModelShader.SetMVMat(mvMat);
	drawModelShader.SetPMat(pMat);
	drawModelShader.SetUVRotMat(uvRotMat);

	model.RenderParameterized();

	drawModelShader.Disable();
	glutSwapBuffers();
}

void Render()
{
	glutSetWindow(mainWindow);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glutSwapBuffers();
}

void RenderAll()
{
	RenderMeshWindow();
	RenderTexCoordWindow();
}


//Timer event
void My_Timer(int val)
{
	glutPostRedisplay();
	glutTimerFunc(16, My_Timer, val);
}

//Mouse event
void My_Mouse(int button, int state, int x, int y)
{
	if (!TwEventMouseButtonGLUT(button, state, x, y))
	{
		if (glutGetWindow() == meshWindow)
		{
			meshWindowCam.mouseEvents(button, state, x, y);

			if (button == GLUT_RIGHT_BUTTON)
			{
				if (state == GLUT_DOWN)
				{
					isRightButtonPress = true;
				}
				else if (state == GLUT_UP)
				{
					isRightButtonPress = false;
				}
			}
		}

		if (glutGetWindow() == texCoordWindow)
		{
			texCoordWindowCam.mouseEvents(button, state, x, y);
		}
	}
}

//Keyboard event
void My_Keyboard(unsigned char key, int x, int y)
{
	if (!TwEventKeyboardGLUT(key, x, y))
	{
		meshWindowCam.keyEvents(key);
	}

	if (key == 'g' || key == 'G')
	{
		glutSetWindow(meshWindow);
		GLubyte *data = new GLubyte[windowWidth / 2 * windowHeight * 4];
		glReadPixels(0, 0, windowWidth / 2, windowHeight, GL_RGBA, GL_UNSIGNED_BYTE, data);
		stbi_write_png("ScreenShot777.png", windowWidth / 2, windowHeight, 4, data, windowWidth / 2 * 4);

		glutSetWindow(texCoordWindow);
		glReadPixels(0, 0, windowWidth / 2, windowHeight, GL_RGBA, GL_UNSIGNED_BYTE, data);
		stbi_write_png("ScreenShot888.png", windowWidth / 2, windowHeight, 4, data, windowWidth / 2 * 4);
	}
}


void My_Mouse_Moving(int x, int y) {
	if (!TwEventMouseMotionGLUT(x, y))
	{
		meshWindowCam.mouseMoveEvent(x, y);

		if (isRightButtonPress && !drawTexture)
		{
			GLuint faceID = pickingTexture.ReadTexture(x, windowHeight - y - 1);
			if (faceID != 0)
			{
				model.AddSelectedFace(faceID - 1);
			}
		}
	}
}

int main(int argc, char *argv[])
{
#ifdef __APPLE__
	//Change working directory to source code path
	chdir(__FILEPATH__("/../Assets/"));
#endif
	// Initialize GLUT and GLEW, then create a window.
	////////////////////
	glutInit(&argc, argv);
#ifdef _MSC_VER
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_ALPHA);
#else
	glutInitDisplayMode(GLUT_3_2_CORE_PROFILE | GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
#endif

	glutInitWindowPosition(100, 100);
	glutInitWindowSize(windowHeight, windowHeight);
	mainWindow = glutCreateWindow(ProjectName.c_str()); // You cannot use OpenGL functions before this line; The OpenGL context must be created first by glutCreateWindow()!
#ifdef _MSC_VER
	glewInit();
#endif

	glutDisplayFunc(Render);
	glutReshapeFunc(Reshape);
	glutIdleFunc(RenderAll);
	glutSetOption(GLUT_RENDERING_CONTEXT, GLUT_USE_CURRENT_CONTEXT);
	setupGUI();

	meshWindow = glutCreateSubWindow(mainWindow, 0, 0, windowWidth / 2, windowHeight);
	glutMouseFunc(My_Mouse);
	glutKeyboardFunc(My_Keyboard);
	glutMotionFunc(My_Mouse_Moving);
	glutDisplayFunc(RenderMeshWindow);
	InitOpenGL();
	InitData();

	texCoordWindow = glutCreateSubWindow(mainWindow, windowWidth / 2, 0, windowWidth / 2, windowHeight);
	glutMouseFunc(My_Mouse);
	glutDisplayFunc(RenderTexCoordWindow);
	InitOpenGL();

	//Print debug information 
	Common::DumpInfo();

	// Enter main event loop.
	glutMainLoop();

	return 0;
}

