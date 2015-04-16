#include "BoundingBoxManagerSingleton.h"

//  BoundingBoxManagerSingleton
BoundingBoxManagerSingleton* BoundingBoxManagerSingleton::m_pInstance = nullptr;
void BoundingBoxManagerSingleton::Init(void)
{
	m_nBoxs = 0;
}
void BoundingBoxManagerSingleton::Release(void)
{
	//Clean the list of Boxs
	for(int n = 0; n < m_nBoxs; n++)
	{
		//Make sure to release the memory of the pointers
		if(m_lBox[n] != nullptr)
		{
			delete m_lBox[n];
			m_lBox[n] = nullptr;
		}
	}
	m_lBox.clear();
	m_lMatrix.clear();
	m_lColor.clear();
	m_nBoxs = 0;
}
BoundingBoxManagerSingleton* BoundingBoxManagerSingleton::GetInstance()
{
	if(m_pInstance == nullptr)
	{
		m_pInstance = new BoundingBoxManagerSingleton();
	}
	return m_pInstance;
}
void BoundingBoxManagerSingleton::ReleaseInstance()
{
	if(m_pInstance != nullptr)
	{
		delete m_pInstance;
		m_pInstance = nullptr;
	}
}
//The big 3
BoundingBoxManagerSingleton::BoundingBoxManagerSingleton(){Init();}
BoundingBoxManagerSingleton::BoundingBoxManagerSingleton(BoundingBoxManagerSingleton const& other){ }
BoundingBoxManagerSingleton& BoundingBoxManagerSingleton::operator=(BoundingBoxManagerSingleton const& other) { return *this; }
BoundingBoxManagerSingleton::~BoundingBoxManagerSingleton(){Release();};
//Accessors
int BoundingBoxManagerSingleton::GetBoxTotal(void){ return m_nBoxs; }

//--- Non Standard Singleton Methods
void BoundingBoxManagerSingleton::GenerateBoundingBox(matrix4 a_mModelToWorld, String a_sInstanceName)
{
	MeshManagerSingleton* pMeshMngr = MeshManagerSingleton::GetInstance();
	//Verify the instance is loaded
	if(pMeshMngr->IsInstanceCreated(a_sInstanceName))
	{//if it is check if the Box has already been created
		int nBox = IdentifyBox(a_sInstanceName);
		if(nBox == -1)
		{
			//Create a new bounding Box
			BoundingBoxClass* pBB = new BoundingBoxClass();
			//construct its information out of the instance name
			pBB->GenerateOrientedBoundingBox(a_sInstanceName);
			//Push the Box back into the list
			m_lBox.push_back(pBB);
			//Push a new matrix into the list
			m_lMatrix.push_back(matrix4(IDENTITY));
			//Specify the color the Box is going to have
			m_lColor.push_back(vector3(1.0f));
			//Increase the number of Boxes
			m_nBoxs++;
		}
		else //If the box has already been created you will need to check its global orientation
		{
			m_lBox[nBox]->GenerateAxisAlignedBoundingBox(a_mModelToWorld);
		}
		nBox = IdentifyBox(a_sInstanceName);
		m_lMatrix[nBox] = a_mModelToWorld;
	}
}

void BoundingBoxManagerSingleton::SetBoundingBoxSpace(matrix4 a_mModelToWorld, String a_sInstanceName)
{
	int nBox = IdentifyBox(a_sInstanceName);
	//If the Box was found
	if(nBox != -1)
	{
		//Set up the new matrix in the appropriate index
		m_lMatrix[nBox] = a_mModelToWorld;
	}
}

int BoundingBoxManagerSingleton::IdentifyBox(String a_sInstanceName)
{
	//Go one by one for all the Boxs in the list
	for(int nBox = 0; nBox < m_nBoxs; nBox++)
	{
		//If the current Box is the one we are looking for we return the index
		if(a_sInstanceName == m_lBox[nBox]->GetName())
			return nBox;
	}
	return -1;//couldn't find it return with no index
}

void BoundingBoxManagerSingleton::AddBoxToRenderList(String a_sInstanceName)
{
	//If I need to render all
	if(a_sInstanceName == "ALL")
	{
		for(int nBox = 0; nBox < m_nBoxs; nBox++)
		{
			m_lBox[nBox]->AddAABBToRenderList(m_lMatrix[nBox], m_lColor[nBox], true);
		}
	}
	else
	{
		int nBox = IdentifyBox(a_sInstanceName);
		if(nBox != -1)
		{
			m_lBox[nBox]->AddAABBToRenderList(m_lMatrix[nBox], m_lColor[nBox], true);
		}
	}
}

