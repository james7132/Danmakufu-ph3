#include"DnhReplay.hpp"
#include"DnhGcLibImpl.hpp"

/**********************************************************
//ReplayInformation
**********************************************************/
ReplayInformation::ReplayInformation()
{
	userData_ = new ScriptCommonData();
}
ReplayInformation::~ReplayInformation()
{
}
std::vector<int> ReplayInformation::GetStageIndexList()
{
	std::vector<int> res;

	std::map<int, ref_count_ptr<StageData> >::iterator itr = mapStageData_.begin();
	for(; itr!=mapStageData_.end() ; itr++)
	{
		int stage = itr->first;
		res.push_back(stage);
	}

	std::sort(res.begin(), res.end());
	return res;
}
std::wstring ReplayInformation::GetDateAsString()
{
	std::wstring res = StringUtility::Format(
		L"%04d/%02d/%02d %02d:%02d",
		date_.wYear, date_.wMonth, date_.wDay,
		date_.wHour, date_.wMinute
	);

	return res;
}
void ReplayInformation::SetUserData(std::string key, gstd::value val)
{
	userData_->SetValue(key, val);
}
gstd::value ReplayInformation::GetUserData(std::string key)
{
	gstd::value res = userData_->GetValue(key);
	return res;
}
bool ReplayInformation::IsUserDataExists(std::string key)
{
	bool res = userData_->IsExists(key);
	return res;
}

bool ReplayInformation::SaveToFile(std::wstring scriptPath, int index)
{
	std::wstring dir = EPathProperty::GetReplaySaveDirectory(scriptPath);
	std::wstring scriptName = PathProperty::GetFileNameWithoutExtension(scriptPath);
	std::wstring path = dir + scriptName + StringUtility::Format(L"_replay%02d.dat", index);

	//フォルダ作成
	File fDir(dir);
	fDir.CreateDirectory();

	RecordBuffer rec;
	rec.SetRecordAsInteger("version", 1);
	rec.SetRecordAsStringW("playerScriptID", playerScriptID_);
	rec.SetRecordAsStringW("playerScriptFileName", playerScriptFileName_);
	rec.SetRecordAsStringW("playerScriptReplayName", playerScriptReplayName_);

	rec.SetRecordAsStringW("comment", comment_);
	rec.SetRecordAsStringW("userName", userName_);
	rec.SetRecord("totalScore", totalScore_);
	rec.SetRecordAsDouble("fpsAvarage", fpsAvarage_);
	rec.SetRecord("date", date_);
	
	RecordBuffer recUserData;
	userData_->WriteRecord(recUserData);
	rec.SetRecordAsRecordBuffer("userData", recUserData);

	std::vector<int> listStage = GetStageIndexList();
	rec.SetRecordAsInteger("stageCount", listStage.size());
	rec.SetRecord("stageIndexList", &listStage[0], sizeof(int) * listStage.size());
	for(int iStage = 0 ; iStage < listStage.size(); iStage++)
	{
		int stage = listStage[iStage];
		std::string key = StringUtility::Format("stage%d", stage);

		ref_count_ptr<StageData> data = mapStageData_[stage];
		gstd::RecordBuffer recStage;
		data->WriteRecord(recStage);
		rec.SetRecordAsRecordBuffer(key, recStage);
	}

	rec.WriteToFile(path, "REPLAY");
	return true;
}
ref_count_ptr<ReplayInformation> ReplayInformation::CreateFromFile(std::wstring scriptPath, std::wstring fileName)
{
	std::wstring dir = EPathProperty::GetReplaySaveDirectory(scriptPath);
//	std::string scriptName = PathProperty::GetFileNameWithoutExtension(scriptPath);
//	std::string path = dir + scriptName + StringUtility::Format("_replay%02d.dat", index);
	std::wstring path = dir + fileName;
	
	ref_count_ptr<ReplayInformation> res = CreateFromFile(path);
	return res; 
}
ref_count_ptr<ReplayInformation> ReplayInformation::CreateFromFile(std::wstring path)
{
	RecordBuffer rec;
	if(!rec.ReadFromFile(path, "REPLAY"))return NULL;

	int version = rec.GetRecordAsInteger("version");
	if(version != 1)return NULL;

	ref_count_ptr<ReplayInformation> res = new ReplayInformation();
	res->path_ = path;
	res->playerScriptID_ = rec.GetRecordAsStringW("playerScriptID");
	res->playerScriptFileName_ = rec.GetRecordAsStringW("playerScriptFileName");
	res->playerScriptReplayName_ = rec.GetRecordAsStringW("playerScriptReplayName");

	res->comment_ = rec.GetRecordAsStringW("comment");
	res->userName_ = rec.GetRecordAsStringW("userName");
	rec.GetRecord("totalScore", res->totalScore_);
	res->fpsAvarage_ = rec.GetRecordAsDouble("fpsAvarage");
	rec.GetRecord("date", res->date_);
	
	res->userData_->Clear();
	if(rec.IsExists("userData"))
	{
		RecordBuffer recUserData;
		rec.GetRecordAsRecordBuffer("userData", recUserData);
		res->userData_->ReadRecord(recUserData);
	}

	int stageCount = rec.GetRecordAsInteger("stageCount");
	std::vector<int> listStage;
	listStage.resize(stageCount);
	rec.GetRecord("stageIndexList",&listStage[0], sizeof(int) * stageCount);
	for(int iStage = 0 ; iStage < listStage.size(); iStage++)
	{
		int stage = listStage[iStage];
		std::string key = StringUtility::Format("stage%d", stage);
		ref_count_ptr<StageData> data = new StageData();
		gstd::RecordBuffer recStage;
		rec.GetRecordAsRecordBuffer(key, recStage);
		data->ReadRecord(recStage);
		res->mapStageData_[stage] = data;
	}

	return res;
}


