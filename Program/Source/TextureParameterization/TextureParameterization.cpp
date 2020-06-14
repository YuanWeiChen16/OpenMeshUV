#include <Common.h>
#include <ViewManager.h>
#include <AntTweakBar/AntTweakBar.h>
#include <ResourcePath.h>

#include <Eigen/Sparse>

#include "OpenMesh.h"
#include "MeshObject.h"
#include "DrawModelShader.h"
#include "PickingShader.h"
#include "PickingTexture.h"
#include "DrawPickingFaceShader.h"
#include "DrawTextureShader.h"
#include "DrawPointShader.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../../Include/STB/stb_image.h"


//#define STB_IMAGE_WRITE_IMPLEMENTATION
//#include "../../Include/STB/stb_image_write.h"





//ifdebug===============================================
//#define DEBUG 

using namespace glm;
using namespace std;
using namespace Eigen;

glm::vec3 worldPos;
bool updateFlag = false;
bool isRightButtonPress = false;
GLuint currentFaceID = 0;
int currentMouseX = 0;
int currentMouseY = 0;
int windowWidth = 1200;
int windowHeight = 600;
const std::string ProjectName = "TextureParameterization";

GLuint			program;			// shader program
mat4			proj_matrix;		// projection matrix
float			aspect;
ViewManager		meshWindowCam;

MeshObject model;
MeshObject BeSelectModel;

vector<unsigned int> OuterPoint;
vector<unsigned int> InnerPoint;

float OuterLengh = 0;


float TextureX = 0;
float TextureY = 0;
float TexRotate = 0;


unsigned int textureID;
unsigned int NormalID;



// shaders
DrawModelShader drawModelShader;
DrawPickingFaceShader drawPickingFaceShader;
PickingShader pickingShader;
PickingTexture pickingTexture;
DrawPointShader drawPointShader;

// vbo for drawing point
GLuint vboPoint;

int mainWindow;
enum SelectionMode
{
	ADD_FACE,
	DEL_FACE,
	SELECT_POINT,
	ADD_POINT,
	DEL_POINT,
	ONE_RING
};
SelectionMode selectionMode = ADD_FACE;

TwBar* bar;
TwEnumVal SelectionModeEV[] = { {ADD_FACE, "Add face"}, {DEL_FACE, "Delete face"}, {SELECT_POINT, "Point"},{ADD_POINT,"Add point"},{DEL_POINT,"Delete point"},{ONE_RING,"One Ring"} };
TwType SelectionModeType;


//calu Angle 
double PointAngle(MyMesh::Point P1, MyMesh::Point P2, MyMesh::Point VPoint)
{
	double V1x = (P1[0] - VPoint[0]);
	double V1y = (P1[1] - VPoint[1]);
	double V1z = (P1[2] - VPoint[2]);

	double V2x = (P2[0] - VPoint[0]);
	double V2y = (P2[1] - VPoint[1]);
	double V2z = (P2[2] - VPoint[2]);

	double A_B = V1x * V2x + V1y * V2y + V1z * V2z;
	double lAl = sqrt(V1x*V1x + V1y * V1y + V1z * V1z);
	double lBl = sqrt(V2x*V2x + V2y * V2y + V2z * V2z);

	double angle = acos(A_B / (lAl*lBl));

	angle = (angle*180.0) / 3.1415926;
	return angle;
}
unsigned int loadTexture(std::string path, int imageType);
void magic();
void magic_delete();



void SetupGUI()
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

	// Defining season enum type
	SelectionModeType = TwDefineEnum("SelectionModeType", SelectionModeEV, 3);
	// Adding season to bar
	TwAddVarRW(bar, "SelectionMode", SelectionModeType, &selectionMode, NULL);
}

void My_LoadModel()
{
	if (model.Init(ResourcePath::modelPath))
	{
		/*int id = 0;
		while (model.AddSelectedFace(id))
		{
			++id;
		}
		model.Parameterization();
		drawTexture = true;*/

		puts("Load Model");
	}
	else
	{
		puts("Load Model Failed");
	}
}

void InitOpenGL()
{
	glEnable(GL_TEXTURE_2D);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_POINT_SMOOTH);
}

void InitData()
{

	ResourcePath::shaderPath = "./Shader/" + ProjectName + "/";
	//ResourcePath::imagePath = "./Imgs/" + ProjectName + "/";
	ResourcePath::imagePath = "./Imgs/" + ProjectName + "/";

	ResourcePath::modelPath = "./Model/bear.obj";

	//Initialize shaders
	///////////////////////////	
	drawModelShader.Init();
	pickingShader.Init();
	pickingTexture.Init(windowWidth, windowHeight);
	drawPickingFaceShader.Init();
	drawPointShader.Init();

	glGenBuffers(1, &vboPoint);

	//Load model to shader program
	My_LoadModel();
	textureID = loadTexture("Imgs\\TextureParameterization\\checkerboard4.jpg", GL_RGB);
	NormalID = loadTexture("Imgs\\TextureParameterization\\normalmap.jpg", GL_RGB);

}

