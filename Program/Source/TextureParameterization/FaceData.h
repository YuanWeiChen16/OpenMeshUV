#pragma once
#include <Common.h>
#include <iostream>

#include <ViewManager.h>
#include <AntTweakBar/AntTweakBar.h>
#include <ResourcePath.h>
#include <fstream>
#include <string>
#include <map>

#include <Eigen/Sparse>
class FaceData {
public:
	int ID = -1;
	//raycast�쪺�I���ƶq
	int pointcount = 0;
	//raycast�쪺�I���`��
	std::vector<double> Depth;
	//raycast�쪺�I��Normal
	std::vector<glm::vec3> Normal;
	//����normal
	glm::vec3 realNormal;


	double FaceData::operator-(FaceData& FaceData);
	FaceData FaceData::operator+(FaceData& FaceData);
};