//ReplayInformation::StageData
double ReplayInformation::StageData::GetFramePerSecondAvarage()
{
	double res = 0;
	for(int iFrame = 0 ; iFrame < listFramePerSecond_.size(); iFrame++)
	{
		res += listFramePerSecond_[iFrame];
	}

	if(listFramePerSecond_.size() > 0)
		res /= listFramePerSecond_.size();
	return res;
}
std::set<std::string> ReplayInformation::StageData::GetCommonDataAreaList()
{
	std::set<std::string> res;
	std::map<std::string, ref_count_ptr<RecordBuffer> >::iterator itrCommonData;
	for(itrCommonData = mapCommonData_.begin() ; itrCommonData != mapCommonData_.end() ; itrCommonData++)
	{
		std::string key = itrCommonData->first;
		res.insert(key);
	}
	return res;
}
ref_count_ptr<ScriptCommonData> ReplayInformation::StageData::GetCommonData(std::string area)
{
	ref_count_ptr<ScriptCommonData> res = new ScriptCommonData();
	if(mapCommonData_.find(area) != mapCommonData_.end())
	{
		ref_count_ptr<RecordBuffer> record = mapCommonData_[area];
		res->ReadRecord(*record);
	}
	return res;
}
void ReplayInformation::StageData::SetCommonData(std::string area, ref_count_ptr<ScriptCommonData> commonData)
{
	ref_count_ptr<RecordBuffer> record = new RecordBuffer();
	if(commonData != NULL)
		commonData->WriteRecord(*record);
	mapCommonData_[area] = record;
}

