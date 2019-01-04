#include"Task.hpp"

using namespace gstd;

/**********************************************************
//TaskFunction
**********************************************************/
std::wstring TaskFunction::GetInfoAsString()
{
	return task_->GetInfoAsString();
}

/**********************************************************
//TaskBase
**********************************************************/
TaskBase::TaskBase()
{
	indexTask_ = -1;
	idTask_ = TASK_FREE_ID;
	idTaskGroup_ = TASK_GROUP_FREE_ID;
}
TaskBase::~TaskBase()
{

}

/**********************************************************
//TaskManager
**********************************************************/
gstd::CriticalSection TaskManager::lockStatic_;
TaskManager::TaskManager()
{
	indexTaskManager_ = 0;
}
TaskManager::~TaskManager()
{
	this->Clear();
	panelInfo_ = NULL;
}
void TaskManager::_ArrangeTask()
{
	//�^�X�N�폜�̈搮��
	std::list<ref_count_ptr<TaskBase> >::iterator itrTask;
	for(itrTask=listTask_.begin();itrTask!=listTask_.end();)
	{
		if(*itrTask==NULL)itrTask=listTask_.erase(itrTask);
		else itrTask++;
	}

	//�֐��폜�̈搮��
	std::map<int, std::vector<std::list<ref_count_ptr<TaskFunction> > > >::iterator itrType;
	for(itrType=mapFunc_.begin();itrType!=mapFunc_.end();itrType++)
	{
		std::vector<std::list<ref_count_ptr<TaskFunction> > > *vectPri = &itrType->second;
		std::vector<std::list<ref_count_ptr<TaskFunction> > >::iterator itrPri;
		for(itrPri=vectPri->begin(); itrPri!=vectPri->end();itrPri++)
		{
			std::list<ref_count_ptr<TaskFunction> > &listFunc = *itrPri;
			std::list<ref_count_ptr<TaskFunction> >::iterator itrFunc;
			for(itrFunc=listFunc.begin();itrFunc!=listFunc.end();)
			{
				if(*itrFunc==NULL)itrFunc=listFunc.erase(itrFunc);
				else 
				{
					int delay = (*itrFunc)->GetDelay();
					delay = max(0, delay -1);
					(*itrFunc)->SetDelay(delay);
					itrFunc++;
				}
			}
		}
	}

	//�^�X�N���p�l���X�V
	if(panelInfo_ != NULL)panelInfo_->Update(this);
}
void TaskManager::_CheckInvalidFunctionDivision(int divFunc)
{
	if(mapFunc_.find(divFunc) == mapFunc_.end())
		throw gstd::wexception(L"���݂��Ȃ��@�\�敪");
}
void TaskManager::Clear()
{
	listTask_.clear();
	mapFunc_.clear();
}
void TaskManager::ClearTask()
{
	listTask_.clear();
	std::map<int, std::vector<std::list<ref_count_ptr<TaskFunction> > > >::iterator itrDiv;
	for(itrDiv=mapFunc_.begin();itrDiv!=mapFunc_.end();itrDiv++)
	{
		std::vector<std::list<ref_count_ptr<TaskFunction> > > &vectPri = (*itrDiv).second;
		vectPri.clear();
	}
}
void TaskManager::AddTask(ref_count_ptr<TaskBase> task)
{
	std::list<ref_count_ptr<TaskBase> >::iterator itr;
	for(itr=listTask_.begin();itr!=listTask_.end();itr++)
	{
		ref_count_ptr<TaskBase> &tTask = (*itr);
		if( (*itr) == NULL)continue;
		if( (*itr) == task)return;
	}
//	task->mTask_ = this;

	//TODO ID�̊���U��
	task->indexTask_ = indexTaskManager_;
	indexTaskManager_++;
	listTask_.push_back(task);
}
ref_count_ptr<TaskBase> TaskManager::GetTask(int idTask)
{
	std::list<ref_count_ptr<TaskBase> >::iterator itr;
	for(itr=listTask_.begin();itr!=listTask_.end();itr++)
	{
		if((*itr)==NULL)continue;
		if((*itr)->idTask_!=idTask)continue;
		return (*itr);
	}
	return NULL;
}
ref_count_ptr<TaskBase> TaskManager::GetTask(const std::type_info& info)
{
	std::list<ref_count_ptr<TaskBase> >::iterator itr;
	for(itr=listTask_.begin();itr!=listTask_.end();itr++)
	{
		if((*itr)==NULL)continue;
		const std::type_info& tInfo = typeid(*(*itr).GetPointer());
		if(info != tInfo)continue;
		return (*itr);
	}
	return NULL;
}
void TaskManager::RemoveTask(TaskBase* task)
{
	std::list<ref_count_ptr<TaskBase> >::iterator itr;
	for(itr=listTask_.begin();itr!=listTask_.end();itr++)
	{
		if((*itr)==NULL)continue;
		if((*itr).GetPointer() != task)continue;
		if((*itr)->idTask_!=task->idTask_)continue;
		this->RemoveFunction(task);
		(*itr)=NULL;
		break;
	}
}
void TaskManager::RemoveTask(int idTask)
{
	std::list<ref_count_ptr<TaskBase> >::iterator itr;
	for(itr=listTask_.begin();itr!=listTask_.end();itr++)
	{
		if((*itr)==NULL)continue;
		if((*itr)->idTask_!=idTask)continue;
		this->RemoveFunction((*itr).GetPointer());
		(*itr) = NULL;
		break;
	}
}
void TaskManager::RemoveTaskGroup(int idGroup)
{
	std::list<ref_count_ptr<TaskBase> >::iterator itr;
	for(itr=listTask_.begin();itr!=listTask_.end();itr++)
	{
		if((*itr)==NULL)continue;
		if((*itr)->idTaskGroup_!=idGroup)continue;
		this->RemoveFunction((*itr).GetPointer());
		(*itr) = NULL;
	}
}
void TaskManager::RemoveTask(const std::type_info& info)
{
	std::list<ref_count_ptr<TaskBase> >::iterator itr;
	for(itr=listTask_.begin();itr!=listTask_.end();itr++)
	{
		if((*itr)==NULL)continue;
		const std::type_info& tInfo = typeid(*(*itr).GetPointer());
		if(info != tInfo)continue;
		this->RemoveFunction((*itr).GetPointer());
		(*itr)=NULL;
	}
}
void TaskManager::RemoveTaskWithoutTypeInfo(std::set<const std::type_info*> listInfo)
{
	std::list<ref_count_ptr<TaskBase> >::iterator itr;
	for(itr=listTask_.begin();itr!=listTask_.end();itr++)
	{
		if((*itr)==NULL)continue;
		const std::type_info& tInfo = typeid(*(*itr).GetPointer());
		if(listInfo.find(&tInfo) != listInfo.end())continue;
		this->RemoveFunction((*itr).GetPointer());
		(*itr)=NULL;
	}
}
void TaskManager::InitializeFunctionDivision(int divFunc, int maxPri)
{
	if(mapFunc_.find(divFunc) != mapFunc_.end())
		throw gstd::wexception(L"���łɑ��݂��Ă���@�\�敪�����������悤�Ƃ��܂����B");
	std::vector<std::list<ref_count_ptr<TaskFunction> > > vectPri;
	vectPri.resize(maxPri);
	mapFunc_[divFunc] = vectPri;
}
void TaskManager::CallFunction(int divFunc)
{
	_CheckInvalidFunctionDivision(divFunc);

	std::vector<std::list<ref_count_ptr<TaskFunction> > > &vectPri = mapFunc_[divFunc];
	std::vector<std::list<ref_count_ptr<TaskFunction> > >::iterator itrPri;
	for(itrPri=vectPri.begin();itrPri!=vectPri.end();itrPri++)
	{
		std::list<ref_count_ptr<TaskFunction> > &listFunc = *itrPri;
		std::list<ref_count_ptr<TaskFunction> >::iterator itrFunc;
		for(itrFunc=listFunc.begin();itrFunc!=listFunc.end();itrFunc++)
		{
			if(*itrFunc==NULL)continue;
			if((*itrFunc)->bEnable_ == false)continue;
			if((*itrFunc)->IsDelay())continue;
			(*itrFunc)->Call();
		}
	}
	_ArrangeTask();
}
void TaskManager::AddFunction(int divFunc, ref_count_ptr<TaskFunction> func,int pri,int idFunc)
{
	if(mapFunc_.find(divFunc) == mapFunc_.end())
		throw gstd::wexception(L"���݂��Ȃ��@�\�敪");
	std::vector<std::list<ref_count_ptr<TaskFunction> > > &vectPri = mapFunc_[divFunc];
	func->id_ = idFunc;
	vectPri[pri].push_back(func);
}
void TaskManager::RemoveFunction(TaskBase* task)
{
	std::map<int, std::vector<std::list<ref_count_ptr<TaskFunction> > > >::iterator itrDiv;
	for(itrDiv=mapFunc_.begin();itrDiv!=mapFunc_.end();itrDiv++)
	{
		std::vector<std::list<ref_count_ptr<TaskFunction> > > &vectPri = (*itrDiv).second;
		std::vector<std::list<ref_count_ptr<TaskFunction> > >::iterator itrPri;
		for(itrPri=vectPri.begin(); itrPri!=vectPri.end();itrPri++)
		{
			std::list<ref_count_ptr<TaskFunction> > &listFunc = *itrPri;
			std::list<ref_count_ptr<TaskFunction> >::iterator itrFunc;
			for(itrFunc=listFunc.begin();itrFunc!=listFunc.end();itrFunc++)
			{
				if(*itrFunc == NULL)continue;
				if((*itrFunc)->task_ != task)continue;
				if((*itrFunc)->task_->idTask_ != task->idTask_)continue;
				(*itrFunc)=NULL;
			}
		}
	}
}
void TaskManager::RemoveFunction(TaskBase* task,int divFunc, int idFunc)
{
	_CheckInvalidFunctionDivision(divFunc);
	std::vector<std::list<ref_count_ptr<TaskFunction> > > &vectPri = mapFunc_[divFunc];
	std::vector<std::list<ref_count_ptr<TaskFunction> > >::iterator itrPri;
	for(itrPri=vectPri.begin(); itrPri!=vectPri.end();itrPri++)
	{
		std::list<ref_count_ptr<TaskFunction> > &listFunc = *itrPri;
		std::list<ref_count_ptr<TaskFunction> >::iterator itrFunc;
		for(itrFunc=listFunc.begin();itrFunc!=listFunc.end();itrFunc++)
		{
			if(*itrFunc == NULL)continue;
			if((*itrFunc)->id_ != idFunc)continue;
			if((*itrFunc)->task_->idTask_ != task->idTask_)continue;
			(*itrFunc)=NULL;
		}
	}
}
void TaskManager::RemoveFunction(const std::type_info& info)
{
	std::map<int, std::vector<std::list<ref_count_ptr<TaskFunction> > > >::iterator itrDiv;
	for(itrDiv=mapFunc_.begin();itrDiv!=mapFunc_.end();itrDiv++)
	{
		std::vector<std::list<ref_count_ptr<TaskFunction> > > &vectPri = (*itrDiv).second;
		std::vector<std::list<ref_count_ptr<TaskFunction> > >::iterator itrPri;
		for(itrPri=vectPri.begin(); itrPri!=vectPri.end();itrPri++)
		{
			std::list<ref_count_ptr<TaskFunction> > &listFunc = *itrPri;
			std::list<ref_count_ptr<TaskFunction> >::iterator itrFunc;
			for(itrFunc=listFunc.begin();itrFunc!=listFunc.end();itrFunc++)
			{
				if(*itrFunc == NULL)continue;
				const std::type_info& tInfo = typeid(*(*itrFunc)->task_);
				if(info != tInfo)continue;
				(*itrFunc)=NULL;
			}
		}
	}
}
void TaskManager::SetFunctionEnable(bool bEnable)
{
	std::map<int, std::vector<std::list<ref_count_ptr<TaskFunction> > > >::iterator itrDiv;
	for(itrDiv=mapFunc_.begin();itrDiv!=mapFunc_.end();itrDiv++)
	{
		std::vector<std::list<ref_count_ptr<TaskFunction> > > &vectPri = (*itrDiv).second;
		std::vector<std::list<ref_count_ptr<TaskFunction> > >::iterator itrPri;
		for(itrPri=vectPri.begin(); itrPri!=vectPri.end();itrPri++)
		{
			std::list<ref_count_ptr<TaskFunction> > &listFunc = *itrPri;
			std::list<ref_count_ptr<TaskFunction> >::iterator itrFunc;
			for(itrFunc=listFunc.begin();itrFunc!=listFunc.end();itrFunc++)
			{
				if(*itrFunc == NULL)continue;
				(*itrFunc)->bEnable_ = bEnable;
			}
		}
	}
}
void TaskManager::SetFunctionEnable(bool bEnable, int divFunc)
{
	_CheckInvalidFunctionDivision(divFunc);
	std::vector<std::list<ref_count_ptr<TaskFunction> > > &vectPri = mapFunc_[divFunc];
	std::vector<std::list<ref_count_ptr<TaskFunction> > >::iterator itrPri;
	for(itrPri=vectPri.begin(); itrPri!=vectPri.end();itrPri++)
	{
		std::list<ref_count_ptr<TaskFunction> > &listFunc = *itrPri;
		std::list<ref_count_ptr<TaskFunction> >::iterator itrFunc;
		for(itrFunc=listFunc.begin();itrFunc!=listFunc.end();itrFunc++)
		{
			if(*itrFunc == NULL)continue;
			(*itrFunc)->bEnable_ = bEnable;
		}
	}
}
void TaskManager::SetFunctionEnable(bool bEnable, int idTask, int divFunc)
{
	ref_count_ptr<TaskBase> task = this->GetTask(idTask);
	if(task == NULL)return;
	this->SetFunctionEnable(bEnable, task.GetPointer(), divFunc);
}
void TaskManager::SetFunctionEnable(bool bEnable, int idTask, int divFunc, int idFunc)
{
	ref_count_ptr<TaskBase> task = this->GetTask(idTask);
	if(task == NULL)return;
	this->SetFunctionEnable(bEnable, task.GetPointer(), divFunc, idFunc);
}
void TaskManager::SetFunctionEnable(bool bEnable, TaskBase* task, int divFunc)
{
	_CheckInvalidFunctionDivision(divFunc);
	std::vector<std::list<ref_count_ptr<TaskFunction> > > &vectPri = mapFunc_[divFunc];
	std::vector<std::list<ref_count_ptr<TaskFunction> > >::iterator itrPri;
	for(itrPri=vectPri.begin(); itrPri!=vectPri.end();itrPri++)
	{
		std::list<ref_count_ptr<TaskFunction> > &listFunc = *itrPri;
		std::list<ref_count_ptr<TaskFunction> >::iterator itrFunc;
		for(itrFunc=listFunc.begin();itrFunc!=listFunc.end();itrFunc++)
		{
			if(*itrFunc == NULL)continue;
			if((*itrFunc)->task_ != task)continue;
			if((*itrFunc)->task_->idTask_ != task->idTask_)continue;
			(*itrFunc)->bEnable_ = bEnable;
		}
	}
}
void TaskManager::SetFunctionEnable(bool bEnable, TaskBase* task, int divFunc, int idFunc)
{
	_CheckInvalidFunctionDivision(divFunc);
	std::vector<std::list<ref_count_ptr<TaskFunction> > > &vectPri = mapFunc_[divFunc];
	std::vector<std::list<ref_count_ptr<TaskFunction> > >::iterator itrPri;
	for(itrPri=vectPri.begin(); itrPri!=vectPri.end();itrPri++)
	{
		std::list<ref_count_ptr<TaskFunction> > &listFunc = *itrPri;
		std::list<ref_count_ptr<TaskFunction> >::iterator itrFunc;
		for(itrFunc=listFunc.begin();itrFunc!=listFunc.end();itrFunc++)
		{
			if(*itrFunc == NULL)continue;
			if((*itrFunc)->task_ != task)continue;
			if((*itrFunc)->task_->idTask_ != task->idTask_)continue;
			if((*itrFunc)->id_ != idFunc)continue;
			(*itrFunc)->bEnable_ = bEnable;
		}
	}
}
void TaskManager::SetFunctionEnable(bool bEnable, const std::type_info& info, int divFunc)
{
	_CheckInvalidFunctionDivision(divFunc);

	std::vector<std::list<ref_count_ptr<TaskFunction> > > &vectPri = mapFunc_[divFunc];
	std::vector<std::list<ref_count_ptr<TaskFunction> > >::iterator itrPri;
	for(itrPri=vectPri.begin(); itrPri!=vectPri.end();itrPri++)
	{
		std::list<ref_count_ptr<TaskFunction> > &listFunc = *itrPri;
		std::list<ref_count_ptr<TaskFunction> >::iterator itrFunc;
		for(itrFunc=listFunc.begin();itrFunc!=listFunc.end();itrFunc++)
		{
			if(*itrFunc == NULL)continue;
			const std::type_info& tInfo = typeid(*(*itrFunc)->task_);
			if(info != tInfo)continue;
			(*itrFunc)->bEnable_ = bEnable;
		}
	}
}

