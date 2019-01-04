#include"ElfreinaMesh.hpp"
#include"DirectGraphics.hpp"
#include"DxUtility.hpp"

using namespace gstd;
using namespace directx;

/**********************************************************
//ElfreinaMesh
**********************************************************/
//ElfreinaMeshData
ElfreinaMeshData::ElfreinaMeshData()
{
}
ElfreinaMeshData::~ElfreinaMeshData()
{

}
bool ElfreinaMeshData::CreateFromFileReader(gstd::ref_count_ptr<gstd::FileReader> reader)
{
	bool res = false;
	path_ = reader->GetOriginalPath();
	int size = reader->GetFileSize();
	std::vector<char> textUtf8;
	textUtf8.resize(size+1);
	reader->Read(&textUtf8[0], size);
	textUtf8[size] = '\0';

	//�����R�[�h�ϊ�
	std::wstring text = StringUtility::ConvertUtf8ToWide(textUtf8);

	gstd::Scanner scanner(text);
	try
	{
		while(scanner.HasNext())
		{
			gstd::Token& tok = scanner.Next();
			if(tok.GetElement() == L"MeshContainer")
			{
				scanner.CheckType(scanner.Next(), Token::TK_OPENC);
				_ReadMeshContainer(scanner);
			}
			else if(tok.GetElement() == L"HierarchyList")
			{
				scanner.CheckType(scanner.Next(), Token::TK_OPENC);
				_ReadHierarchyList(scanner);
			}
			else if(tok.GetElement() == L"AnimationList")
			{
				scanner.CheckType(scanner.Next(), Token::TK_OPENC);
				_ReadAnimationList(scanner);
			}
		}

		//���_�o�b�t�@�쐬
		for(int iMesh = 0 ; iMesh<mesh_.size(); iMesh++)
		{
			mesh_[iMesh]->InitializeVertexBuffer();
		}

		res = true;
	}
	catch(gstd::wexception& e)
	{
		Logger::WriteTop(StringUtility::Format(L"ElfreinaMeshData�ǂݍ��ݎ��s %s %d", e.what(), scanner.GetCurrentLine()));
		res = false;
	}	

	if(false)
	{
		Logger::WriteTop(StringUtility::Format(L"ElfreinaMeshData�ǂݍ��ݐ���[%s]", reader->GetOriginalPath().c_str()));
		for(int iMesh = 0 ; iMesh<mesh_.size(); iMesh++)
		{
			std::wstring name = mesh_[iMesh]->name_;
			std::wstring log = L" Mesh ";
			log += StringUtility::Format(L"name[%s] ",name.c_str());
			log += StringUtility::Format(L"vert[%d] ",mesh_[iMesh]->GetVertexCount());
			Logger::WriteTop(log);

			if(iMesh == 6)
			{
				int countVert = mesh_[iMesh]->GetVertexCount();
				int iVert=0;
				for(iVert=0;iVert<countVert;iVert++)
				{
					VERTEX_B4NX* vert = mesh_[iMesh]->GetVertex(iVert);
					//VERTEX_B2NX* vert = mesh_[iMesh]->GetVertex(iVert);
					log = L"  vert ";
					log += StringUtility::Format(L"pos[%f,%f,%f] ",vert->position.x, vert->position.y, vert->position.z);
					log += StringUtility::Format(L"blend1[%d,%f] ",BitAccess::GetByte(vert->blendIndex, 0), vert->blendRate);
					log += StringUtility::Format(L"blend2[%d] ",BitAccess::GetByte(vert->blendIndex, 4));
					Logger::WriteTop(log);
				}

				int iFace = 0;
				std::vector<short>& indexVert = mesh_[iMesh]->vertexIndices_;
				int countFace = mesh_[iMesh]->vertexIndices_.size()/3;
				for(iFace=0;iFace<countFace;iFace++)
				{
					log = L"  vert ";
					log += StringUtility::Format(L"index[%d,%d,%d] ",
						indexVert[iFace*3], indexVert[iFace*3+1], indexVert[iFace*3+2]);
					Logger::WriteTop(log);
				}
			
			}
		}
		for(int iBone = 0; iBone<bone_.size(); iBone++)
		{
			std::wstring log = L" Bone ";
			std::wstring name = bone_[iBone]->name_;
			log += StringUtility::Format(L"name[%s] ",name.c_str());
			Logger::WriteTop(log);
		}

		std::map<std::wstring, gstd::ref_count_ptr<AnimationData> >::iterator itr;
		for(itr = anime_.begin();itr!=anime_.end();itr++)
		{
			std::wstring name = itr->first;
			std::wstring log = L" Anime ";
			log += StringUtility::Format(L"name[%s] ",name.c_str());
			Logger::WriteTop(log);

		}
	}

	return res;
}
void ElfreinaMeshData::_ReadMeshContainer(gstd::Scanner& scanner)
{
	int countReadMesh = 0;
	while(scanner.HasNext())
	{
		gstd::Token& tok = scanner.Next();
		if(tok.GetType() == Token::TK_CLOSEC)break;

		if(tok.GetElement() == L"Name")
		{
		}
		else if(tok.GetElement() == L"BoneCount")
		{
			scanner.CheckType(scanner.Next(), Token::TK_EQUAL);
			tok = scanner.Next();
			int countBone = tok.GetInteger();
			bone_.resize(countBone);
			for(int iBone = 0; iBone<countBone; iBone++)
			{
				bone_[iBone] = new Bone();
			}
		}
		else if(tok.GetElement() == L"MeshCount")
		{
			scanner.CheckType(scanner.Next(), Token::TK_EQUAL);
			tok = scanner.Next();
			int countObj = tok.GetInteger();
			mesh_.resize(countObj);
			for(int iObj=0; iObj<countObj; iObj++)
			{
				mesh_[iObj] = new Mesh();
			}
		}
		else if(tok.GetElement() == L"VertexFormat")
		{
			scanner.CheckType(scanner.Next(), Token::TK_OPENC);
			while(true)
			{
				if(scanner.Next().GetType() == Token::TK_CLOSEC)break;
			}
		}
		else if(tok.GetElement() == L"BoneNames")
		{
			scanner.CheckType(scanner.Next(), Token::TK_OPENC);
			int iBone = 0;
			while(true)
			{
				tok = scanner.Next();
				if(tok.GetType() == Token::TK_CLOSEC)break;
				if(tok.GetType() != Token::TK_STRING)continue;
				bone_[iBone]->name_ = tok.GetString();
				iBone++;
			}
		}
		else if(tok.GetElement() == L"OffsetMatrices")
		{
			int iCount = 0;
			int iBone = -1;
			scanner.CheckType(scanner.Next(), Token::TK_OPENC);
			while(true)
			{
				tok = scanner.Next();
				if(tok.GetType() == Token::TK_CLOSEC)break;

				if(tok.GetType() == Token::TK_NEWLINE)
				{
					if(iCount!= 0 && iBone != -1 && iCount!=16)
						throw gstd::wexception(L"ElfreinaMeshData:OffsetMatrices�̐����s���ł�");			
					iCount = 0;
					iBone++;
				}

				if(tok.GetType() != Token::TK_INT && 
					tok.GetType() != Token::TK_REAL)continue;

				bone_[iBone]->matOffset_.m[iCount/4][iCount%4] = tok.GetReal();
				iCount++;
			}
		}
		else if(tok.GetElement() == L"Materials")
		{
			int posMat = 0;
			scanner.CheckType(scanner.Next(), Token::TK_OPENC);
			while(true)
			{
				tok = scanner.Next();
				if(tok.GetType() == Token::TK_CLOSEC)break;
				if(tok.GetElement() == L"MaterialCount")
				{
					scanner.CheckType(scanner.Next(), Token::TK_EQUAL);
					tok = scanner.Next();
					int countMaterial = tok.GetInteger();
					material_.resize(countMaterial);
					for(int iMat=0; iMat<countMaterial; iMat++)
					{
						material_[iMat] = new Material();
					}
				}
				else if(tok.GetElement() == L"Material")
				{
					scanner.CheckType(scanner.Next(), Token::TK_OPENC);
					_ReadMaterials(scanner, *material_[posMat].GetPointer());
					posMat++;
				}
			}
		}
		else if(tok.GetElement() == L"Mesh")
		{
			scanner.CheckType(scanner.Next(), Token::TK_OPENC);
			_ReadMesh(scanner, *mesh_[countReadMesh].GetPointer());
			countReadMesh++;
		}
	}
}
void ElfreinaMeshData::_ReadMaterials(gstd::Scanner& scanner, Material& material)
{
	while(true)
	{
		gstd::Token& tok = scanner.Next();
		if(tok.GetType() == Token::TK_CLOSEC)break;
		
		if(tok.GetElement() == L"Name")
		{
			scanner.CheckType(scanner.Next(), Token::TK_EQUAL);
			tok = scanner.Next();
			material.name_ = tok.GetString();
		}
		else if(tok.GetElement() == L"Diffuse" ||
				tok.GetElement() == L"Ambient" ||
				tok.GetElement() == L"Emissive" ||
				tok.GetElement() == L"Specular")
		{
			D3DMATERIAL9& mat = material.mat_;
			D3DCOLORVALUE* color;
			if(tok.GetElement() == L"Diffuse")color = &mat.Diffuse;
			else if(tok.GetElement() == L"Ambient")color = &mat.Ambient;
			else if(tok.GetElement() == L"Emissive")color = &mat.Emissive;
			else if(tok.GetElement() == L"Specular")color = &mat.Specular;

			scanner.CheckType(scanner.Next(), Token::TK_EQUAL);
			color->a = scanner.Next().GetReal();
			scanner.CheckType(scanner.Next(), Token::TK_COLON);
			color->r = scanner.Next().GetReal();
			scanner.CheckType(scanner.Next(), Token::TK_COLON);
			color->g = scanner.Next().GetReal();
			scanner.CheckType(scanner.Next(), Token::TK_COLON);
			color->b = scanner.Next().GetReal();
		}
		else if(tok.GetElement() == L"SpecularSharpness")
		{
		}
		else if(tok.GetElement() == L"TextureFilename")
		{
			scanner.CheckType(scanner.Next(), Token::TK_EQUAL);
			tok = scanner.Next();

			std::wstring wPathTexture = tok.GetString();
			std::wstring path = PathProperty::GetFileDirectory(path_) + wPathTexture;
			material.texture_ = new Texture();
			material.texture_->CreateFromFile(path);
		}
	}
}
void ElfreinaMeshData::_ReadMesh(gstd::Scanner& scanner, Mesh& mesh)
{
	std::vector<std::vector<Mesh::BoneWeightData> > vectWeight;
	while(true)
	{
		gstd::Token& tok = scanner.Next();
		if(tok.GetType() == Token::TK_CLOSEC)break;
		
		if(tok.GetElement() == L"Name")
		{
			scanner.CheckType(scanner.Next(), Token::TK_EQUAL);
			tok = scanner.Next();
			mesh.name_ = tok.GetString();
		}
		else if(tok.GetElement() == L"VertexCount")
		{
			scanner.CheckType(scanner.Next(), Token::TK_EQUAL);
			tok = scanner.Next();
			int countVertex = tok.GetInteger();
			mesh.SetVertexCount(countVertex);
			vectWeight.resize(countVertex);
		}
		else if(tok.GetElement() == L"FaceCount")
		{
			scanner.CheckType(scanner.Next(), Token::TK_EQUAL);
			tok = scanner.Next();
			int countFace= tok.GetInteger();
			mesh.vertexIndices_.resize(countFace*3);
		}
		else if(tok.GetElement() == L"Positions" || tok.GetElement() == L"Normals")
		{
			bool bPosition = tok.GetElement() == L"Positions";
			bool bNormal = tok.GetElement() == L"Normals";
			scanner.CheckType(scanner.Next(), Token::TK_OPENC);
			scanner.CheckType(scanner.Next(), Token::TK_NEWLINE);

			int countVertex = mesh.GetVertexCount();
			for(int iVert = 0 ; iVert < countVertex; iVert++)
			{
				float x = scanner.Next().GetReal();
				scanner.CheckType(scanner.Next(), Token::TK_COLON);
				float y = scanner.Next().GetReal();
				scanner.CheckType(scanner.Next(), Token::TK_COLON);
				float z = scanner.Next().GetReal();
				scanner.CheckType(scanner.Next(), Token::TK_NEWLINE);
				if(bPosition)
					mesh.SetVertexPosition(iVert, x, y, z);
				else if(bNormal)
					mesh.SetVertexNormal(iVert, x, y, z);
			}
			scanner.CheckType(scanner.Next(), Token::TK_CLOSEC);
		}
		else if(tok.GetElement() == L"BlendList")
		{
			scanner.CheckType(scanner.Next(), Token::TK_OPENC);
			scanner.CheckType(scanner.Next(), Token::TK_NEWLINE);
			while(true)
			{
				gstd::Token& tok = scanner.Next();
				if(tok.GetType() == Token::TK_CLOSEC)break;
				if(tok.GetElement() == L"BlendPart")
				{
					scanner.CheckType(scanner.Next(), Token::TK_OPENC);
					scanner.CheckType(scanner.Next(), Token::TK_NEWLINE);

					scanner.CheckIdentifer(scanner.Next(), L"BoneName");
					scanner.CheckType(scanner.Next(), Token::TK_EQUAL);
					std::wstring name = scanner.Next().GetString();

					scanner.CheckType(scanner.Next(), Token::TK_NEWLINE);
					scanner.CheckIdentifer(scanner.Next(), L"TransformIndex");
					scanner.CheckType(scanner.Next(), Token::TK_EQUAL);
					int indexBone = scanner.Next().GetInteger();

					scanner.CheckType(scanner.Next(), Token::TK_NEWLINE);
					scanner.CheckIdentifer(scanner.Next(), L"VertexBlend");
					scanner.CheckType(scanner.Next(), Token::TK_OPENC);
					Bone* bone = bone_[indexBone].GetPointer();
					bone->name_ = name;

					while(true)
					{
						tok = scanner.Next();
						if(tok.GetType() == Token::TK_CLOSEC)break;
						if(tok.GetType() != Token::TK_INT && tok.GetType() != Token::TK_REAL)continue;

						int index = tok.GetInteger();
						scanner.CheckType(scanner.Next(), Token::TK_COMMA);
						float weight = scanner.Next().GetReal();

						Mesh::BoneWeightData data;
						data.index = indexBone;
						data.weight = weight;
						vectWeight[index].push_back(data);
					}
					scanner.CheckType(scanner.Next(), Token::TK_NEWLINE);
					scanner.CheckType(scanner.Next(), Token::TK_CLOSEC);
				}
			}

		}
		else if(tok.GetElement() == L"TextureUV")
		{
			scanner.CheckType(scanner.Next(), Token::TK_OPENC);
			scanner.CheckType(scanner.Next(), Token::TK_NEWLINE);

			int countVertex = mesh.GetVertexCount();
			for(int iVert = 0 ; iVert < countVertex; iVert++)
			{
				float u = scanner.Next().GetReal();
				scanner.CheckType(scanner.Next(), Token::TK_COLON);
				float v = scanner.Next().GetReal();
				scanner.CheckType(scanner.Next(), Token::TK_NEWLINE);
				mesh.SetVertexUV(iVert, u, v);
			}
			scanner.CheckType(scanner.Next(), Token::TK_CLOSEC);
		}
		else if(tok.GetElement() == L"VertexIndices")
		{
			scanner.CheckType(scanner.Next(), Token::TK_OPENC);
			scanner.CheckType(scanner.Next(), Token::TK_NEWLINE);

			int countFace = mesh.vertexIndices_.size() / 3;
			for(int iFace = 0 ; iFace < countFace; iFace++)
			{
				int face = scanner.Next().GetInteger();
				if(face !=3)throw gstd::wexception(L"3���_�Ō`������Ă��Ȃ��ʂ�����܂�");
				scanner.CheckType(scanner.Next(), Token::TK_COMMA);
				mesh.vertexIndices_[iFace*3] = scanner.Next().GetInteger();
				scanner.CheckType(scanner.Next(), Token::TK_COLON);
				mesh.vertexIndices_[iFace*3+1] = scanner.Next().GetInteger();
				scanner.CheckType(scanner.Next(), Token::TK_COLON);
				mesh.vertexIndices_[iFace*3+2] = scanner.Next().GetInteger();
				scanner.CheckType(scanner.Next(), Token::TK_NEWLINE);
			}
			scanner.CheckType(scanner.Next(), Token::TK_CLOSEC);
		}
		else if(tok.GetElement() == L"Attributes")
		{
			int attr = -1;
			scanner.CheckType(scanner.Next(), Token::TK_OPENC);
			scanner.CheckType(scanner.Next(), Token::TK_NEWLINE);
			int countFace = mesh.vertexIndices_.size() / 3;
			for(int iFace = 0 ; iFace < countFace; iFace++)
			{
				int index = scanner.Next().GetInteger();
				if(attr ==-1)attr = index;
				if(attr != index)throw gstd::wexception(L"1�I�u�W�F�N�g�ɕ����}�e���A���͔�Ή�");
				scanner.CheckType(scanner.Next(), Token::TK_NEWLINE);
			}
			mesh.material_ = material_[attr];
			scanner.CheckType(scanner.Next(), Token::TK_CLOSEC);
		}
	}

	//�{�[���E�F�C�g�f�[�^����
	int boneCount = bone_.size();
	std::vector<double> totalWeight;
	totalWeight.resize(boneCount);
	ZeroMemory(&totalWeight[0], sizeof(double) * bone_.size());
	for(int iVert = 0 ; iVert < vectWeight.size(); iVert++)
	{
		std::vector<Mesh::BoneWeightData>& datas = vectWeight[iVert];
		if(false)
		{
			//BLEND2
			Mesh::BoneWeightData* data1 = NULL;
			Mesh::BoneWeightData* data2 = NULL;
			for(int iDatas=0; iDatas<datas.size(); iDatas++)
			{
				Mesh::BoneWeightData* data = &datas[iDatas];
				totalWeight[data->index] += data->weight;
				if(data1 == NULL)data1 = data;
				else if(data2 == NULL)
				{
					data2 = data;
					if(data1->weight < data2->weight)
					{
						Mesh::BoneWeightData* temp = data1;
						data1 = data2;
						data2 = temp;
					}
				}
				else
				{
					if(data1->weight < data->weight)
					{
						data2 = data1;
						data1 = data;
					}
					else if(data2->weight < data->weight)
					{
						data2 = data;
					}
				}
			}
		
			//���_�f�[�^�ɐݒ�
			if(data1 != NULL && data2 != NULL)
			{
				float sub = 1.0f - data1->weight - data2->weight;
				data1->weight += sub/2.0f;
				data2->weight += sub/2.0f;
				if(data1->weight > 1.0f)data1->weight=1.0f;
				if(data2->weight > 1.0f)data2->weight=1.0f;
			}

			if(data1 != NULL)mesh.SetVertexBlend(iVert, 0, data1->index, data1->weight);
			if(data2 != NULL)mesh.SetVertexBlend(iVert, 1, data2->index, data2->weight);
		}
		else
		{
			int dataCount = datas.size();
			for(int iDatas=0; iDatas<dataCount; iDatas++)
			{
				Mesh::BoneWeightData* data = &datas[iDatas];
				if(data == NULL)continue;
				totalWeight[data->index] += data->weight;
				mesh.SetVertexBlend(iVert, iDatas, data->index, data->weight);
			}
		}
	}

	//�d�S
	double maxWeight = 0;
	mesh.CalculateWeightCenter();
	for(int iBone = 0 ; iBone < totalWeight.size() ; iBone++)
	{
		if(totalWeight[iBone] < maxWeight)continue;
		mesh.indexWeightForCalucZValue_ = iBone;
		maxWeight = totalWeight[iBone];
	}
}

