#ifndef __TOUHOUDANMAKUFU_DNHSTG_STAGESCRIPT__
#define __TOUHOUDANMAKUFU_DNHSTG_STAGESCRIPT__

#include"StgCommon.hpp"
#include"StgControlScript.hpp"

class StgStageScriptObjectManager;
class StgStageScript;


/**********************************************************
//StgStageScriptManager
**********************************************************/
class StgStageScriptManager : public StgControlScriptManager
{
	protected:
		StgStageController* stageController_;
		ref_count_ptr<StgStageScriptObjectManager> objManager_;

		_int64 idPlayerScript_;
		_int64 idItemScript_;
		_int64 idShotScript_;

	public:
		StgStageScriptManager(StgStageController* stageController);
		virtual ~StgStageScriptManager();

		virtual void SetError(std::wstring error);
		virtual bool IsError();

		gstd::ref_count_ptr<StgStageScriptObjectManager> GetObjectManager(){return objManager_;}
		virtual ref_count_ptr<ManagedScript> Create(int type);

		_int64 GetPlayerScriptID(){return idPlayerScript_;}
		void SetPlayerScriptID(_int64 id){idPlayerScript_ = id;}
		_int64 GetItemScriptID(){return idItemScript_;}
		void SetItemScriptID(_int64 id){idItemScript_ = id;}
		ref_count_ptr<ManagedScript> GetItemScript();
		_int64 GetShotScriptID(){return idShotScript_;}
		void SetShotScriptID(_int64 id){idShotScript_ = id;}
		ref_count_ptr<ManagedScript> GetShotScript();
};

/**********************************************************
//StgStageScriptObjectManager
**********************************************************/
class StgStageScriptObjectManager : public DxScriptObjectManager
{
		StgStageController* stageController_;

		int idObjPleyer_;

	public:
		StgStageScriptObjectManager(StgStageController* stageController);
		~StgStageScriptObjectManager();
		virtual void RenderObject();
		virtual void RenderObject(int priMin, int priMax);

		int GetPlayerObjectID(){return idObjPleyer_;}
		int CreatePlayerObject();


};


/**********************************************************
//StgStageScript
**********************************************************/
class StgStageScript : public StgControlScript
{
		friend StgStageScriptManager;
	public:
		enum
		{
			TYPE_SYSTEM,
			TYPE_STAGE,
			TYPE_PLAYER,
			TYPE_ITEM,

			TYPE_ALL,
			TYPE_SHOT,
			TYPE_CHILD,
			TYPE_IMMEDIATE,
			TYPE_FADE,


			OBJ_PLAYER = 100,
			OBJ_SPELL_MANAGE,
			OBJ_SPELL,
			OBJ_ENEMY,
			OBJ_ENEMY_BOSS,
			OBJ_ENEMY_BOSS_SCENE,
			OBJ_SHOT,
			OBJ_LOOSE_LASER,//射出型レーザー
			OBJ_STRAIGHT_LASER,//設置型レーザー
			OBJ_CURVE_LASER,//曲がるレーザー
			OBJ_ITEM,

			INFO_LIFE,
			INFO_DAMAGE_RATE_SHOT,
			INFO_DAMAGE_RATE_SPELL,
			INFO_SHOT_HIT_COUNT,

			INFO_TIMER,
			INFO_TIMERF,
			INFO_ORGTIMERF,
			INFO_IS_SPELL,
			INFO_IS_LAST_SPELL,
			INFO_IS_DURABLE_SPELL,
			INFO_SPELL_SCORE,
			INFO_REMAIN_STEP_COUNT,
			INFO_ACTIVE_STEP_LIFE_COUNT,
			INFO_ACTIVE_STEP_TOTAL_MAX_LIFE,
			INFO_ACTIVE_STEP_TOTAL_LIFE,
			INFO_ACTIVE_STEP_LIFE_RATE_LIST,
			INFO_IS_LAST_STEP,
			INFO_PLAYER_SHOOTDOWN_COUNT,
			INFO_PLAYER_SPELL_COUNT,
			INFO_CURRENT_LIFE,
			INFO_CURRENT_LIFE_MAX,

			INFO_ITEM_SCORE,

			INFO_RECT,
			INFO_DELAY_COLOR,
			INFO_BLEND,
			INFO_COLLISION,
			INFO_COLLISION_LIST,