void Reshape(int width, int height)
{
	windowWidth = width;
	windowHeight = height;

	TwWindowSize(width, height);
	glutReshapeWindow(windowWidth, windowHeight);
	glViewport(0, 0, windowWidth, windowHeight);

	aspect = windowWidth * 1.0f / windowHeight;
	meshWindowCam.SetWindowSize(windowWidth, windowHeight);
	pickingTexture.Init(windowWidth, windowHeight);
}

// GLUT callback. Called to draw the scene.
void RenderMeshWindow()
{
	//Update shaders' input variable
	///////////////////////////	

	glm::mat4 mvMat = meshWindowCam.GetViewMatrix() * meshWindowCam.GetModelMatrix();
	glm::mat4 pMat = meshWindowCam.GetProjectionMatrix(aspect);

	// write faceID+1 to framebuffer
	pickingTexture.Enable();

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	pickingShader.Enable();
	pickingShader.SetMVMat(value_ptr(mvMat));
	pickingShader.SetPMat(value_ptr(pMat));

	model.Render();

	pickingShader.Disable();
	pickingTexture.Disable();


	// draw model
	glClearColor(0.5f, 0.5f, 0.5f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	drawModelShader.Enable();
	//glm::mat3 normalMat; // glm::transpose(glm::inverse(glm::mat3(mvMat)));
	glm::mat3 normalMat = glm::mat3(mvMat);//glm::transpose(glm::inverse(glm::mat3(mvMat)));

	drawModelShader.SetWireColor(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
	drawModelShader.SetFaceColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	drawModelShader.UseLighting(true);
	drawModelShader.DrawWireframe(true);
	drawModelShader.SetTexcoord(0, 0, 0);
	drawModelShader.SetNormalMat(normalMat);
	drawModelShader.SetMVMat(mvMat);
	drawModelShader.SetPMat(pMat);
	model.Render();

	if (model.selectedPoint.size() > 0)
	{
		drawModelShader.SetWireColor(glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));
		drawModelShader.SetFaceColor(glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));

		//drawModelShader.DrawTexCoord(true);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textureID);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, NormalID);

		drawModelShader.DrawTexture(true);
		drawModelShader.DrawWireframe(false);
		//BeSelectModel.Render();
		drawModelShader.SetTexcoord(TextureX, TextureY, TexRotate);
		//drawModelShader.DrawTexCoord(true);
		BeSelectModel.Render();
		drawModelShader.DrawTexture(false);
		drawModelShader.DrawWireframe(true);
		//drawModelShader.DrawTexCoord(false);

	}
	else
	{

	}
	drawModelShader.Disable();



	// render selected face
	if (selectionMode == SelectionMode::ADD_FACE || selectionMode == SelectionMode::DEL_FACE)
	{
		drawPickingFaceShader.Enable();
		drawPickingFaceShader.SetMVMat(value_ptr(mvMat));
		drawPickingFaceShader.SetPMat(value_ptr(pMat));
		if (model.selectedPoint.size() <= 0)
		{
			model.RenderSelectedFace();
		}
		drawPickingFaceShader.Disable();
	}

	glUseProgram(0);

	// render closest point
	if (selectionMode == SelectionMode::SELECT_POINT)
	{
		unsigned int Pointidx;
		MyMesh::VertexHandle VH;
		if (updateFlag)
		{
			float depthValue = 0;
			int windowX = currentMouseX;
			int windowY = windowHeight - currentMouseY;
			glReadPixels(windowX, windowY, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depthValue);

			GLint _viewport[4];
			glGetIntegerv(GL_VIEWPORT, _viewport);
			glm::vec4 viewport(_viewport[0], _viewport[1], _viewport[2], _viewport[3]);
			glm::vec3 windowPos(windowX, windowY, depthValue);
			glm::vec3 wp = glm::unProject(windowPos, mvMat, pMat, viewport);
			model.FindClosestPoint(currentFaceID - 1, wp, worldPos, VH);
			magic_delete();
			model.selectedFace.clear();

			//MyMesh::Vertex Vthis = model.model.mesh.vertex(VH);

			for (MyMesh::VFIter VF = model.model.mesh.vf_begin(VH); VF != model.model.mesh.vf_end(VH); ++VF)
			{
				MyMesh::FaceHandle Fh = model.model.mesh.face_handle(VF->idx());
				model.AddSelectedFace(Fh.idx());
				for (MyMesh::FFIter FF1 = model.model.mesh.ff_begin(Fh); FF1 != model.model.mesh.ff_end(Fh); ++FF1)
				{
					MyMesh::FaceHandle Fh1 = model.model.mesh.face_handle(FF1->idx());
					model.AddSelectedFace(Fh1.idx());
					for (MyMesh::FFIter FF2 = model.model.mesh.ff_begin(*FF1); FF2 != model.model.mesh.ff_end(*FF1); ++FF2)
					{
						MyMesh::FaceHandle Fh2 = model.model.mesh.face_handle(FF2->idx());
						model.AddSelectedFace(Fh2.idx());
						for (MyMesh::FFIter FF3 = model.model.mesh.ff_begin(*FF2); FF3 != model.model.mesh.ff_end(*FF2); ++FF3)
						{
							MyMesh::FaceHandle Fh3 = model.model.mesh.face_handle(FF3->idx());
							model.AddSelectedFace(Fh3.idx());
							for (MyMesh::FFIter FF4 = model.model.mesh.ff_begin(*FF3); FF4 != model.model.mesh.ff_end(*FF3); ++FF4)
							{
								MyMesh::FaceHandle Fh4 = model.model.mesh.face_handle(FF4->idx());
								model.AddSelectedFace(Fh4.idx());
								/*for (MyMesh::FFIter FF5 = model.model.mesh.ff_begin(*FF4); FF3 != model.model.mesh.ff_end(*FF4); ++FF3)
								{
									MyMesh::FaceHandle Fh5 = model.model.mesh.face_handle(FF5->idx());
									model.AddSelectedFace(Fh5.idx());
									cout << "HA" << kkk << endl;;
									kkk ++ ;
								}*/
							}
						}
					}
				}
			}
			///this who magic work
			magic();

			updateFlag = false;
		}
		drawPickingFaceShader.Enable();
		drawPickingFaceShader.SetMVMat(value_ptr(mvMat));
		drawPickingFaceShader.SetPMat(value_ptr(pMat));
		if (model.selectedPoint.size() <= 0)
		{
			model.RenderSelectedFace();
		}
		drawPickingFaceShader.Disable();


		glBindBuffer(GL_ARRAY_BUFFER, vboPoint);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3), glm::value_ptr(worldPos), GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(0);
		glm::vec4 pointColor(1.0, 0.0, 1.0, 1.0);
		drawPointShader.Enable();
		drawPointShader.SetMVMat(mvMat);
		drawPointShader.SetPMat(pMat);
		drawPointShader.SetPointColor(pointColor);
		drawPointShader.SetPointSize(15.0);
		glDrawArrays(GL_POINTS, 0, 1);
		drawPointShader.Disable();
		glBindBuffer(GL_ARRAY_BUFFER, 0);


	}

	if (model.selectedPoint.size() > 0)
	{

		/*glColor3f(0, 1, 1);
		glBegin(GL_TRIANGLES);
		for (MyMesh::FaceIter F = BeSelectModel.model.mesh.faces_begin(); F != BeSelectModel.model.mesh.faces_end(); ++F)
		{
			for (MyMesh::FVIter FV = BeSelectModel.model.mesh.fv_begin(*F); FV != BeSelectModel.model.mesh.fv_end(*F); ++FV)
			{
				MyMesh::VHandle DrawPointVH = BeSelectModel.model.mesh.vertex_handle(FV->idx());
				MyMesh::Point DrawPoint = BeSelectModel.model.mesh.point(DrawPointVH);
				MyMesh::TexCoord2D DrawPoint_TEXUV = BeSelectModel.model.mesh.texcoord2D(DrawPointVH);
				glVertex3f(DrawPoint[0], DrawPoint[1], DrawPoint[2]);

				cout << "DRAW¡@TRI"<< DrawPoint <<endl;
				glTexCoord2f(DrawPoint_TEXUV[0], DrawPoint_TEXUV[1]);
			}
		}
		glEnd();*/

		glBindTexture(GL_TEXTURE_2D, textureID);
		glColor3f(1, 1, 1);
		glBegin(GL_QUADS);
		glTexCoord2d(0, 0); glVertex2d(0, 0);
		glTexCoord2d(1, 0); glVertex2d(0, 1);
		glTexCoord2d(1, 1); glVertex2d(1, 1);
		glTexCoord2d(0, 1); glVertex2d(1, 0);
		glEnd();


		glColor3f(0, 1, 0);
		glBegin(GL_POINTS);
		glPointSize(50);

		for (MyMesh::VertexIter V = BeSelectModel.model.mesh.vertices_begin(); V != BeSelectModel.model.mesh.vertices_end(); ++V)
		{
			MyMesh::VHandle DrawPoint1 = BeSelectModel.model.mesh.vertex_handle(V->idx());
			MyMesh::TexCoord2D outPoint1_p = BeSelectModel.model.mesh.texcoord2D(DrawPoint1);
			glVertex3f(outPoint1_p[0], outPoint1_p[1], 0);
		}
		glEnd();

		glColor3f(1, 0, 0);
		glBegin(GL_LINES);
		glLineWidth(5);
		for (MyMesh::EdgeIter E = BeSelectModel.model.mesh.edges_begin(); E != BeSelectModel.model.mesh.edges_end(); ++E)
		{
			MyMesh::EdgeHandle EH = BeSelectModel.model.mesh.edge_handle(E->idx());
			MyMesh::HalfedgeHandle HFH = BeSelectModel.model.mesh.halfedge_handle(EH, 0);
			MyMesh::VHandle DrawPoint1 = BeSelectModel.model.mesh.from_vertex_handle(HFH);
			MyMesh::VHandle DrawPoint2 = BeSelectModel.model.mesh.to_vertex_handle(HFH);
			MyMesh::TexCoord2D outPoint1_p = BeSelectModel.model.mesh.texcoord2D(DrawPoint1);
			MyMesh::TexCoord2D outPoint2_p = BeSelectModel.model.mesh.texcoord2D(DrawPoint2);
			glVertex3f(outPoint1_p[0], outPoint1_p[1], 0);
			glVertex3f(outPoint2_p[0], outPoint2_p[1], 0);
		}
		glEnd();

	}


	TwDraw();
	glutSwapBuffers();
}