void BoundingBoxManagerSingleton::CalculateCollision(void)
{
	//Create a placeholder for all center points
	std::vector<vector3> lCentroid;
	//for all Boxs...
	for(int nBox = 0; nBox < m_nBoxs; nBox++)
	{
		//Make all the Boxs white
		m_lColor[nBox] = vector3(1.0f);
		//Place all the centroids of Boxs in global space
		lCentroid.push_back(static_cast<vector3>(m_lMatrix[nBox] * vector4(m_lBox[nBox]->GetCentroid(), 1.0f)));
	}

	//Now the actual check
	for(int i = 0; i < m_nBoxs - 1; i++)
	{
		for(int j = i + 1; j < m_nBoxs; j++)
		{
			//If the distance between the center of both Boxs is less than the sum of their radius there is a collision
			//For this check we will assume they will be colliding unless they are not in the same space in X, Y or Z
			//so we place them in global positions
			vector3 v1Min = m_lBox[i]->GetMinimumAABB();
			vector3 v1Max = m_lBox[i]->GetMaximumAABB();

			vector3 v2Min = m_lBox[j]->GetMinimumAABB();
			vector3 v2Max = m_lBox[j]->GetMaximumAABB();

			bool bColliding = true;
			//bool bCollidingTwo
			if(v1Max.x < v2Min.x || v1Min.x > v2Max.x)
				bColliding = false;
			else if(v1Max.y < v2Min.y || v1Min.y > v2Max.y)
				bColliding = false;
			else if(v1Max.z < v2Min.z || v1Min.z > v2Max.z)
				bColliding = false;

			//put another if statement inside this one with the new code
			//if(bColliding){
				//TestOBBOBB(i,j);
				if(TestOBBOBB(i,j) == 1){ //another &&, "is this the inside box?"
					m_lColor[i] = m_lColor[j] = MERED; //We make the Boxes red
				}
			//}
		}
	}
}



/*//put another if statement inside this one with the new code
			if(bColliding){

				if(TestOBBOBB(i, j) == 1){
					m_lColor[i] = m_lColor[j] = MERED; //We make the Boxes red
				}
			}
		}
	}
}*/

//Point c = centroid
//Vector u[3] = local x-, y-, and z-axis
//Vector e = Positive halfwidth extents of OBB along each axis