			//イベント
			EV_REQUEST_LIFE = 1,//敵ライフ要求
			EV_REQUEST_TIMER,//敵スペルタイマ要求
			EV_REQUEST_IS_SPELL,//スペルカード宣言
			EV_REQUEST_IS_LAST_SPELL,//ラストスペル
			EV_REQUEST_IS_DURABLE_SPELL,//耐久スペル
			EV_REQUEST_SPELL_SCORE,//スペルカードスコア
			EV_REQUEST_REPLAY_TARGET_COMMON_AREA,//リプレイ対象

			EV_TIMEOUT,//スペルタイムアウト
			EV_START_BOSS_SPELL,//ボススペルカード開始
			EV_GAIN_SPELL,//スペルカード取得
			EV_START_BOSS_STEP,//スペルカード開始
			EV_END_BOSS_STEP,//スペルカード終了

			EV_PLAYER_SHOOTDOWN,//自機ダウン
			EV_PLAYER_SPELL,//自機スペル
			EV_PLAYER_REBIRTH,//自機復帰

			EV_PAUSE_ENTER,//停止開始
			EV_PAUSE_LEAVE,//停止解除

			EV_DELETE_SHOT_IMMEDIATE,
			EV_DELETE_SHOT_TO_ITEM,
			EV_DELETE_SHOT_FADE,

			TARGET_ALL,
			TARGET_ENEMY,
			TARGET_PLAYER,
		};
	protected:

		StgStageController* stageController_;
	public:
		StgStageScript(StgStageController* stageController);
		virtual ~StgStageScript();

		ref_count_ptr<StgStageScriptObjectManager> GetStgObjectManager();