void RenderAll()
{
	RenderMeshWindow();
}


//Timer event
void My_Timer(int val)
{
	glutPostRedisplay();
	glutTimerFunc(16, My_Timer, val);
}

void SelectionHandler(unsigned int x, unsigned int y)
{
	GLuint faceID = pickingTexture.ReadTexture(x, windowHeight - y - 1);
	if (faceID != 0)
	{
		currentFaceID = faceID;
	}

	if (selectionMode == ADD_FACE)
	{
		if (faceID != 0)
		{
			model.AddSelectedFace(faceID - 1);
		}
	}
	else if (selectionMode == DEL_FACE)
	{
		if (faceID != 0)
		{
			model.DeleteSelectedFace(faceID - 1);
		}
	}
	else if (selectionMode == SELECT_POINT)
	{
		currentMouseX = x;
		currentMouseY = y;
		updateFlag = true;
	}
}

//Mouse event
void MyMouse(int button, int state, int x, int y)
{
	if (!TwEventMouseButtonGLUT(button, state, x, y))
	{
		meshWindowCam.mouseEvents(button, state, x, y);

		if (button == GLUT_RIGHT_BUTTON)
		{
			if (state == GLUT_DOWN)
			{
				isRightButtonPress = true;
				SelectionHandler(x, y);
			}
			else if (state == GLUT_UP)
			{
				isRightButtonPress = false;
			}
		}
	}
}