void ElfreinaMeshData::_ReadHierarchyList(gstd::Scanner& scanner)
{
	for(int iBone = 0 ; iBone<bone_.size(); iBone++)
	{
		std::wstring name = bone_[iBone]->name_;
		mapBoneNameIndex_[name] = iBone;
	}

	while(true)
	{
		gstd::Token& tok = scanner.Next();
		if(tok.GetType() == Token::TK_CLOSEC)break;
		if(tok.GetElement() == L"Node")
		{
			scanner.CheckType(scanner.Next(), Token::TK_OPENC);
			scanner.CheckType(scanner.Next(), Token::TK_NEWLINE);
			_ReadNode(scanner, Bone::NO_PARENT);
		}
	}
}
int ElfreinaMeshData::_ReadNode(gstd::Scanner& scanner, int parent)
{
	int indexBone = -1;
	Bone* bone = NULL;
	while(true)
	{
		gstd::Token& tok = scanner.Next();
		if(tok.GetType() == Token::TK_CLOSEC)break;
		if(tok.GetElement() == L"NodeName")
		{
			scanner.CheckType(scanner.Next(), Token::TK_EQUAL);
			std::wstring name = scanner.Next().GetString();
			indexBone = mapBoneNameIndex_[name];
			bone = bone_[indexBone].GetPointer();
			bone->indexParent_ = parent;
		}
		else if(tok.GetElement() == L"InitPostureMatrix")
		{
			scanner.CheckType(scanner.Next(), Token::TK_EQUAL);
			for(int iCount = 0; iCount<16; iCount++)
			{
				bone->matInitPosture_.m[iCount/4][iCount%4] = scanner.Next().GetReal();
				if(iCount<=14)scanner.CheckType(scanner.Next(), Token::TK_COLON);
			}
			scanner.CheckType(scanner.Next(), Token::TK_NEWLINE);
		}
		if(tok.GetElement() == L"Node")
		{
			scanner.CheckType(scanner.Next(), Token::TK_OPENC);
			scanner.CheckType(scanner.Next(), Token::TK_NEWLINE);
			int indexChild = _ReadNode(scanner, indexBone);
			bone->indexChild_.push_back(indexChild);
		}
	}
	return indexBone;
}