void ReplayInformation::StageData::ReadRecord(gstd::RecordBuffer& record)
{
	mainScriptID_ = record.GetRecordAsStringW("mainScriptID");
	mainScriptName_ = record.GetRecordAsStringW("mainScriptName");
	mainScriptRelativePath_ = record.GetRecordAsStringW("mainScriptRelativePath");
	if(record.IsExists("scoreStart"))
		record.GetRecord("scoreStart", &scoreStart_, sizeof(_int64));
	if(record.IsExists("scoreLast"))
		record.GetRecord("scoreLast", &scoreLast_, sizeof(_int64));
	record.GetRecord("graze", &graze_, sizeof(_int64));
	record.GetRecord("point", &point_, sizeof(_int64));
	frameEnd_ = record.GetRecordAsInteger("frameEnd");
	randSeed_ = record.GetRecordAsInteger("randSeed");
	record.GetRecordAsRecordBuffer("recordKey", *recordKey_);

	int countFramePerSecond = record.GetRecordAsInteger("countFramePerSecond");
	listFramePerSecond_.resize(countFramePerSecond);
	record.GetRecord("listFramePerSecond",&listFramePerSecond_[0], sizeof(float) * listFramePerSecond_.size());

	//共通データ
	gstd::RecordBuffer recComMap;
	record.GetRecordAsRecordBuffer("mapCommonData", recComMap);
	std::vector<std::string> listKeyCommonData = recComMap.GetKeyList();
	for(int iCommonData = 0 ; iCommonData < listKeyCommonData.size() ; iCommonData++)
	{
		std::string key = listKeyCommonData[iCommonData];
		ref_count_ptr<RecordBuffer> recComData = new RecordBuffer();
		recComMap.GetRecordAsRecordBuffer(key, *recComData);
		mapCommonData_[key] = recComData;
	}

	//自機情報
	playerScriptID_ = record.GetRecordAsStringW("playerScriptID");
	playerScriptFileName_ = record.GetRecordAsStringW("playerScriptFileName");
	playerScriptReplayName_ = record.GetRecordAsStringW("playerScriptReplayName");
	playerLife_ = record.GetRecordAsDouble("playerLife");
	playerBombCount_ = record.GetRecordAsDouble("playerBombCount");
	playerPower_ = record.GetRecordAsDouble("playerPower");
	playerRebirthFrame_ = record.GetRecordAsInteger("playerRebirthFrame");
}
void ReplayInformation::StageData::WriteRecord(gstd::RecordBuffer& record)
{
	record.SetRecordAsStringW("mainScriptID", mainScriptID_);
	record.SetRecordAsStringW("mainScriptName", mainScriptName_);
	record.SetRecordAsStringW("mainScriptRelativePath", mainScriptRelativePath_);
	record.SetRecord("scoreStart", &scoreStart_, sizeof(_int64));
	record.SetRecord("scoreLast", &scoreLast_, sizeof(_int64));
	record.SetRecord("graze", &graze_, sizeof(_int64));
	record.SetRecord("point", &point_, sizeof(_int64));
	record.SetRecordAsInteger("frameEnd", frameEnd_);
	record.SetRecordAsInteger("randSeed", randSeed_);
	record.SetRecordAsRecordBuffer("recordKey", *recordKey_);

	int countFramePerSecond = listFramePerSecond_.size();
	record.SetRecordAsInteger("countFramePerSecond", countFramePerSecond);
	record.SetRecord("listFramePerSecond",&listFramePerSecond_[0], sizeof(float) * listFramePerSecond_.size());

	//共通データ
	gstd::RecordBuffer recComMap;
	std::map<std::string, ref_count_ptr<RecordBuffer> >::iterator itrCommonData;
	for(itrCommonData = mapCommonData_.begin() ; itrCommonData != mapCommonData_.end() ; itrCommonData++)
	{
		std::string key = itrCommonData->first;
		ref_count_ptr<RecordBuffer> recComData = itrCommonData->second;
		recComMap.SetRecordAsRecordBuffer(key, *recComData);
	}
	record.SetRecordAsRecordBuffer("mapCommonData", recComMap);

	//自機情報
	record.SetRecordAsStringW("playerScriptID", playerScriptID_);
	record.SetRecordAsStringW("playerScriptFileName", playerScriptFileName_);
	record.SetRecordAsStringW("playerScriptReplayName", playerScriptReplayName_);
	record.SetRecordAsDouble("playerLife", playerLife_);
	record.SetRecordAsDouble("playerBombCount", playerBombCount_);
	record.SetRecordAsDouble("playerPower", playerPower_);
	record.SetRecordAsInteger("playerRebirthFrame", playerRebirthFrame_);
}

/**********************************************************
//ReplayInformationManager
**********************************************************/
ReplayInformationManager::ReplayInformationManager()
{

}
ReplayInformationManager::~ReplayInformationManager()
{
}
void ReplayInformationManager::UpdateInformationList(std::wstring pathScript)
{
	mapInfo_.clear();

	std::wstring scriptName = PathProperty::GetFileNameWithoutExtension(pathScript);
	std::wstring fileNameHead = scriptName + L"_replay";
	std::wstring dir = EPathProperty::GetReplaySaveDirectory(pathScript);
	std::vector<std::wstring> listPath = File::GetFilePathList(dir);

	int indexFree = ReplayInformation::INDEX_USER;
	std::vector<std::wstring>::iterator itr;
	for(itr = listPath.begin() ; itr != listPath.end() ; itr++)
	{
		std::wstring path = *itr;
		std::wstring fileName = PathProperty::GetFileName(path);

		if(fileName.find(fileNameHead) == std::wstring::npos)continue;

		ref_count_ptr<ReplayInformation> info = ReplayInformation::CreateFromFile(pathScript, fileName);
		if(info == NULL)continue;

		std::wstring strKey = fileName.substr(fileNameHead.size());
		strKey = PathProperty::GetFileNameWithoutExtension(strKey);
		int index = StringUtility::ToInteger(strKey);
		if(index > 0)
		{
			strKey = StringUtility::Format(L"%d", index);
		}
		else
		{
			index = indexFree;
			indexFree++;
			strKey = StringUtility::Format(L"%d", index);
		}

		int key = StringUtility::ToInteger(strKey);
		mapInfo_[key] = info;
	}

}
std::vector<int> ReplayInformationManager::GetIndexList()
{
	std::vector<int> res;
	std::map<int, ref_count_ptr<ReplayInformation> >::iterator itr;
	for(itr = mapInfo_.begin() ; itr != mapInfo_.end() ; itr++)
	{
		int key = itr->first;
		res.push_back(key);
	}
	return res;
}
ref_count_ptr<ReplayInformation> ReplayInformationManager::GetInformation(int index)
{
	if(mapInfo_.find(index) == mapInfo_.end())return NULL;
	return mapInfo_[index];
}