//two indices are the only parameters we need, that way we get matrix easy
//define in .h file
//m_lBox is name of index
int BoundingBoxManagerSingleton::TestOBBOBB(int indexA, int indexB) //takes in two boxes
{
    //we already have centroid
	BoundingBoxClass *a = m_lBox[indexA];
	BoundingBoxClass *b = m_lBox[indexB];
	
	//halfWidth vector
	vector3 e = a->m_v3Size/2.0f;
	vector3 et = b->m_v3Size/2.0f;

	//use a pointer of whatever type you are storing
	//

	//Local axes
	/*a->u[0] = vector3(m_lMatrix[indexA] * vector4(1.0,0.0,0.0,0.0)); //THIS IS THE RIGHT ONE!
	a->u[1] = vector3(m_lMatrix[indexA] * vector4(0.0,1.0,0.0,0.0));
	a->u[2] = vector3(m_lMatrix[indexA] * vector4(0.0,0.0,1.0,0.0));

	b->u[0] = vector3(m_lMatrix[indexB] * vector4(1.0,0.0,0.0,0.0));
	b->u[1] = vector3(m_lMatrix[indexB] * vector4(0.0,1.0,0.0,0.0));
	b->u[2] = vector3(m_lMatrix[indexB] * vector4(0.0,0.0,1.0,0.0));*/

	a->u[0] = vector4(m_lMatrix[indexA] * vector4(1.0,0.0,0.0,0.0)); //THIS IS THE RIGHT ONE!
	a->u[1] = vector4(m_lMatrix[indexA] * vector4(0.0,1.0,0.0,0.0));
	a->u[2] = vector4(m_lMatrix[indexA] * vector4(0.0,0.0,1.0,0.0));

	b->u[0] = vector4(m_lMatrix[indexB] * vector4(1.0,0.0,0.0,0.0));
	b->u[1] = vector4(m_lMatrix[indexB] * vector4(0.0,1.0,0.0,0.0));
	b->u[2] = vector4(m_lMatrix[indexB] * vector4(0.0,0.0,1.0,0.0));
	
	float ra, rb;
    glm::mat3x3 R, AbsR; 

    // Compute rotation matrix expressing b in a's coordinate frame
	//could be "i<3" or "i<axes.length"
    for (int i = 0; i < 3; i++)
       for (int j = 0; j < 3; j++) //R[i][j] = dot(_____)
           R[i][j] = glm::dot(a->u[i], b->u[j]); //?

    //a.u[0] = x-axis
	//a.u[1] = y-axis
	//a.u[2] = z-axis
	// Compute translation vector t
	vector4 t = m_lMatrix[indexB] * vector4(b->m_v3Centroid, 1.0f) - m_lMatrix[indexA] * vector4(a->m_v3Centroid, 1.0f);
    // Bring translation into a's coordinate frame
	//does order of the operands matter?
    t = vector4(glm::dot(a->u[0], t), glm::dot(t, a->u[1]), glm::dot(t, a->u[2]), 1.0f);

    // Compute common subexpressions. Add in an epsilon term to
    // counteract arithmetic errors when two edges are parallel and
    // their cross product is (near) null (see text for details)
    for (int i = 0; i < 3; i++)
       for (int j = 0; j < 3; j++)
		   AbsR[i][j] = abs(R[i][j]) + FLT_EPSILON;

    // Test axes L = A0, L = A1, L = A2
    for (int i = 0; i < 3; i++) {
        ra = e[i];
        rb = et[0] * AbsR[i][0] + et[1] * AbsR[i][1] + et[2] * AbsR[i][2];
        //abs = absolute value
		if (abs(t[i]) > ra + rb) return 0;
    }

    // Test axes L = B0, L = B1, L = B2
    for (int i = 0; i < 3; i++) {
        ra = e[0] * AbsR[0][i] + e[1] * AbsR[1][i] + e[2] * AbsR[2][i];
        rb = et[i];
        if (abs(t[0] * R[0][i] + t[1] * R[1][i] + t[2] * R[2][i]) > ra + rb) return 0;
    }

    // Test axis L = A0 x B0
    ra = e[1] * AbsR[2][0] + e[2] * AbsR[1][0];
    rb = e[1] * AbsR[0][2] + e[2] * AbsR[0][1];
    if (abs(t[2] * R[1][0] - t[1] * R[2][0]) > ra + rb) return 0;

    // Test axis L = A0 x B1
    ra = e[1] * AbsR[2][1] + e[2] * AbsR[1][1];
    rb = et[0] * AbsR[0][2] + et[2] * AbsR[0][0];
    if (abs(t[2] * R[1][1] - t[1] * R[2][1]) > ra + rb) return 0;

    // Test axis L = A0 x B2
    ra = e[1] * AbsR[2][2] + e[2] * AbsR[1][2];
    rb = et[0] * AbsR[0][1] + et[1] * AbsR[0][0];
    if (abs(t[2] * R[1][2] - t[1] * R[2][2]) > ra + rb) return 0;

    // Test axis L = A1 x B0
    ra = e[0] * AbsR[2][0] + e[2] * AbsR[0][0];
    rb = et[1] * AbsR[1][2] + et[2] * AbsR[1][1];
    if (abs(t[0] * R[2][0] - t[2] * R[0][0]) > ra + rb) return 0;

    // Test axis L = A1 x B1
    ra = e[0] * AbsR[2][1] + e[2] * AbsR[0][1];
    rb = et[0] * AbsR[1][2] + et[2] * AbsR[1][0];
    if (abs(t[0] * R[2][1] - t[2] * R[0][1]) > ra + rb) return 0;

    // Test axis L = A1 x B2
    ra = e[0] * AbsR[2][2] + e[2] * AbsR[0][2];
    rb = et[0] * AbsR[1][1] + et[1] * AbsR[1][0];
    if (abs(t[0] * R[2][2] - t[2] * R[0][2]) > ra + rb) return 0;

    // Test axis L = A2 x B0
    ra = e[0] * AbsR[1][0] + e[1] * AbsR[0][0];
    rb = et[1] * AbsR[2][2] + et[2] * AbsR[2][1];
    if (abs(t[1] * R[0][0] - t[0] * R[1][0]) > ra + rb) return 0;

    // Test axis L = A2 x B1
    ra = e[0] * AbsR[1][1] + e[1] * AbsR[0][1];
    rb = et[0] * AbsR[2][2] + et[2] * AbsR[2][0];
    if (abs(t[1] * R[0][1] - t[0] * R[1][1]) > ra + rb) return 0;

    // Test axis L = A2 x B2
    ra = e[0] * AbsR[1][2] + e[1] * AbsR[0][2];
    rb = et[0] * AbsR[2][1] + et[1] * AbsR[2][0];
    if (abs(t[1] * R[0][2] - t[0] * R[1][2]) > ra + rb) return 0;

    // Since no separating axis is found, the OBBs must be intersecting
    return 1;
}