void ElfreinaMeshData::_ReadAnimationList(gstd::Scanner& scanner)
{
	while(true)
	{
		gstd::Token& tok = scanner.Next();
		if(tok.GetType() == Token::TK_CLOSEC)break;
		if(tok.GetElement() == L"AnimationCount")
		{
			scanner.CheckType(scanner.Next(), Token::TK_EQUAL);
			scanner.Next().GetInteger();
		}
		else if(tok.GetElement() == L"AnimationData")
		{
			scanner.CheckType(scanner.Next(), Token::TK_OPENC);
			scanner.CheckType(scanner.Next(), Token::TK_NEWLINE);
			gstd::ref_count_ptr<AnimationData> anime = _ReadAnimationData(scanner);
			if(anime != NULL)
			{
				anime_[anime->name_] = anime;
			}
		}
	}
}
gstd::ref_count_ptr<ElfreinaMeshData::AnimationData> ElfreinaMeshData::_ReadAnimationData(gstd::Scanner& scanner)
{
	gstd::ref_count_ptr<AnimationData> anime = new AnimationData();
	anime->animeBone_.resize(bone_.size());
	AnimationData* pAnime = anime.GetPointer();
	while(true)
	{
		gstd::Token& tok = scanner.Next();
		if(tok.GetType() == Token::TK_CLOSEC)break;
		if(tok.GetElement() == L"AnimationName")
		{
			scanner.CheckType(scanner.Next(), Token::TK_EQUAL);
			pAnime->name_ = scanner.Next().GetString();
		}
		else if(tok.GetElement() == L"AnimationTime")
		{
			scanner.CheckType(scanner.Next(), Token::TK_EQUAL);
			pAnime->timeTotal_ = scanner.Next().GetInteger();
		}
		else if(tok.GetElement() == L"FrameParSecond")
		{
			scanner.CheckType(scanner.Next(), Token::TK_EQUAL);
			pAnime->framePerSecond_ = scanner.Next().GetInteger();
		}
		else if(tok.GetElement() == L"TransitionTime")
		{
			scanner.CheckType(scanner.Next(), Token::TK_EQUAL);
		}
		else if(tok.GetElement() == L"Priority")
		{
			scanner.CheckType(scanner.Next(), Token::TK_EQUAL);
		}
		else if(tok.GetElement() == L"Loop")
		{
			scanner.CheckType(scanner.Next(), Token::TK_EQUAL);
			std::wstring id = scanner.Next().GetElement();
			pAnime->bLoop_ = id == L"True";
		}
		else if(tok.GetElement() == L"BoneAnimation")
		{
			scanner.CheckType(scanner.Next(), Token::TK_OPENC);
			scanner.CheckType(scanner.Next(), Token::TK_NEWLINE);
			_ReadBoneAnimation(scanner, *pAnime);
		}
		else if(tok.GetElement() == L"UVAnimation")
		{
			// TODO
			int count = 1;
			scanner.CheckType(scanner.Next(), Token::TK_OPENC);
			scanner.CheckType(scanner.Next(), Token::TK_NEWLINE);
			while(true)
			{
				tok = scanner.Next();
				if(tok.GetType() == Token::TK_OPENC)count++;
				else if(tok.GetType() == Token::TK_CLOSEC)count--;
				if(count == 0)break;
			}
		}
	}
	return anime;
}
void ElfreinaMeshData::_ReadBoneAnimation(gstd::Scanner& scanner, AnimationData& anime)
{
	while(true)
	{
		gstd::Token& tok = scanner.Next();
		if(tok.GetType() == Token::TK_CLOSEC)break;
		if(tok.GetElement() == L"TimeKeys")
		{
			scanner.CheckType(scanner.Next(), Token::TK_OPENC);
			scanner.CheckType(scanner.Next(), Token::TK_NEWLINE);
			while(scanner.Next().GetType() != Token::TK_CLOSEC){}
		}
		else if(tok.GetElement() == L"AnimationPart")
		{
			scanner.CheckType(scanner.Next(), Token::TK_OPENC);
			scanner.CheckType(scanner.Next(), Token::TK_NEWLINE);
			_ReadBoneAnimationPart(scanner, anime);
		}
	}
}
void ElfreinaMeshData::_ReadBoneAnimationPart(gstd::Scanner& scanner, AnimationData& anime)
{
	int indexBone = -1;
	BoneAnimationPart* part = new BoneAnimationPart();
	while(true)
	{
		gstd::Token& tok = scanner.Next();
		if(tok.GetType() == Token::TK_CLOSEC)break;
		if(tok.GetElement() == L"NodeName")
		{
			scanner.CheckType(scanner.Next(), Token::TK_EQUAL);
			std::wstring name = scanner.Next().GetString();
			indexBone = mapBoneNameIndex_[name];
		}
		else if(tok.GetElement() == L"TimeKeys")
		{
			scanner.CheckType(scanner.Next(), Token::TK_OPENC);
			scanner.CheckType(scanner.Next(), Token::TK_NEWLINE);
			while(true)
			{
				gstd::Token& tok = scanner.Next();
				if(tok.GetType() == Token::TK_CLOSEC)break;
				if(tok.GetType() != Token::TK_INT && tok.GetType() != Token::TK_REAL)continue;
				part->keyTime_.push_back(tok.GetReal());
			}
		}
		else if(tok.GetElement() == L"TransKeys"||
			tok.GetElement() == L"ScaleKeys" ||
			tok.GetElement() == L"RotateKeys")
		{
			bool bTrans = tok.GetElement() == L"TransKeys";
			bool bScale = tok.GetElement() == L"ScaleKeys";
			bool bRotate = tok.GetElement() == L"RotateKeys";
			scanner.CheckType(scanner.Next(), Token::TK_OPENC);
			scanner.CheckType(scanner.Next(), Token::TK_NEWLINE);
			while(true)
			{
				gstd::Token& tok = scanner.Next();
				if(tok.GetType() == Token::TK_CLOSEC)break;
				if(tok.GetType() != Token::TK_INT && tok.GetType() != Token::TK_REAL)continue;
				float x = tok.GetReal();
				scanner.CheckType(scanner.Next(), Token::TK_COLON);
				float y = scanner.Next().GetReal();
				scanner.CheckType(scanner.Next(), Token::TK_COLON);
				float z = scanner.Next().GetReal();
				
				if(bTrans || bScale)
				{
					D3DXVECTOR3 vect(x, y, z);
					if(bTrans)part->keyTrans_.push_back(vect);
					else if(bScale)part->keyScale_.push_back(vect);
				}
				else if(bRotate)
				{
					scanner.CheckType(scanner.Next(), Token::TK_COLON);
					float w = scanner.Next().GetReal();

					D3DXQUATERNION vect(x, y, z, w);
					part->keyRotate_.push_back(vect);
				}

				scanner.CheckType(scanner.Next(), Token::TK_NEWLINE);
			}
		}
	}
	if(indexBone != -1)
		anime.animeBone_[indexBone] = part;
}