//Keyboard event
void MyKeyboard(unsigned char key, int x, int y)
{
	if (!TwEventKeyboardGLUT(key, x, y))
	{
		meshWindowCam.keyEvents(key);
	}

	std::cout << "Input : " << key << endl;

	if ((key == 'f') && (model.selectedFace.size() > 0))
	{
		magic();
	}
	else if (key == 'g')
	{
		magic_delete();
	}

	else if (key == 'u')
	{
		TextureY += 0.01;
	}
	else if (key == 'j')
	{
		TextureY -= 0.01;
	}
	else if (key == 'h')
	{
		TextureX -= 0.01;
	}
	else if (key == 'k')
	{
		TextureX += 0.01;
	}
	else if (key == 'i')
	{
		TexRotate += 0.01;
	}
	else if (key == 'y')
	{
		TexRotate -= 0.01;
	}

}


void MyMouseMoving(int x, int y) {
	if (!TwEventMouseMotionGLUT(x, y))
	{
		meshWindowCam.mouseMoveEvent(x, y);

		if (isRightButtonPress)
		{
			SelectionHandler(x, y);
		}
	}
}

int main(int argc, char* argv[])
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
	glutInitWindowSize(windowWidth, windowHeight);
	mainWindow = glutCreateWindow(ProjectName.c_str()); // You cannot use OpenGL functions before this line; The OpenGL context must be created first by glutCreateWindow()!
#ifdef _MSC_VER
	glewInit();
#endif

	glutReshapeFunc(Reshape);
	glutIdleFunc(RenderAll);
	glutSetOption(GLUT_RENDERING_CONTEXT, GLUT_USE_CURRENT_CONTEXT);
	SetupGUI();

	glutMouseFunc(MyMouse);
	glutKeyboardFunc(MyKeyboard);
	glutMotionFunc(MyMouseMoving);
	glutDisplayFunc(RenderMeshWindow);
	InitOpenGL();
	InitData();

	//Print debug information 
	Common::DumpInfo();

	// Enter main event loop.
	glutMainLoop();

	return 0;
}

unsigned int loadTexture(std::string path, int imageType)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	int width, height, nrComponents;
	unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrComponents, 0);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, imageType, width, height, 0, imageType, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
		stbi_image_free(data);
	}
	else
	{
		std::cout << "texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	return textureID;
}

