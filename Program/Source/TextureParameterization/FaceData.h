#pragma once
#include <Common.h>
#include <iostream>

#include <ViewManager.h>
#include <AntTweakBar/AntTweakBar.h>
#include <ResourcePath.h>
#include <fstream>
#include <string>
#include <map>

#include <omp.h>

#include <Eigen/Sparse>
#include <Eigen/Dense>

//�P�@�����O�b2D��v�W��������A�b�o�̤]�|�������
class FaceData {
public:
	int ID = -1;
	//raycast�쪺�I���ƶq
	int pointcount = 0;
	//raycast�쪺�I���`��
	std::vector<double> Depth;
	
	//�I�b�ϤW��UV��m�A�p��b�ϤW��m
	std::vector<glm::vec2> UV;	


	//�Pnormal�զ� ������{���p��QEM
	//�I���T�������b2D��v�������I
	glm::vec3 position;
	//����normal
	glm::vec3 realNormal;

	//
	glm::vec2 AvgUV;

	//���P�s�����t?
	//�ثe�u�O��ID
	std::vector<int> ConnectFace;
	//�P��L���bUV�W��graph �Z��
	std::map<int, int> Grapth;
	//2D UV bounding box
	//�[�t�p��
	glm::vec2 Max;
	glm::vec2 Min;

	//�w�g��L��dis
	std::map<int, double> PreDis;



	//�p��⭱���Z���A�ݦҼ{�h�ر��p
	double FaceDistance(FaceData, FaceData);
	// 
	//

	double QEM(FaceData);

	FaceData FaceAdd(FaceData, FaceData);
	FaceData FaceAvg(std::vector<FaceData>);

	bool CheckConnect(FaceData);
	


};