//ElfreinaMeshData::Mesh
ElfreinaMeshData::Mesh::Mesh()
{
	SetPrimitiveType(D3DPT_TRIANGLELIST);
}
ElfreinaMeshData::Mesh::~Mesh()
{
}
void ElfreinaMeshData::Mesh::Render()
{
	IDirect3DDevice9* device = DirectGraphics::GetBase()->GetDevice();
	ElfreinaMeshData::Material* material = material_.GetPointer();
	SetTexture(material->texture_);
	materialBNX_ = material->mat_;
	device->SetMaterial(&materialBNX_);
	RenderObjectB4NX::Render();
	//RenderObjectB2NX::Render();
/*
	{
		RenderObjectLX obj;
		int count = GetVertexCount();
		obj.SetVertexCount(count);
		for(int iVert=0;iVert<count;iVert++)
		{
			VERTEX_B2NX* b2 = GetVertex(iVert);
			obj.SetVertexPosition(iVert, b2->position.x, b2->position.y, b2->position.z);
			obj.SetVertexUV(iVert, b2->texcoord.x, b2->texcoord.y);
			obj.SetVertexAlpha(iVert, 255);
			obj.SetVertexColorARGB(iVert,255,255,255,255);
		}
		obj.SetVertexIndicies(vertexIndices_);
		obj.SetTexture(mat->texture_);
		obj.Render();
	}
*/
}
gstd::ref_count_ptr<RenderBlock> ElfreinaMeshData::Mesh::CreateRenderBlock()
{
	gstd::ref_count_ptr<RenderObjectElfreinaBlock> res = new RenderObjectElfreinaBlock();
	res->SetPosition(position_);
	res->SetAngle(angle_);
	res->SetScale(scale_);
	res->SetMatrix(matrix_);
	return res;
}