/**********************************************************
//TaskInfoPanel
**********************************************************/
TaskInfoPanel::TaskInfoPanel()
{
	addressLastFindManager_=0;
	timeLastUpdate_=0;
	timeUpdateInterval_ = 500;
}
bool TaskInfoPanel::_AddedLogger(HWND hTab)
{
	Create(hTab);

	gstd::WTreeView::Style styleTreeView;
	styleTreeView.SetStyle(WS_CHILD | WS_VISIBLE | 
		TVS_HASLINES | TVS_HASBUTTONS | TVS_LINESATROOT);
	styleTreeView.SetStyleEx(WS_EX_CLIENTEDGE);
	wndTreeView_.Create(hWnd_, styleTreeView);

	WTreeView::ItemStyle treeIteSmtyle;
	treeIteSmtyle.SetMask(TVIF_TEXT|TVIF_PARAM);
	wndTreeView_.CreateRootItem(treeIteSmtyle);

	gstd::WListView::Style styleListView;
	styleListView.SetStyle(WS_CHILD | WS_VISIBLE | 
		LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SINGLESEL| LVS_NOSORTHEADER);
	styleListView.SetStyleEx(WS_EX_CLIENTEDGE);
	styleListView.SetListViewStyleEx(LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES);
	wndListView_.Create(hWnd_, styleListView);
	wndListView_.AddColumn(64, ROW_FUNC_ADDRESS, L"Address");
	wndListView_.AddColumn(32, ROW_FUNC_CLASS, L"Class");
	wndListView_.AddColumn(32, ROW_FUNC_ID, L"ID");
	wndListView_.AddColumn(32, ROW_FUNC_DIVISION, L"Div");
	wndListView_.AddColumn(32, ROW_FUNC_PRIORITY, L"Pri");
	wndListView_.AddColumn(32, ROW_FUNC_ENABLE, L"Enable");
	wndListView_.AddColumn(256, ROW_FUNC_INFO, L"Info");

	wndSplitter_.Create(hWnd_, WSplitter::TYPE_HORIZONTAL);
	wndSplitter_.SetRatioY(0.25f);

	return true;
}
void TaskInfoPanel::LocateParts()
{
	int wx = GetClientX();
	int wy = GetClientY();
	int wWidth = GetClientWidth();
	int wHeight = GetClientHeight();

	int ySplitter = (int)((float)wHeight*wndSplitter_.GetRatioY());
	int heightSplitter = 6;

	wndSplitter_.SetBounds(wx, ySplitter, wWidth, heightSplitter);
	wndTreeView_.SetBounds(wx, wy, wWidth, ySplitter);
	wndListView_.SetBounds(wx, ySplitter+heightSplitter, wWidth, wHeight-ySplitter-heightSplitter);
}
void TaskInfoPanel::Update(TaskManager* taskManager)
{
	if(!IsWindowVisible())return;
	int time = timeGetTime();
	if(abs(time - timeLastUpdate_) < timeUpdateInterval_)return;
	timeLastUpdate_ = time;

	ref_count_ptr<WTreeView::Item> itemRoot = wndTreeView_.GetRootItem();
	itemRoot->SetText(taskManager->GetInfoAsString());
	itemRoot->SetParam((LPARAM)taskManager);
	_UpdateTreeView(taskManager, itemRoot);

	int addressManager = 0;
	ref_count_ptr<WTreeView::Item> itemSelected = wndTreeView_.GetSelectedItem();
	if(itemSelected != NULL)
	{
		addressManager = itemSelected->GetParam();
	}
	_UpdateListView((TaskManager*)addressManager);
}
void TaskInfoPanel::_UpdateTreeView(TaskManager* taskManager, ref_count_ptr<WTreeView::Item> item)
{
	//�o�^
	std::set<int> setAddress;
	{
		std::list<ref_count_ptr<TaskBase> > listTask = taskManager->GetTaskList();
		std::list<ref_count_ptr<TaskBase> >::iterator itrTask;
		for(itrTask = listTask.begin(); itrTask!= listTask.end(); itrTask++)
		{
			if(*itrTask == NULL)continue;
			TaskManager* task = dynamic_cast<TaskManager*>((*itrTask).GetPointer());
			if(task == NULL)continue;

			int address = (int)task;
			ref_count_ptr<WTreeView::Item> itemChild = NULL;
			std::list<ref_count_ptr<WTreeView::Item> > listChild = item->GetChildList();
			std::list<ref_count_ptr<WTreeView::Item> >::iterator itrChild;
			for(itrChild=listChild.begin(); itrChild!=listChild.end();itrChild++)
			{
				ref_count_ptr<WTreeView::Item> iItem = *itrChild;
				LPARAM param = iItem->GetParam();
				if(param != address)continue;
				itemChild = iItem;
			}

			if(itemChild == NULL)
			{
				WTreeView::ItemStyle treeIteSmtyle;
				treeIteSmtyle.SetMask(TVIF_TEXT|TVIF_PARAM);
				itemChild = item->CreateChild(treeIteSmtyle);
			}
			itemChild->SetText(task->GetInfoAsString());
			itemChild->SetParam(address);
			_UpdateTreeView(task, itemChild);
			setAddress.insert(address);
		}
	}

	//�폜
	{
		std::list<ref_count_ptr<WTreeView::Item> > listChild = item->GetChildList();
		std::list<ref_count_ptr<WTreeView::Item> >::iterator itrChild;
		for(itrChild=listChild.begin(); itrChild!=listChild.end();itrChild++)
		{
			ref_count_ptr<WTreeView::Item> iItem = *itrChild;
			LPARAM param = iItem->GetParam();
			if(setAddress.find(param) == setAddress.end())iItem->Delete();
		}	
	}
}
void TaskInfoPanel::_UpdateListView(TaskManager* taskManager)
{
	if(addressLastFindManager_ != (int)taskManager)
	{
		wndListView_.Clear();
	}
		
	if(taskManager == 0)
	{
		wndListView_.Clear();
		return;
	}

	std::set<std::wstring> setKey;
	TaskManager::function_map mapFunc = taskManager->GetFunctionMap();
	std::map<int, std::vector<std::list<ref_count_ptr<TaskFunction> > > >::iterator itrType;
	for(itrType=mapFunc.begin();itrType!=mapFunc.end();itrType++)
	{
		int division = itrType->first;
		int priority = 0;
		std::vector<std::list<ref_count_ptr<TaskFunction> > > *vectPri = &itrType->second;
		std::vector<std::list<ref_count_ptr<TaskFunction> > >::iterator itrPri;
		for(itrPri=vectPri->begin(); itrPri!=vectPri->end();itrPri++)
		{
			std::list<ref_count_ptr<TaskFunction> > &listFunc = *itrPri;
			std::list<ref_count_ptr<TaskFunction> >::iterator itrFunc;
			for(itrFunc=listFunc.begin();itrFunc!=listFunc.end();itrFunc++)
			{
				if(*itrFunc==NULL)continue;
				std::string keyList;
				
				TaskFunction* func = itrFunc->GetPointer();
				int address = (int)func;
				std::wstring key = StringUtility::Format(L"%08x", address);
				int index = wndListView_.GetIndexInColumn(key, ROW_FUNC_ADDRESS);
				if(index == -1)
				{
					index = wndListView_.GetRowCount();
					wndListView_.SetText(index, ROW_FUNC_ADDRESS, key);
				}
				
				std::wstring className = StringUtility::ConvertMultiToWide((char*)typeid(*func).name());
				wndListView_.SetText(index, ROW_FUNC_CLASS, className);
				wndListView_.SetText(index, ROW_FUNC_ID, StringUtility::Format(L"%d", func->GetID()));
				wndListView_.SetText(index, ROW_FUNC_DIVISION, StringUtility::Format(L"%d",division));
				wndListView_.SetText(index, ROW_FUNC_PRIORITY, StringUtility::Format(L"%d",priority));
				wndListView_.SetText(index, ROW_FUNC_ENABLE, StringUtility::Format(L"%d", func->IsEnable()));
				wndListView_.SetText(index, ROW_FUNC_INFO, func->GetInfoAsString());

				setKey.insert(key);
			}
			priority++;
		}
	}

	for(int iRow=0; iRow<wndListView_.GetRowCount();)
	{
		std::wstring key = wndListView_.GetText(iRow, ROW_FUNC_ADDRESS);
		if(setKey.find(key) != setKey.end())iRow++;
		else wndListView_.DeleteRow(iRow);
	}

	addressLastFindManager_ = (int)taskManager;
}
/**********************************************************
//WorkRenderTaskManager
**********************************************************/
WorkRenderTaskManager::WorkRenderTaskManager()
{

}
WorkRenderTaskManager::~WorkRenderTaskManager()
{

}
void WorkRenderTaskManager::InitializeFunctionDivision(int maxPriWork, int maxPriRender)
{
	this->TaskManager::InitializeFunctionDivision(DIV_FUNC_WORK, maxPriWork);
	this->TaskManager::InitializeFunctionDivision(DIV_FUNC_RENDER, maxPriRender);
}
void WorkRenderTaskManager::CallWorkFunction()
{
	CallFunction(DIV_FUNC_WORK);
}
void WorkRenderTaskManager::AddWorkFunction(ref_count_ptr<TaskFunction> func,int pri,int idFunc)
{
	func->SetDelay(1);
	AddFunction(DIV_FUNC_WORK, func, pri, idFunc);
}
void WorkRenderTaskManager::RemoveWorkFunction(TaskBase* task, int idFunc)
{
	RemoveFunction(task, DIV_FUNC_WORK, idFunc);
}
void WorkRenderTaskManager::SetWorkFunctionEnable(bool bEnable)
{
	SetFunctionEnable(bEnable, DIV_FUNC_WORK);
}
void WorkRenderTaskManager::SetWorkFunctionEnable(bool bEnable, int idTask)
{
	SetFunctionEnable(bEnable, idTask, DIV_FUNC_WORK);
}
void WorkRenderTaskManager::SetWorkFunctionEnable(bool bEnable, int idTask, int idFunc)
{
	SetFunctionEnable(bEnable, idTask, DIV_FUNC_WORK, idFunc);
}
void WorkRenderTaskManager::SetWorkFunctionEnable(bool bEnable, TaskBase* task)
{
	SetFunctionEnable(bEnable, task, DIV_FUNC_WORK);
}
void WorkRenderTaskManager::SetWorkFunctionEnable(bool bEnable, TaskBase* task, int idFunc)
{
	SetFunctionEnable(bEnable, task, DIV_FUNC_WORK, idFunc);
}
void WorkRenderTaskManager::SetWorkFunctionEnable(bool bEnable, const std::type_info& info)
{
	SetFunctionEnable(bEnable, info, DIV_FUNC_WORK);
}

