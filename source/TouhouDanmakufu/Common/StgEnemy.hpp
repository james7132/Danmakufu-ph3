#ifndef __TOUHOUDANMAKUFU_DNHSTG_ENEMY__
#define __TOUHOUDANMAKUFU_DNHSTG_ENEMY__

#include"StgCommon.hpp"
#include"StgIntersection.hpp"

class StgEnemyObject;
class StgEnemyBossSceneObject;
/**********************************************************
//StgEnemyManager
**********************************************************/
class StgEnemyManager
{
		StgStageController* stageController_;
		std::list<ref_count_ptr<StgEnemyObject>::unsync > listObj_;
		ref_count_ptr<StgEnemyBossSceneObject>::unsync objBossScene_;
	public:
		StgEnemyManager(StgStageController* stageController);
		virtual ~StgEnemyManager();
		void Work();
		void RegistIntersectionTarget();

		void AddEnemy(ref_count_ptr<StgEnemyObject>::unsync obj){listObj_.push_back(obj);}
		int GetEnemyCount(){return listObj_.size();}

		void SetBossSceneObject(ref_count_ptr<StgEnemyBossSceneObject>::unsync obj);
		ref_count_ptr<StgEnemyBossSceneObject>::unsync GetBossSceneObject();
		std::list<ref_count_ptr<StgEnemyObject>::unsync >& GetEnemyList(){return listObj_;}

};

/**********************************************************
//StgEnemyObject
**********************************************************/
class StgEnemyObject : public DxScriptSpriteObject2D, public StgMoveObject, public StgIntersectionObject
{
	protected:
		StgStageController* stageController_;

		double life_;
		double rateDamageShot_;
		double rateDamageSpell_;
		int intersectedPlayerShotCount_;

		virtual void _Move();
		virtual void _AddRelativeIntersection();
	public:
		StgEnemyObject(StgStageController* stageController);
		virtual ~StgEnemyObject();

		virtual void Work();
		virtual void Activate();
		virtual void Intersect(ref_count_ptr<StgIntersectionTarget>::unsync ownTarget, ref_count_ptr<StgIntersectionTarget>::unsync otherTarget);
		virtual void ClearEnemyObject(){ClearIntersectionRelativeTarget();}
		virtual void RegistIntersectionTarget();

		virtual void SetX(double x){posX_ = x; DxScriptRenderObject::SetX(x);}
		virtual void SetY(double y){posY_ = y; DxScriptRenderObject::SetY(y);}

		ref_count_ptr<StgEnemyObject>::unsync GetOwnObject();
		double GetLife(){return life_;}
		void SetLife(double life){life_ = life;}
		void AddLife(double inc){life_ += inc; life_ = max(life_, 0);}
		void SetDamageRate(double rateShot, double rateSpell){rateDamageShot_ = rateShot; rateDamageSpell_ = rateSpell;}
		double GetShotDamageRate(){return rateDamageShot_;}
		double GetSpellDamageRate(){return rateDamageSpell_;}
		int GetIntersectedPlayerShotCount(){return intersectedPlayerShotCount_;}
};

/**********************************************************
//StgEnemyBossObject
**********************************************************/
class StgEnemyBossObject : public StgEnemyObject
{
	private:
		int timeSpellCard_;
	public:
		StgEnemyBossObject(StgStageController* stageController);
};

/**********************************************************
//StgEnemyBossSceneObject
**********************************************************/
class StgEnemyBossSceneData;
class StgEnemyBossSceneObject : public DxScriptObjectBase
{
	private:
		StgStageController* stageController_;
		volatile bool bLoad_;

		int dataStep_;
		int dataIndex_;
		ref_count_ptr<StgEnemyBossSceneData>::unsync activeData_;
		std::vector<std::vector<ref_count_ptr<StgEnemyBossSceneData>::unsync > > listData_;

		bool _NextStep();
	public:
		StgEnemyBossSceneObject(StgStageController* stageController);
		virtual void Work();
		virtual void Activate();
		virtual void Render(){}//何もしない
		virtual void SetRenderState(){}//何もしない

		void AddData(int step, ref_count_ptr<StgEnemyBossSceneData>::unsync data);
		ref_count_ptr<StgEnemyBossSceneData>::unsync GetActiveData(){return activeData_;}
		void LoadAllScriptInThread();

		int GetRemainStepCount();
		int GetActiveStepLifeCount();
		double GetActiveStepTotalMaxLife();
		double GetActiveStepTotalLife();
		double GetActiveStepLife(int index);
		std::vector<double> GetActiveStepLifeRateList();
		int GetDataStep(){return dataStep_;}
		int GetDataIndex(){return dataIndex_;}

		void AddPlayerShootDownCount();
		void AddPlayerSpellCount();
};

class StgEnemyBossSceneData
{
	private:
		std::wstring path_;
		_int64 isScript_;
		std::vector<double> listLife_;
		std::vector<ref_count_ptr<StgEnemyBossObject>::unsync > listEnemyObject_;
		int countCreate_;//ボス生成数。listEnemyObject_を超えて生成しようとしたらエラー。
		bool bReadyNext_;

		bool bSpell_;//スペルカード
		bool bLastSpell_;//ラストスペル
		bool bDurable_;//耐久スペル
		_int64 scoreSpell_;
		int timerSpellOrg_;//初期タイマー フレーム単位 -1で無効
		int timerSpell_;//タイマー フレーム単位 -1で無効
		int countPlayerShootDown_;//自機撃破数
		int countPlayerSpell_;//自機スペル使用数

	public:
		StgEnemyBossSceneData();
		virtual ~StgEnemyBossSceneData(){}
		std::wstring GetPath(){return path_;}
		void SetPath(std::wstring path){path_ = path;}
		_int64 GetScriptID(){return isScript_;}
		void SetScriptID(_int64 id){isScript_ = id;}
		std::vector<double>& GetLifeList(){return listLife_;}
		void SetLifeList(std::vector<double>& list){listLife_ = list;}
		std::vector<ref_count_ptr<StgEnemyBossObject>::unsync >& GetEnemyObjectList(){return listEnemyObject_;}
		void SetEnemyObjectList(std::vector<ref_count_ptr<StgEnemyBossObject>::unsync >& list){listEnemyObject_ = list;}
		int GetEnemyBossIdInCreate();
		bool IsReadyNext(){return bReadyNext_;}
		void SetReadyNext(){bReadyNext_ = true;}

		_int64 GetCurrentSpellScore();
		_int64 GetSpellScore(){return scoreSpell_;}
		void SetSpellScore(_int64 score){scoreSpell_ = score;}
		int GetSpellTimer(){return timerSpell_;}
		void SetSpellTimer(int timer){timerSpell_ = timer;}
		int GetOriginalSpellTimer(){return timerSpellOrg_;}
		void SetOriginalSpellTimer(int timer){timerSpellOrg_ = timer; timerSpell_ = timer;}
		bool IsSpellCard(){return bSpell_;}
		void SetSpellCard(bool b){bSpell_ = b;}
		bool IsLastSpell(){return bLastSpell_;}
		void SetLastSpell(bool b){bLastSpell_ = b;}
		bool IsDurable(){return bDurable_;}
		void SetDurable(bool b){bDurable_ = b;}

		void AddPlayerShootDownCount(){countPlayerShootDown_++;}
		int GetPlayerShootDownCount(){return countPlayerShootDown_;}
		void AddPlayerSpellCount(){countPlayerSpell_++;}
		int GetPlayerSpellCount(){return countPlayerSpell_;}
};

#endif