//ElfreinaMeshData::AnimationData
gstd::ref_count_ptr<Matrices> ElfreinaMeshData::AnimationData::CreateBoneAnimationMatrix(double time, ElfreinaMeshData* mesh)
{
	std::vector<gstd::ref_count_ptr<Bone> >& bones = mesh->GetBones();

	gstd::ref_count_ptr<Matrices> matrix = new Matrices();
	matrix->SetSize(bones.size());

	//�ŏ�ʂ̐e��T��
	for(int iBone = 0; iBone < bones.size() ; iBone++)
	{
		if(bones[iBone]->GetParentIndex() != ElfreinaMeshData::Bone::NO_PARENT)continue;

		D3DXMATRIX& matInit = bones[iBone]->GetInitPostureMatrix();
		D3DXMATRIX& matOffset = bones[iBone]->GetOffsetMatrix();
		D3DXMATRIX mat = _CalculateMatrix(time, iBone) * matInit;

		//���g�̍s���ݒ�
		matrix->SetMatrix(iBone, matOffset * mat);

		//�q�̍s����v�Z
		std::vector<int>& indexChild = bones[iBone]->GetChildIndex();
		for(int iChild = 0 ; iChild < indexChild.size(); iChild++)
		{
			int index = indexChild[iChild];
			_CreateBoneAnimationMatrix(time, mesh, matrix, index, mat);
		}
	}

	return matrix;
}
void ElfreinaMeshData::AnimationData::_CreateBoneAnimationMatrix(int time, ElfreinaMeshData* mesh, gstd::ref_count_ptr<Matrices> matrix, int indexOwn, D3DXMATRIX& matrixParentAnime)
{
	std::vector<gstd::ref_count_ptr<Bone> >& bones = mesh->GetBones();

	D3DXMATRIX& matInit = bones[indexOwn]->GetInitPostureMatrix();
	D3DXMATRIX& matOffset = bones[indexOwn]->GetOffsetMatrix();
	D3DXMATRIX mat = _CalculateMatrix(time, indexOwn) * matInit * matrixParentAnime;

	//���g�̍s���ݒ�
	matrix->SetMatrix(indexOwn, matOffset * mat);

	//�q�̍s����v�Z
	std::vector<int>& indexChild = bones[indexOwn]->GetChildIndex();
	for(int iChild = 0 ; iChild < indexChild.size(); iChild++)
	{
		int index = indexChild[iChild];
		_CreateBoneAnimationMatrix(time, mesh, matrix, index, mat);
	}
}
D3DXMATRIX ElfreinaMeshData::AnimationData::_CalculateMatrix(double time, int index)
{
	D3DXMATRIX res;
	BoneAnimationPart* part = animeBone_[index].GetPointer();
	if(part == NULL)
	{
		D3DXMatrixIdentity(&res);
		return res;
	}

	std::vector<float>& keyTime = part->GetTimeKey();

	//�A�j���[�V�����C���f�b�N�X������
	int keyNext = -1;
	int keyPrevious = -1;
	for(int iTime = 0 ; iTime < keyTime.size(); iTime++)
	{
		int tTime = timeTotal_ * keyTime[iTime];
		if(tTime < time)continue;
		keyPrevious = iTime - 1;
		keyNext = iTime;
		break;
	}
	if(keyNext == -1)
	{
		keyNext = keyTime.size() - 1;
		keyPrevious = keyNext - 1;
	}
	if(keyPrevious < 0)keyPrevious = 0;
	if(keyNext < 0 )keyNext = 0;

	float timeDiffKey = (float)timeTotal_ * keyTime[keyNext] - (float)timeTotal_ * keyTime[keyPrevious];
	float timeDiffAnime = (float)(time - timeTotal_ * keyTime[keyPrevious]);
	float rateToNext = timeDiffKey != 0.0f ? timeDiffAnime / timeDiffKey : 0;
	
	//�e�s����쐬
	std::vector<D3DXVECTOR3>& keyTrans = part->GetTransKey();
	D3DXMATRIX matTrans;
	D3DXMatrixIdentity(&matTrans);
	D3DXVECTOR3 pos;
	pos.x = keyTrans[keyPrevious].x + (keyTrans[keyNext].x-keyTrans[keyPrevious].x) * rateToNext;
	pos.y = keyTrans[keyPrevious].y + (keyTrans[keyNext].y-keyTrans[keyPrevious].y) * rateToNext;
	pos.z = keyTrans[keyPrevious].z + (keyTrans[keyNext].z-keyTrans[keyPrevious].z) * rateToNext;
	D3DXMatrixTranslation(&matTrans, pos.x, pos.y, pos.z);

	std::vector<D3DXVECTOR3>& keyScale = part->GetScaleKey();
	D3DXMATRIX matScale;
	D3DXMatrixIdentity(&matScale);
	D3DXVECTOR3 scale;
	scale.x = keyScale[keyPrevious].x + (keyScale[keyNext].x-keyScale[keyPrevious].x) * rateToNext;
	scale.y = keyScale[keyPrevious].y + (keyScale[keyNext].y-keyScale[keyPrevious].y) * rateToNext;
	scale.z = keyScale[keyPrevious].z + (keyScale[keyNext].z-keyScale[keyPrevious].z) * rateToNext;
	D3DXMatrixScaling(&matScale, scale.x, scale.y, scale.z);

	std::vector<D3DXQUATERNION>& keyRotate = part->GetRotateKey();
	D3DXMATRIX matRotate;
	D3DXMatrixIdentity(&matRotate);
	D3DXQUATERNION quat;
	D3DXQuaternionSlerp(&quat, &keyRotate[keyPrevious], &keyRotate[keyNext], rateToNext);
	D3DXMatrixRotationQuaternion(&matRotate, &quat);

	res = matScale * matRotate * matTrans;
	return res;
}

