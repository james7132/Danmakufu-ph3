#ifndef __GSTD_TASK__
#define __GSTD_TASK__

#include"GstdConstant.hpp"
#include"GstdUtility.hpp"
#include"Logger.hpp"

namespace gstd
{
	class TaskBase;
	class TaskFunction;
	class TaskManager;

	enum
	{
		TASK_FREE_ID=0xffffffff,
		TASK_GROUP_FREE_ID=0xffffffff,
	};
	/**********************************************************
	//TaskFunction
	**********************************************************/
	class TaskFunction : public IStringInfo
	{
		friend TaskManager;
		protected:
			ref_count_ptr<TaskBase> task_;//�^�X�N�ւ̃|�C���^
			int id_;//id
			bool bEnable_;
			int delay_;
		public:
			TaskFunction(){task_=NULL;id_=TASK_FREE_ID;bEnable_=true;delay_ = 0;}
			virtual ~TaskFunction(){}
			virtual void Call() = 0;
			
			ref_count_ptr<TaskBase> GetTask(){return task_;}
			int GetID(){return id_;}
			bool IsEnable(){return bEnable_;}

			int GetDelay(){return delay_;}
			void SetDelay(int delay){delay_ = delay;}
			bool IsDelay(){return delay_ > 0;}

			virtual std::wstring GetInfoAsString();
	};

	template <class T>
	class TTaskFunction : public TaskFunction
	{
		public:
			typedef void (T::*Function)();
		protected:

			Function pFunc;//�����o�֐��|�C���^
		public:
			TTaskFunction(ref_count_ptr<T> task,Function func){task_=task;pFunc=func;}
			virtual void Call()
			{
				if(task_ != NULL)
					((T*)task_.GetPointer()->*pFunc)();
			}

			static ref_count_ptr<TaskFunction> Create(ref_count_ptr<TaskBase> task, Function func)
			{
				ref_count_ptr<T> dTask = ref_count_ptr<T>::DownCast(task);
				return TTaskFunction<T>::Create(dTask, func);
			}
			static ref_count_ptr<TaskFunction> Create(ref_count_ptr<T> task, Function func)
			{
				return new TTaskFunction<T>(task, func);
			}
	};

	/**********************************************************
	//TaskBase
	**********************************************************/
	class TaskBase : public IStringInfo
	{
		friend TaskManager;
		protected:
			_int64 indexTask_;//TaskManager�ɂ���Ă������ӂ̃C���f�b�N�X
			int idTask_;//ID
			int idTaskGroup_;//�O���[�vID

		public:
			TaskBase();
			virtual ~TaskBase();
			int GetTaskID(){return idTask_;}
			_int64 GetTaskIndex(){return indexTask_;}
	};

	/**********************************************************
	//TaskManager
	**********************************************************/
	class TaskInfoPanel;
	class TaskManager : public TaskBase
	{
		friend TaskInfoPanel;
		public:
			typedef std::map<int, std::vector<std::list<ref_count_ptr<TaskFunction> > > > function_map;
		protected:
			static gstd::CriticalSection lockStatic_;
			std::list<ref_count_ptr<TaskBase> > listTask_;//�^�X�N�̌��N���X
			function_map mapFunc_;//�^�X�N�@�\�̃��X�g(divFunc, priority, func)
			_int64 indexTaskManager_;//��ӂ̃C���f�b�N�X
			ref_count_ptr<TaskInfoPanel> panelInfo_;

			void _ArrangeTask();//�K�v�̂Ȃ��Ȃ����̈�폜
			void _CheckInvalidFunctionDivision(int divFunc);

		public:
			TaskManager();
			virtual ~TaskManager();
			void Clear();//�S�^�X�N�폜
			void ClearTask();
			void AddTask(ref_count_ptr<TaskBase> task);//�^�X�N��ǉ�
			ref_count_ptr<TaskBase> GetTask(int idTask);//�w�肵��ID�̃^�X�N���擾
			ref_count_ptr<TaskBase> GetTask(const std::type_info& info);
			void RemoveTask(TaskBase* task);//�w�肵���^�X�N���폜
			void RemoveTask(int idTask);//�^�X�N��ID�ō폜
			void RemoveTaskGroup(int idGroup);//�^�X�N���O���[�v�ō폜
			void RemoveTask(const std::type_info& info);//�N���X�^�ō폜
			void RemoveTaskWithoutTypeInfo(std::set<const std::type_info*> listInfo);//�N���X�^�ȊO�̃^�X�N���폜
			std::list<ref_count_ptr<TaskBase> > GetTaskList(){return listTask_;}

