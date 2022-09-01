#include"FaceData.h"

//�p��Z��
double FaceData::FaceDistance(FaceData A, FaceData B)
{
	int TotalPointCount = A.pointcount * B.pointcount;
	//Depth �Z��
	double DepthTerm = 0;
	//normal �@�P
	int NormalDiffFlag = 0;
	if (glm::length(A.realNormal - B.realNormal) < 0.0001)
	{
		double DepthDis = A.Depth[0] - B.Depth[0];
		DepthTerm = DepthDis * TotalPointCount;
	}
	else //normal ���@��
	{
		NormalDiffFlag = 1;
	}

	//UV �Z��
	double UVterm = 0;
#pragma omp parallel for
	for (int i = 0; i < A.pointcount; i++)
	{
#pragma omp parallel for
		for (int j = 0; j < B.pointcount; j++)
		{
			// UV Distance
			double DisX = A.UV[i][0] - B.UV[j][0];
			double DisY = A.UV[i][1] - B.UV[j][1];
			double UVDistance = sqrt(DisX * DisX + DisY * DisY);
			//Depth Distance
			UVterm += UVDistance;

			if (NormalDiffFlag == 1)
			{
				double DepthDis = A.Depth[i] - B.Depth[j];
				DepthTerm += DepthDis;
			}
		}
	}


	//UV Normalize?
	//UVterm /= (double)(TotalPointCount);

	//Noraml �Z��
	double NormalTerm = glm::length(A.realNormal - B.realNormal)*((double)(TotalPointCount));

	//�u���Z��Term
	return DepthTerm + UVterm + NormalTerm;

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