//RenderObjectElfreinaBlock
RenderObjectElfreinaBlock::~RenderObjectElfreinaBlock()
{

}
bool RenderObjectElfreinaBlock::IsTranslucent()
{
	ElfreinaMeshData::Mesh* obj = (ElfreinaMeshData::Mesh*)obj_.GetPointer();
	D3DMATERIAL9& mat = obj->material_->mat_;
	bool bTrans = true;
	bTrans &= (mat.Diffuse.a != 1.0f);
	return RenderObjectB2NXBlock::IsTranslucent() || bTrans;
}
void RenderObjectElfreinaBlock::CalculateZValue()
{
	DirectGraphics* graph = DirectGraphics::GetBase();
	IDirect3DDevice9* pDevice = graph->GetDevice();
	ElfreinaMeshData::Mesh* obj = (ElfreinaMeshData::Mesh*)obj_.GetPointer();

	D3DXMATRIX matTrans = obj->_CreateWorldTransformMaxtrix();
	D3DXMATRIX matView;
	D3DXMATRIX matPers;
	pDevice->GetTransform(D3DTS_VIEW, &matView);
	pDevice->GetTransform(D3DTS_PROJECTION, &matPers);
	D3DXMATRIX matAnime = matrix_->GetMatrix(obj->indexWeightForCalucZValue_);

	D3DXMATRIX mat = matAnime * matTrans * matView * matPers;
	D3DXVECTOR3 posCenter = obj->GetWeightCenter();
	D3DXVec3TransformCoord(&posCenter, &posCenter, &mat);
	posSortKey_ = posCenter.z;
}