		//STG共通関数：共通データ
		static gstd::value Func_SaveCommonDataAreaToReplayFile(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_LoadCommonDataAreaFromReplayFile(gstd::script_machine* machine, int argc, gstd::value const * argv);

		//STG共通関数：システム関連
		static gstd::value Func_GetMainStgScriptPath(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_GetMainStgScriptDirectory(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_SetStgFrame(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_SetItemRenderPriorityI(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_SetShotRenderPriorityI(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_GetStgFrameRenderPriorityMinI(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_GetStgFrameRenderPriorityMaxI(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_GetItemRenderPriorityI(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_GetShotRenderPriorityI(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_GetPlayerRenderPriorityI(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_GetCameraFocusPermitPriorityI(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_CloseStgScene(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_GetReplayFps(gstd::script_machine* machine, int argc, gstd::value const * argv);

		//STG共通関数：自機
		static gstd::value Func_GetPlayerObjectID(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_GetPlayerScriptID(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_SetPlayerSpeed(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_SetPlayerClip(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_SetPlayerLife(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_SetPlayerSpell(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_SetPlayerPower(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_SetPlayerInvincibilityFrame(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_SetPlayerDownStateFrame(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_SetPlayerRebirthFrame(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_SetPlayerRebirthLossFrame(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_SetPlayerAutoItemCollectLine(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_SetForbidPlayerShot(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_SetForbidPlayerSpell(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_GetPlayerX(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_GetPlayerY(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_GetPlayerState(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_GetPlayerSpeed(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_GetPlayerClip(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_GetPlayerLife(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_GetPlayerSpell(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_GetPlayerPower(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_GetPlayerInvincibilityFrame(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_GetPlayerDownStateFrame(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_GetPlayerRebirthFrame(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_GetAngleToPlayer(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_IsPermitPlayerShot(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_IsPermitPlayerSpell(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_IsPlayerLastSpellWait(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_IsPlayerSpellActive(gstd::script_machine* machine, int argc, gstd::value const * argv);


		//STG共通関数：敵
		static gstd::value Func_GetEnemyBossSceneObjectID(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_GetEnemyBossObjectID(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_GetAllEnemyID(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_GetIntersectionRegistedEnemyID(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_GetAllEnemyIntersectionPosition(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_GetEnemyIntersectionPosition(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_GetEnemyIntersectionPositionByIdA1(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_GetEnemyIntersectionPositionByIdA2(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_LoadEnemyShotData(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_ReloadEnemyShotData(gstd::script_machine* machine, int argc, gstd::value const * argv);


		//STG共通関数：弾
		static gstd::value Func_DeleteShotAll(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_DeleteShotInCircle(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_CreateShotA1(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_CreateShotA2(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_CreateShotOA1(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_CreateShotB1(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_CreateShotB2(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_CreateShotOB1(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_CreateLooseLaserA1(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_CreateStraightLaserA1(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_CreateCurveLaserA1(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_SetShotIntersectionCircle(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_SetShotIntersectionLine(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_GetShotIdInCircleA1(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_GetShotIdInCircleA2(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_GetShotCount(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_SetShotAutoDeleteClip(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_GetShotDataInfoA1(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_StartShotScript(gstd::script_machine* machine, int argc, gstd::value const * argv);


		//STG共通関数：アイテム
		static gstd::value Func_CreateItemA1(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_CreateItemA2(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_CreateItemU1(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_CreateItemU2(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_CreateItemScore(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_CollectAllItems(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_CollectItemsByType(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_CollectItemsInCircle(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_CancelCollectItems(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_StartItemScript(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_SetDefaultBonusItemEnable(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_LoadItemData(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_ReloadItemData(gstd::script_machine* machine, int argc, gstd::value const * argv);


		//STG共通関数：その他
		static gstd::value Func_StartSlow(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_StopSlow(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_IsIntersected_Line_Circle(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_IsIntersected_Obj_Obj(gstd::script_machine* machine, int argc, gstd::value const * argv);

		//STG共通関数：移動オブジェクト操作
		static gstd::value Func_ObjMove_SetX(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_ObjMove_SetY(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_ObjMove_SetPosition(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_ObjMove_SetSpeed(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_ObjMove_SetAngle(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_ObjMove_SetAcceleration(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_ObjMove_SetMaxSpeed(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_ObjMove_SetAngularVelocity(gstd::script_machine* machine, int argc, gstd::value const * argv);	
		static gstd::value Func_ObjMove_SetDestAtSpeed(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_ObjMove_SetDestAtFrame(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_ObjMove_SetDestAtWeight(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_ObjMove_AddPatternA1(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_ObjMove_AddPatternA2(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_ObjMove_AddPatternA3(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_ObjMove_AddPatternA4(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_ObjMove_AddPatternB1(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_ObjMove_AddPatternB2(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_ObjMove_AddPatternB3(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_ObjMove_GetX(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_ObjMove_GetY(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_ObjMove_GetSpeed(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_ObjMove_GetAngle(gstd::script_machine* machine, int argc, gstd::value const * argv);

		//STG共通関数：敵オブジェクト操作
		static gstd::value Func_ObjEnemy_Create(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_ObjEnemy_Regist(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_ObjEnemy_GetInfo(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_ObjEnemy_SetLife(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_ObjEnemy_AddLife(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_ObjEnemy_SetDamageRate(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_ObjEnemy_AddIntersectionCircleA(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_ObjEnemy_SetIntersectionCircleToShot(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_ObjEnemy_SetIntersectionCircleToPlayer(gstd::script_machine* machine, int argc, gstd::value const * argv);


		//STG共通関数：敵ボスシーンオブジェクト操作
		static gstd::value Func_ObjEnemyBossScene_Create(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_ObjEnemyBossScene_Regist(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_ObjEnemyBossScene_Add(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_ObjEnemyBossScene_LoadInThread(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_ObjEnemyBossScene_GetInfo(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_ObjEnemyBossScene_SetSpellTimer(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_ObjEnemyBossScene_StartSpell(gstd::script_machine* machine, int argc, gstd::value const * argv);

		//STG共通関数：弾オブジェクト操作
		static gstd::value Func_ObjShot_Create(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_ObjShot_Regist(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_ObjShot_SetAutoDelete(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_ObjShot_SetDeleteFrame(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_ObjShot_FadeDelete(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_ObjShot_SetDelay(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_ObjShot_SetSpellResist(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_ObjShot_SetGraphic(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_ObjShot_SetSourceBlendType(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_ObjShot_SetDamage(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_ObjShot_SetPenetration(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_ObjShot_SetEraseShot(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_ObjShot_SetSpellFactor(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_ObjShot_ToItem(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_ObjShot_AddShotA1(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_ObjShot_AddShotA2(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_ObjShot_SetIntersectionCircleA1(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_ObjShot_SetIntersectionCircleA2(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_ObjShot_SetIntersectionLine(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_ObjShot_SetIntersectionEnable(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_ObjShot_SetItemChange(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_ObjShot_GetDelay(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_ObjShot_GetDamage(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_ObjShot_GetPenetration(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_ObjShot_IsSpellResist(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_ObjShot_GetImageID(gstd::script_machine* machine, int argc, gstd::value const * argv);

		static gstd::value Func_ObjLaser_SetLength(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_ObjLaser_SetRenderWidth(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_ObjLaser_SetIntersectionWidth(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_ObjLaser_SetInvalidLength(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_ObjLaser_SetGrazeInvalidFrame(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_ObjLaser_SetItemDistance(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_ObjLaser_GetLength(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_ObjLaser_GetRenderWidth(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_ObjLaser_GetIntersectionWidth(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_ObjStLaser_SetAngle(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_ObjStLaser_GetAngle(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_ObjStLaser_SetSource(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_ObjCrLaser_SetTipDecrement(gstd::script_machine* machine, int argc, gstd::value const * argv);

		//STG共通関数：アイテムオブジェクト操作
		static gstd::value Func_ObjItem_Create(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_ObjItem_Regist(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_ObjItem_SetItemID(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_ObjItem_SetRenderScoreEnable(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_ObjItem_SetAutoCollectEnable(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_ObjItem_SetDefinedMovePatternA1(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_ObjItem_GetInfo(gstd::script_machine* machine, int argc, gstd::value const * argv);

		//STG共通関数：自機オブジェクト操作
		static gstd::value Func_ObjPlayer_AddIntersectionCircleA1(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_ObjPlayer_AddIntersectionCircleA2(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_ObjPlayer_ClearIntersection(gstd::script_machine* machine, int argc, gstd::value const * argv);


		//STG共通関数：当たり判定オブジェクト操作
		static gstd::value Func_ObjCol_IsIntersected(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_ObjCol_GetListOfIntersectedEnemyID(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_ObjCol_GetIntersectedCount(gstd::script_machine* machine, int argc, gstd::value const * argv);
};


/**********************************************************
//StgSystemScript
**********************************************************/
class StgStageSystemScript : public StgStageScript
{
	public:
		StgStageSystemScript(StgStageController* stageController);
		virtual ~StgStageSystemScript();

		//システム専用関数：システム操作

};

/**********************************************************
//StgStageItemScript
**********************************************************/
class StgStageItemScript : public StgStageScript
{
	public:
		enum
		{
			EV_GET_ITEM = 1100,
		};
	public:
		StgStageItemScript(StgStageController* stageController);
		virtual ~StgStageItemScript();

		//システム専用関数：アイテム操作

};

/**********************************************************
//StgStageShotScript
**********************************************************/
class StgStageShotScript : public StgStageScript
{
	public:
		enum
		{

		};
	public:
		StgStageShotScript(StgStageController* stageController);
		virtual ~StgStageShotScript();

		//システム専用関数：アイテム操作
		static gstd::value Func_SetShotDeleteEventEnable(gstd::script_machine* machine, int argc, gstd::value const * argv);

};


/**********************************************************
//StgStagePlayerScript
**********************************************************/
class StgStagePlayerScript : public StgStageScript
{
	public:
		enum
		{
			EV_REQUEST_SPELL = 1000,
			EV_GRAZE,
			EV_HIT,
		};
	public:
		StgStagePlayerScript(StgStageController* stageController);
		virtual ~StgStagePlayerScript();

		//自機専用関数
		static gstd::value Func_CreatePlayerShotA1(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_CallSpell(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_LoadPlayerShotData(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_ReloadPlayerShotData(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_GetSpellManageObject(gstd::script_machine* machine, int argc, gstd::value const * argv);
	
		//自機専用関数：スペルオブジェクト操作
		static gstd::value Func_ObjSpell_Create(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_ObjSpell_Regist(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_ObjSpell_SetDamage(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_ObjSpell_SetPenetration(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_ObjSpell_SetEraseShot(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_ObjSpell_SetIntersectionCircle(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_ObjSpell_SetIntersectionLine(gstd::script_machine* machine, int argc, gstd::value const * argv);

};


#endif
