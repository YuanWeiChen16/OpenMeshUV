#include"FaceData.h"

//計算距離
double FaceData::FaceDistance(FaceData A, FaceData B)
{
	//normal 一致
	if (glm::length(A.realNormal - B.realNormal) < 0.0001)
	{




	}
	else //normal 不一樣
	{
#pragma omp parallel for
		for (int i = 0; i < A.pointcount; i++)
		{
#pragma omp parallel for
			for (int j = 0; j < B.pointcount; j++)
			{

			}
		}
	}



	return 0.0;
}

FaceData FaceData::FaceAdd(FaceData, FaceData)
{
	return FaceData();
}

FaceData FaceData::FaceAvg(FaceData, int)
{
	return FaceData();
}

double FaceData::operator-(FaceData& facedata)
{
	
	return 0;
}

FaceData FaceData::operator+(FaceData& facedata)
{

	return FaceData();
}