//ElfreinaMesh
bool ElfreinaMesh::CreateFromFileReader(gstd::ref_count_ptr<gstd::FileReader> reader)
{
	bool res = false;
	{
		Lock lock(DxMeshManager::GetBase()->GetLock());
		if(data_ != NULL)Release();

		std::wstring name = reader->GetOriginalPath();

		data_ = _GetFromManager(name);
		if(data_ == NULL)
		{
			if(!reader->Open())throw gstd::wexception(L"�t�@�C�����J���܂���");
			data_ = new ElfreinaMeshData();
			data_->SetName(name);
			ElfreinaMeshData* data = (ElfreinaMeshData*)data_.GetPointer();
			res = data->CreateFromFileReader(reader);
			if(res)
			{
				Logger::WriteTop(StringUtility::Format(L"���b�V����ǂݍ��݂܂���[%s]", name.c_str()));
				_AddManager(name, data_);
			
			}
			else
			{
				data_ = NULL;
			}
		}
		else
		{
			res = true;
		}
	}
	return res;
}
bool ElfreinaMesh::CreateFromFileInLoadThread(std::wstring path)
{
	return DxMesh::CreateFromFileInLoadThread(path, MESH_ELFREINA);
}
std::wstring ElfreinaMesh::GetPath()
{
	if(data_ == NULL)return L"";
	return ((ElfreinaMeshData*)data_.GetPointer())->path_;
}
void ElfreinaMesh::Render()
{
	if(data_ == NULL)return;

	ElfreinaMeshData* data = (ElfreinaMeshData*)data_.GetPointer();
	for(int iMesh = 0 ; iMesh<data->mesh_.size(); iMesh++)
	{
		ElfreinaMeshData::Mesh* mesh = data->mesh_[iMesh].GetPointer();
		mesh->SetPosition(position_);
		mesh->SetAngle(angle_);
		mesh->SetScale(scale_);
		mesh->SetColor(color_);
		mesh->SetCoordinate2D(bCoordinate2D_);
		mesh->Render();
	}
	//mesh_[6]->Render();
}
void ElfreinaMesh::Render(std::wstring nameAnime, int time)
{
	if(data_ == NULL)return;

	gstd::ref_count_ptr<Matrices> matrix = CreateAnimationMatrix(nameAnime, time);
	if(matrix == NULL)return;

	ElfreinaMeshData* data = (ElfreinaMeshData*)data_.GetPointer();
	while(!data->bLoad_)
	{
		::Sleep(1);
	}

	for(int iMesh = 0 ; iMesh<data->mesh_.size(); iMesh++)
	{
		ElfreinaMeshData::Mesh* mesh = data->mesh_[iMesh].GetPointer();
		mesh->SetMatrix(matrix);
	}
	Render();
}
gstd::ref_count_ptr<RenderBlocks> ElfreinaMesh::CreateRenderBlocks()
{
	if(data_ == NULL)return NULL;
	gstd::ref_count_ptr<RenderBlocks> res = new RenderBlocks();
	ElfreinaMeshData* data = (ElfreinaMeshData*)data_.GetPointer();
	for(int iMesh = 0 ; iMesh<data->mesh_.size(); iMesh++)
	{
		gstd::ref_count_ptr<ElfreinaMeshData::Mesh> mesh = data->mesh_[iMesh];
		mesh->SetPosition(position_);
		mesh->SetAngle(angle_);
		mesh->SetScale(scale_);
		mesh->SetColor(color_);

		gstd::ref_count_ptr<RenderBlock> block = mesh->CreateRenderBlock();
		block->SetRenderObject(mesh);
		res->Add(block);
	}
	return res;
}
gstd::ref_count_ptr<RenderBlocks> ElfreinaMesh::CreateRenderBlocks(std::wstring nameAnime, double time)
{
	if(data_ == NULL)return NULL;

	gstd::ref_count_ptr<Matrices> matrix = CreateAnimationMatrix(nameAnime, time);
	if(matrix == NULL)return NULL;
	ElfreinaMeshData* data = (ElfreinaMeshData*)data_.GetPointer();
	for(int iMesh = 0 ; iMesh<data->mesh_.size(); iMesh++)
	{
		ElfreinaMeshData::Mesh* mesh = data->mesh_[iMesh].GetPointer();
		mesh->SetMatrix(matrix);
	}

	return CreateRenderBlocks();
}
double ElfreinaMesh::_CalcFrameToTime(double time, gstd::ref_count_ptr<ElfreinaMeshData::AnimationData> anime)
{
	bool bLoop = anime->bLoop_;
	int framePerSec = anime->framePerSecond_;
	time = (int)((double)1000 / (double)framePerSec * (double)time);

	int timeTotal = anime->timeTotal_;
	if(bLoop)
	{
		int tTime = (int)time / timeTotal;
		time -= (int)timeTotal * tTime;
	}
	else
	{
		if(time > timeTotal)time = timeTotal;
	}
	return time;
}
gstd::ref_count_ptr<Matrices> ElfreinaMesh::CreateAnimationMatrix(std::wstring nameAnime, double time)
{
	if(data_ == NULL)return NULL;

	ElfreinaMeshData* data = (ElfreinaMeshData*)data_.GetPointer();
	bool bExist = data->anime_.find(nameAnime) != data->anime_.end();
	if(!bExist)return NULL;
	gstd::ref_count_ptr<ElfreinaMeshData::AnimationData> anime = data->anime_[nameAnime];

	//���[�v�L���Ŏ��Ԃ��v�Z����
	time = _CalcFrameToTime(time, anime);

	gstd::ref_count_ptr<Matrices> matrix = anime->CreateBoneAnimationMatrix(time, data);
	return matrix;
}