			void InitializeFunctionDivision(int divFunc, int maxPri);
			void CallFunction(int divFunc);//�^�X�N�@�\���s
			void AddFunction(int divFunc, ref_count_ptr<TaskFunction> func,int pri,int idFunc=TASK_FREE_ID);//�^�X�N�@�\�ǉ�
			void RemoveFunction(TaskBase* task);//�^�X�N�@�\�폜
			void RemoveFunction(TaskBase* task,int divFunc, int idFunc);//�^�X�N�@�\�폜
			void RemoveFunction(const std::type_info& info);//�^�X�N�@�\�폜
			function_map GetFunctionMap(){return mapFunc_;}

			void SetFunctionEnable(bool bEnable);//�S�^�X�N�@�\�̏�Ԃ�؂�ւ���
			void SetFunctionEnable(bool bEnable, int divFunc);//�^�X�N�@�\�̏�Ԃ�؂�ւ���
			void SetFunctionEnable(bool bEnable, int idTask, int divFunc);//�^�X�N�@�\�̏�Ԃ�؂�ւ���
			void SetFunctionEnable(bool bEnable, int idTask, int divFunc, int idFunc);//�^�X�N�@�\�̏�Ԃ�؂�ւ���
			void SetFunctionEnable(bool bEnable, TaskBase* task, int divFunc);//�^�X�N�@�\�̏�Ԃ�؂�ւ���
			void SetFunctionEnable(bool bEnable, TaskBase* task, int divFunc, int idFunc);//�^�X�N�@�\�̏�Ԃ�؂�ւ���
			void SetFunctionEnable(bool bEnable, const std::type_info& info, int divFunc);//�^�X�N�@�\�̏�Ԃ�؂�ւ���

			void SetInfoPanel(ref_count_ptr<TaskInfoPanel> panel){panelInfo_ = panel;}
			gstd::CriticalSection& GetStaticLock(){return lockStatic_;}
	};

	/**********************************************************
	//TaskInfoPanel
	**********************************************************/
	class TaskInfoPanel : public WindowLogger::Panel
	{
		protected:
			enum
			{
				ROW_FUNC_ADDRESS = 0,
				ROW_FUNC_CLASS,
				ROW_FUNC_ID,
				ROW_FUNC_DIVISION,
				ROW_FUNC_PRIORITY,
				ROW_FUNC_ENABLE,
				ROW_FUNC_INFO,
			};
			WSplitter wndSplitter_;
			WTreeView wndTreeView_;
			WListView wndListView_;
			int timeLastUpdate_;
			int timeUpdateInterval_;
			int addressLastFindManager_;

			virtual bool _AddedLogger(HWND hTab);
			void _UpdateTreeView(TaskManager* taskManager, ref_count_ptr<WTreeView::Item> item);
			void _UpdateListView(TaskManager* taskManager);
		public:
			TaskInfoPanel();
			void SetUpdateInterval(int time){timeUpdateInterval_=time;}
			virtual void LocateParts();
			virtual void Update(TaskManager* taskManager);
	};

	/**********************************************************
	//WorkRenderTaskManager
	//����A�`��@�\��ێ�����TaskManager
	**********************************************************/
	class WorkRenderTaskManager : public TaskManager
	{
		enum 
		{
			DIV_FUNC_WORK,//����
			DIV_FUNC_RENDER,//�`��
		};

		public:
			WorkRenderTaskManager();
			~WorkRenderTaskManager();
			virtual void InitializeFunctionDivision(int maxPriWork, int maxPriRender);

			//����@�\
			void CallWorkFunction();
			void AddWorkFunction(ref_count_ptr<TaskFunction> func,int pri,int idFunc=TASK_FREE_ID);
			void RemoveWorkFunction(TaskBase* task, int idFunc);
			void SetWorkFunctionEnable(bool bEnable);
			void SetWorkFunctionEnable(bool bEnable, int idTask);
			void SetWorkFunctionEnable(bool bEnable, int idTask, int idFunc);
			void SetWorkFunctionEnable(bool bEnable, TaskBase* task);
			void SetWorkFunctionEnable(bool bEnable, TaskBase* task, int idFunc);
			void SetWorkFunctionEnable(bool bEnable, const std::type_info& info);

			//�`��@�\
			void CallRenderFunction();
			void AddRenderFunction(ref_count_ptr<TaskFunction> func,int pri,int idFunc=TASK_FREE_ID);
			void RemoveRenderFunction(TaskBase* task, int idFunc);
			void SetRenderFunctionEnable(bool bEnable);
			void SetRenderFunctionEnable(bool bEnable, int idTask);
			void SetRenderFunctionEnable(bool bEnable, int idTask, int idFunc);
			void SetRenderFunctionEnable(bool bEnable, TaskBase* task);
			void SetRenderFunctionEnable(bool bEnable, TaskBase* task, int idFunc);
			void SetRenderFunctionEnable(bool bEnable, const std::type_info& info);
	};

}

#endif
