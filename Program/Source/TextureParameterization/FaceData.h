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
class FaceData {
public:
	int ID = -1;
	//raycast�쪺�I���ƶq
	int pointcount = 0;
	//raycast�쪺�I���`��
	std::vector<double> Depth;
	//����normal
	glm::vec3 realNormal;
	//�I�b�ϤW��UV��m�A�p��b�ϤW��m
	std::vector<glm::vec2> UV;

	double FaceData::operator-(FaceData& FaceData);
	
	//�p��⭱���Z���A�ݦҼ{�h�ر��p
	double FaceDistance(FaceData, FaceData);
	//
	FaceData FaceAdd(FaceData, FaceData);
	FaceData FaceAvg(FaceData, int);

	FaceData FaceData::operator+(FaceData& FaceData);
};