void WorkRenderTaskManager::CallRenderFunction()
{
	CallFunction(DIV_FUNC_RENDER);
}
void WorkRenderTaskManager::AddRenderFunction(ref_count_ptr<TaskFunction> func,int pri,int idFunc)
{
	AddFunction(DIV_FUNC_RENDER, func, pri, idFunc);
}
void WorkRenderTaskManager::RemoveRenderFunction(TaskBase* task, int idFunc)
{
	RemoveFunction(task, DIV_FUNC_RENDER, idFunc);
}
void WorkRenderTaskManager::SetRenderFunctionEnable(bool bEnable)
{
	SetFunctionEnable(bEnable, DIV_FUNC_RENDER);
}
void WorkRenderTaskManager::SetRenderFunctionEnable(bool bEnable, int idTask)
{
	SetFunctionEnable(bEnable, idTask, DIV_FUNC_RENDER);
}
void WorkRenderTaskManager::SetRenderFunctionEnable(bool bEnable, int idTask, int idFunc)
{
	SetFunctionEnable(bEnable, idTask, DIV_FUNC_RENDER, idFunc);
}
void WorkRenderTaskManager::SetRenderFunctionEnable(bool bEnable, TaskBase* task)
{
	SetFunctionEnable(bEnable, task, DIV_FUNC_RENDER);
}
void WorkRenderTaskManager::SetRenderFunctionEnable(bool bEnable, TaskBase* task, int idFunc)
{
	SetFunctionEnable(bEnable, task, DIV_FUNC_RENDER, idFunc);
}
void WorkRenderTaskManager::SetRenderFunctionEnable(bool bEnable, const std::type_info& info)
{
	SetFunctionEnable(bEnable, info, DIV_FUNC_RENDER);
}