D3DXMATRIX ElfreinaMesh::GetAnimationMatrix(std::wstring nameAnime, double time, std::wstring nameBone)
{
	D3DXMATRIX res;
	ElfreinaMeshData* data = (ElfreinaMeshData*)data_.GetPointer();
	if(data->mapBoneNameIndex_.find(nameBone) != data->mapBoneNameIndex_.end())
	{
		int indexBone = data->mapBoneNameIndex_[nameBone];
		bool bExist = data->anime_.find(nameAnime) != data->anime_.end();
		if(bExist)
		{
			gstd::ref_count_ptr<ElfreinaMeshData::AnimationData> anime = data->anime_[nameAnime];

			//���[�v�L���Ŏ��Ԃ��v�Z����
			time = _CalcFrameToTime(time, anime);
			gstd::ref_count_ptr<Matrices> matrix = anime->CreateBoneAnimationMatrix(time, data);
			D3DXMATRIX matBone = matrix->GetMatrix(indexBone);

			D3DXMATRIX matInv = data->bone_[indexBone]->matOffset_;
			D3DXMatrixInverse(&matInv, NULL, &matInv);
//			D3DXVECTOR3 pos;
//			D3DXVec3TransformCoord(&pos, &D3DXVECTOR3(0,0,0), &matInv);
//			D3DXMatrixTranslation(&matInv,pos.x,pos.y,pos.z);
			res = matInv * matBone;

			D3DXMATRIX matScale;
			D3DXMatrixScaling(&matScale, scale_.x, scale_.y, scale_.z);
			res = res * matScale;

			D3DXMATRIX matRot;
			D3DXMatrixRotationYawPitchRoll(&matRot, D3DXToRadian(angle_.y),D3DXToRadian(angle_.x),D3DXToRadian(angle_.z));
			res = res * matRot;

			D3DXMATRIX matTrans;
			D3DXMatrixTranslation(&matTrans, position_.x, position_.y, position_.z);
			res = res * matTrans;
		}
	}
	return res;
}