void magic()
{
	model.FaceToPoint();

	for (int i = 0; i < model.selectPoint_Seq.size(); i++)
	{
		for (int j = 0; j < model.selectedPoint.size(); j++)
		{
			if (model.selectedPoint[j] == model.selectPoint_Seq[i])
			{
				model.selectPoint_Seq[i] = j;
			}
		}
	}

	std::vector < MyMesh::VertexHandle> vhandle;
	//std::vector < MyMesh::Normal> ThisNormal;
	//vhandle = new MyMesh::VertexHandle[model.selectedPoint.size()];

	for (int i = 0; i < model.selectedPoint.size(); i++)
	{
		MyMesh::VertexHandle TempPoint = model.model.mesh.vertex_handle(model.selectedPoint[i]);
		MyMesh::Point closestPoint = model.model.mesh.point(TempPoint);
		//MyMesh::Normal closestNor = model.model.mesh.normal(TempPoint);
		//ThisNormal.push_back(closestNor);

		vhandle.push_back(BeSelectModel.model.mesh.add_vertex(closestPoint));
		//BeSelectModel.model.mesh.normal(vhandle[vhandle.size()-1]) = closestNor;

#ifdef DEBUG
		cout << "X: " << closestPoint[0] << "Y: " << closestPoint[1] << "Z: " << closestPoint[2] << endl;
#endif // DEBUG


	}
	std::vector<MyMesh::VertexHandle>  face_vhandles;
	for (int i = 0; i < model.selectPoint_Seq.size() / 3; i++)
	{
		face_vhandles.clear();
		face_vhandles.push_back(vhandle[model.selectPoint_Seq[i * 3]]);
		face_vhandles.push_back(vhandle[model.selectPoint_Seq[(i * 3) + 1]]);
		face_vhandles.push_back(vhandle[model.selectPoint_Seq[(i * 3 + 2)]]);
		BeSelectModel.model.mesh.add_face(face_vhandles);
#ifdef DEBUG
		cout << "FACE " << i << " Is be Rebuild" << endl;
#endif // DEBUG

		/*for (int i=0;i< vhandle.size();i++)
		{
			MyMesh::Normal temp = model.model.mesh.vertex(v_it->idx());
			temp[0] = -temp[0];
			temp[1] = -temp[1];
			temp[2] = -temp[2];
			normals.push_back(temp);
		}
*/


	}

	MyMesh::HalfedgeIter HFI = BeSelectModel.model.mesh.halfedges_begin();
	MyMesh::HalfedgeHandle HFh;
	for (; HFI != BeSelectModel.model.mesh.halfedges_end(); ++HFI)
	{
		HFh = BeSelectModel.model.mesh.halfedge_handle(HFI->idx());

		bool ISBOUND = BeSelectModel.model.mesh.is_boundary(HFh);
		if (ISBOUND)
		{
			MyMesh::VertexHandle outPoint = BeSelectModel.model.mesh.from_vertex_handle(HFh);
			OuterPoint.push_back(outPoint.idx());
			break;
		}
	}

	HFh = BeSelectModel.model.mesh.next_halfedge_handle(HFh);
	for (;;)
	{
		MyMesh::VertexHandle outPoint = BeSelectModel.model.mesh.from_vertex_handle(HFh);
		if (outPoint.idx() == OuterPoint[0])
		{
			break;
		}
		OuterPoint.push_back(outPoint.idx());
		HFh = BeSelectModel.model.mesh.next_halfedge_handle(HFh);
	}



	//caclu Outer lengh===================================================================================================================
	for (int i = 0; i < OuterPoint.size(); i++)
	{
		MyMesh::VertexHandle outPoint1 = BeSelectModel.model.mesh.vertex_handle(OuterPoint[(i) % OuterPoint.size()]);
		MyMesh::VertexHandle outPoint2 = BeSelectModel.model.mesh.vertex_handle(OuterPoint[(i + 1) % OuterPoint.size()]);
		MyMesh::Point outPoint1_p = BeSelectModel.model.mesh.point(outPoint1);
		MyMesh::Point outPoint2_p = BeSelectModel.model.mesh.point(outPoint2);
		MyMesh::Point disPoint = outPoint1_p - outPoint2_p;
		float dis = disPoint[0] * disPoint[0] + disPoint[1] * disPoint[1];
		dis = sqrt(dis);
		dis = sqrt(dis*dis + disPoint[2] * disPoint[2]);
		OuterLengh += dis;
	}
#ifdef DEBUG
	cout << "Outer Lengh is " << OuterLengh << endl;
#endif // DEBUG


	//Make OuterSide UV=====================================================================================================
	BeSelectModel.model.mesh.request_vertex_texcoords2D();// _vertex_texcoords2D();
	float nowDis = 0;
	MyMesh::VertexHandle tempoutPoint1 = BeSelectModel.model.mesh.vertex_handle(OuterPoint[(0) % OuterPoint.size()]);
	MyMesh::TexCoord2D tempUV;
	tempUV[0] = 0;
	tempUV[1] = 0;
#ifdef DEBUG
	cout << "Point " << 0 << " UV " << tempUV[0] << " " << tempUV[1] << endl;
#endif // DEBUG
	BeSelectModel.model.mesh.set_texcoord2D(tempoutPoint1, tempUV);

	//set Outter UV================================================================================================================
	for (int i = 0; i < OuterPoint.size(); i++)
	{
		MyMesh::VertexHandle outPoint1 = BeSelectModel.model.mesh.vertex_handle(OuterPoint[(i) % OuterPoint.size()]);
		MyMesh::VertexHandle outPoint2 = BeSelectModel.model.mesh.vertex_handle(OuterPoint[(i + 1) % OuterPoint.size()]);
		MyMesh::Point outPoint1_p = BeSelectModel.model.mesh.point(outPoint1);
		MyMesh::Point outPoint2_p = BeSelectModel.model.mesh.point(outPoint2);
		MyMesh::Point disPoint = outPoint1_p - outPoint2_p;
		float dis = disPoint[0] * disPoint[0] + disPoint[1] * disPoint[1];
		dis = sqrt(dis);
		dis = sqrt(dis*dis + disPoint[2] * disPoint[2]);
#ifdef DEBUG
		cout << "Dis " << dis;
#endif // DEBUG	
		nowDis = nowDis + dis;
		if (nowDis <= (OuterLengh / 4.0))
		{
			MyMesh::TexCoord2D UV;
			UV[0] = 0;
			UV[1] = nowDis / (OuterLengh / 4.0);
			BeSelectModel.model.mesh.set_texcoord2D(outPoint2, UV);
#ifdef DEBUG
			cout << "Point " << i + 1 << " UV " << UV[0] << " " << UV[1] << endl;
#endif // DEBUG
	}
		else if ((nowDis <= ((OuterLengh / 4.0) * 2.0)) && (nowDis > ((OuterLengh / 4.0)*1.0)))
		{
			MyMesh::TexCoord2D UV;
			UV[0] = ((nowDis - (OuterLengh / 4.0)*1.0) / (OuterLengh / 4.0));
			UV[1] = 1;
			BeSelectModel.model.mesh.set_texcoord2D(outPoint2, UV);
#ifdef DEBUG
			cout << "Point " << i + 1 << " UV " << UV[0] << " " << UV[1] << endl;
#endif // DEBUG
}
		else if ((nowDis <= ((OuterLengh / 4.0) * 3.0)) && (nowDis > ((OuterLengh / 4.0)*2.0)))
		{
			MyMesh::TexCoord2D UV;
			UV[0] = 1;
			UV[1] = 1 - ((nowDis - (OuterLengh / 4.0)*2.0)) / (OuterLengh / 4.0);
			BeSelectModel.model.mesh.set_texcoord2D(outPoint2, UV);
#ifdef DEBUG
			cout << "Point " << i + 1 << " UV " << UV[0] << " " << UV[1] << endl;
#endif // DEBUG
	}
		else if ((nowDis <= (OuterLengh)) && (nowDis > ((OuterLengh / 4.0)*3.0)))
		{
			MyMesh::TexCoord2D UV;
			UV[0] = 1 - ((nowDis - (OuterLengh / 4.0)*3.0)) / (OuterLengh / 4.0);
			UV[1] = 0;
			BeSelectModel.model.mesh.set_texcoord2D(outPoint2, UV);
#ifdef DEBUG
			cout << "Point " << i + 1 << " UV " << UV[0] << " " << UV[1] << endl;
#endif // DEBUG
}
	}
	//select inner point================================================================================================================
	MyMesh::VertexIter VI = BeSelectModel.model.mesh.vertices_begin();
	for (; VI != BeSelectModel.model.mesh.vertices_end(); ++VI)
	{
		MyMesh::VertexHandle SeachPoint = BeSelectModel.model.mesh.vertex_handle(VI->idx());// VI.handle();// BeSelectModel.model.mesh.vertex_handle(VI);
		bool ISOUT = false;
		for (int j = 0; j < OuterPoint.size(); j++)
		{
			if (OuterPoint[j] == SeachPoint.idx())
			{
				ISOUT = true;
			}
		}
		if (ISOUT == false)
		{
			InnerPoint.push_back(SeachPoint.idx());
#ifdef DEBUG
			cout << "InnerPoint " << InnerPoint.size() << " ID: " << SeachPoint.idx() << endl;
#endif // DEBUG
		}
			}


	SparseMatrix<double> A(InnerPoint.size(), InnerPoint.size());
	for (int i = 0; i < InnerPoint.size(); i++)
	{
		A.insert(i, i) = 1.0;
	}
	SparseQR<SparseMatrix<double>, COLAMDOrdering<int>> linearSolver;
	VectorXd Bx(InnerPoint.size());
	VectorXd By(InnerPoint.size());

	OpenMesh::EPropHandleT<MyMesh::Point> Wi;
	BeSelectModel.model.mesh.add_property(Wi);
	//Do wi==================================================================================================================================
	for (MyMesh::EdgeIter EI = BeSelectModel.model.mesh.edges_begin(); EI != BeSelectModel.model.mesh.edges_end(); ++EI)
	{
		MyMesh::Point tmepWi;

		MyMesh::EdgeHandle Eh = BeSelectModel.model.mesh.edge_handle(EI->idx());
		MyMesh::HalfedgeHandle HeH = BeSelectModel.model.mesh.halfedge_handle(Eh, 0);
		if (BeSelectModel.model.mesh.is_boundary(Eh) == false)
		{
			MyMesh::VertexHandle FromVertexH = BeSelectModel.model.mesh.from_vertex_handle(HeH);
			MyMesh::VertexHandle ToVertexH = BeSelectModel.model.mesh.to_vertex_handle(HeH);
			MyMesh::VertexHandle OppositeVertexH = BeSelectModel.model.mesh.opposite_vh(HeH);
			MyMesh::VertexHandle OppoOppoVertexH = BeSelectModel.model.mesh.opposite_he_opposite_vh(HeH);

			MyMesh::Point FromVertex = BeSelectModel.model.mesh.point(FromVertexH);
			MyMesh::Point ToVertex = BeSelectModel.model.mesh.point(ToVertexH);
			MyMesh::Point OppositeVertex = BeSelectModel.model.mesh.point(OppositeVertexH);
			MyMesh::Point OppoOppoVertex = BeSelectModel.model.mesh.point(OppoOppoVertexH);

			double Angle1, Angle2;

			Angle1 = PointAngle(FromVertex, ToVertex, OppositeVertex);
			Angle2 = PointAngle(FromVertex, ToVertex, OppoOppoVertex);
#ifdef DEBUG
			cout << "Tri1_1" << FromVertex[0] << " " << FromVertex[1] << " " << FromVertex[2] << endl;
			cout << "Tri1_2" << ToVertex[0] << " " << ToVertex[1] << " " << ToVertex[2] << endl;
			cout << "Tri1_3" << OppositeVertex[0] << " " << OppositeVertex[1] << " " << OppositeVertex[2] << endl;
			cout << "Tri2_3" << OppoOppoVertex[0] << " " << OppoOppoVertex[1] << " " << OppoOppoVertex[2] << endl;
			cout << "Angles 1 2 " << Angle1 << " " << Angle2 << endl;
#endif // DEBUG

			tmepWi[0] = ((1.0 / tan((Angle1*3.1415926) / 180.0)) + (1.0 / tan((Angle2*3.1415926) / 180.0)));
#ifdef DEBUG
			cout << tmepWi[0] << endl;
#endif // DEBUG
			tmepWi[1] = 123;
			tmepWi[2] = 456;
			BeSelectModel.model.mesh.property(Wi, *EI) = tmepWi;
		}
	}
#ifdef DEBUG
	cout << endl;
#endif // DEBUG
	for (int j = 0; j < InnerPoint.size(); j++)
	{
		Bx[j] = 0;
		By[j] = 0;
	}

	//make matrix================================================================================================================
	for (int i = 0; i < InnerPoint.size(); i++)
	{
		float ABigWi = 0;
		MyMesh::VertexHandle NowVertex = BeSelectModel.model.mesh.vertex_handle(InnerPoint[i]);
		vector<double> ThisA_Array;
		for (int j = 0; j < InnerPoint.size(); j++)
		{
			ThisA_Array.push_back(0.0);
		}
#ifdef DEBUG
		cout << "NOW POINT" << InnerPoint[i] << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
#endif // DEBUG

		//Done Matrix===========================================================================================================================
		for (MyMesh::VVIter VV = BeSelectModel.model.mesh.vv_begin(NowVertex); VV != BeSelectModel.model.mesh.vv_end(NowVertex); ++VV)
		{
			float ThisVertexWi = 0;
			MyMesh::VertexHandle TargetVet = BeSelectModel.model.mesh.vertex_handle(VV->idx());

			///Not Boundry
			for (MyMesh::VEIter VE = BeSelectModel.model.mesh.ve_begin(NowVertex); VE != BeSelectModel.model.mesh.ve_end(NowVertex); ++VE)
			{
				MyMesh::EdgeHandle Eh = BeSelectModel.model.mesh.edge_handle(VE->idx());
				MyMesh::HalfedgeHandle HeH1 = BeSelectModel.model.mesh.halfedge_handle(Eh, 0);
				MyMesh::HalfedgeHandle HeH2 = BeSelectModel.model.mesh.halfedge_handle(Eh, 1);//?
				MyMesh::VertexHandle HeH1_FromVetx = BeSelectModel.model.mesh.from_vertex_handle(HeH1);
				MyMesh::VertexHandle HeH1_ToVetx = BeSelectModel.model.mesh.to_vertex_handle(HeH1);
				if (BeSelectModel.model.mesh.is_boundary(TargetVet) == true)
				{
					//MyMesh::Point tempwi = BeSelectModel.model.mesh.property(Wi, Eh);
					if ((HeH1_FromVetx.idx() == TargetVet.idx()) && (HeH1_ToVetx.idx() == NowVertex.idx()))
					{
						MyMesh::Point EdgeWi = BeSelectModel.model.mesh.property(Wi, Eh);
						MyMesh::TexCoord2D tempT2D = BeSelectModel.model.mesh.texcoord2D(HeH1_FromVetx);
						Bx[i] += EdgeWi[0] * tempT2D[0];//x
						By[i] += EdgeWi[0] * tempT2D[1];//y
						ThisVertexWi = EdgeWi[0];
#ifdef DEBUG
						cout << "wi Right1 " << EdgeWi[0] << " " << tempT2D[0] << endl;
						cout << "wi Right2 " << EdgeWi[0] << " " << tempT2D[1] << endl;
						cout << "wi Right3 " << Bx[i] << " " << By[i] << endl << endl;
#endif // DEBUG
						break;
					}
					else if ((HeH1_FromVetx.idx() == NowVertex.idx()) && (HeH1_ToVetx.idx() == TargetVet.idx()))
					{
						MyMesh::Point EdgeWi = BeSelectModel.model.mesh.property(Wi, Eh);
						MyMesh::TexCoord2D tempT2D = BeSelectModel.model.mesh.texcoord2D(HeH1_ToVetx);
						Bx[i] += EdgeWi[0] * tempT2D[0];//x
						By[i] += EdgeWi[0] * tempT2D[1];//y
						ThisVertexWi = EdgeWi[0];
#ifdef DEBUG
						cout << "wi Right1 " << EdgeWi[0] << " " << tempT2D[0] << endl;
						cout << "wi Right2 " << EdgeWi[0] << " " << tempT2D[1] << endl;
						cout << "wi Right3 " << Bx[i] << " " << By[i] << endl << endl;
#endif // DEBUG
						break;
					}

				}
				else
				{
					if ((HeH1_FromVetx.idx() == TargetVet.idx()) && (HeH1_ToVetx.idx() == NowVertex.idx()))
					{
						MyMesh::Point EdgeWi = BeSelectModel.model.mesh.property(Wi, Eh);
						ThisVertexWi = EdgeWi[0];
#ifdef DEBUG
						cout << "wi Left " << ThisVertexWi << endl << endl;
#endif // DEBUG
						for (int j = 0; j < InnerPoint.size(); j++)
						{
							if (InnerPoint[j] == TargetVet.idx())
							{
								ThisA_Array[j] = ThisVertexWi;
								break;
							}
						}
						break;
					}
					else if ((HeH1_FromVetx.idx() == NowVertex.idx()) && (HeH1_ToVetx.idx() == TargetVet.idx()))
					{
						MyMesh::Point EdgeWi = BeSelectModel.model.mesh.property(Wi, Eh);
						ThisVertexWi = EdgeWi[0];
#ifdef DEBUG
						cout << "wi Left " << ThisVertexWi << endl << endl;
#endif // DEBUG
						for (int j = 0; j < InnerPoint.size(); j++)
						{
							if (InnerPoint[j] == TargetVet.idx())
							{
								ThisA_Array[j] = ThisVertexWi;
								break;
							}
						}
						break;
					}
					}
			}
			ABigWi += ThisVertexWi;
		}
		Bx[i] = Bx[i] / ABigWi;
		By[i] = By[i] / ABigWi;
		for (int j = 0; j < InnerPoint.size(); j++)
		{
			if (i != j)
			{
				A.insert(i, j) = ((-ThisA_Array[j]) / ABigWi);
			}
		}

	}



#ifdef DEBUG
	cout << "A" << endl;
	cout << A;
	cout << "Bx" << endl;
	cout << Bx;
	cout << "By" << endl;
	cout << By;
#endif // DEBUG
	A.makeCompressed();
	linearSolver.compute(A);
	VectorXd Xx = linearSolver.solve(Bx);
	linearSolver.compute(A);
	VectorXd Xy = linearSolver.solve(By);

	for (int i = 0; i < InnerPoint.size(); i++)
	{
		MyMesh::TexCoord2D UV;
		MyMesh::VertexHandle innerPoint = BeSelectModel.model.mesh.vertex_handle(InnerPoint[i]);
		UV[0] = Xx[i];
		UV[1] = Xy[i];
		BeSelectModel.model.mesh.set_texcoord2D(innerPoint, UV);
#ifdef DEBUG
		cout << "InnerUV " << UV[0] << " " << UV[1] << endl;
#endif // DEBUG

		BeSelectModel.model.mesh.request_face_normals();
		BeSelectModel.model.mesh.update_normals();
		BeSelectModel.model.mesh.release_face_normals();

		BeSelectModel.MY_LoadToShader();
	}
	}

void magic_delete()
{
	model.selectedFace.clear();
	model.selectedPoint.clear();
	model.selectPoint_Seq.clear();
	BeSelectModel.model.mesh.clear();
	BeSelectModel.model.mesh.ClearMesh();
	InnerPoint.clear();
	OuterPoint.clear();
	OuterLengh = 0;
}