#ifndef __TOUHOUDANMAKUFU_DNHSTG_INTERSECTION__
#define __TOUHOUDANMAKUFU_DNHSTG_INTERSECTION__

#include"StgCommon.hpp"

class StgIntersectionSpace;
class StgIntersectionCheckList;
class StgIntersectionTarget;
class StgIntersectionObject;
class StgIntersectionTargetPoint;

/**********************************************************
//StgIntersectionManager
//下記を参考
//http://marupeke296.com/COL_2D_No8_QuadTree.html
**********************************************************/
class StgIntersectionManager : public ObjectPool<StgIntersectionTarget, false>
{
	private:
		enum
		{
			SPACE_PLAYER_ENEMY = 0,//自機-敵、敵弾
			SPACE_PLAYERSOHT_ENEMY,//自弾,スペル-敵
			SPACE_PLAYERSHOT_ENEMYSHOT,//自弾,スペル-敵弾
		};
		std::vector<ref_count_ptr<StgIntersectionSpace> > listSpace_;
		std::vector<StgIntersectionTargetPoint> listEnemyTargetPoint_;
		std::vector<StgIntersectionTargetPoint> listEnemyTargetPointNext_;

		virtual void _ResetPoolObject(gstd::ref_count_ptr<StgIntersectionTarget>::unsync& obj);
		virtual gstd::ref_count_ptr<StgIntersectionTarget>::unsync _CreatePoolObject(int type);
	public:
		StgIntersectionManager();
		virtual ~StgIntersectionManager();
		void Work();

		void AddTarget(ref_count_ptr<StgIntersectionTarget>::unsync target);
		void AddEnemyTargetToShot(ref_count_ptr<StgIntersectionTarget>::unsync target);
		void AddEnemyTargetToPlayer(ref_count_ptr<StgIntersectionTarget>::unsync target);
		std::vector<StgIntersectionTargetPoint>* GetAllEnemyTargetPoint(){return &listEnemyTargetPoint_;}

		void CheckDeletedObject(std::string funcName);

		static bool IsIntersected(ref_count_ptr<StgIntersectionTarget>::unsync& target1, ref_count_ptr<StgIntersectionTarget>::unsync& target2);
		static bool IsIntersected(DxCircle& circle, DxWidthLine& line);
		static bool IsIntersected(DxWidthLine& line1, DxWidthLine& line2);
};

/**********************************************************
//StgIntersectionSpace
//以下サイトを参考
//　○×（まるぺけ）つくろーどっとコム
//　http://marupeke296.com/
**********************************************************/
class StgIntersectionSpace
{
	enum
	{
		MAX_LEVEL = 9,
		TYPE_A = 0,
		TYPE_B = 1,
	};
	protected:
		//Cell TARGETA/B listTarget
		std::vector<std::vector<std::vector<ref_count_ptr<StgIntersectionTarget>::unsync > > >listCell_;
		int listCountLevel_[MAX_LEVEL + 1];	// 各レベルのセル数
		double spaceWidth_; // 領域のX軸幅
		double spaceHeight_; // 領域のY軸幅
		double spaceLeft_; // 領域の左側（X軸最小値）
		double spaceTop_; // 領域の上側（Y軸最小値）
		double unitWidth_; // 最小レベル空間の幅単位
		double unitHeight_; // 最小レベル空間の高単位
		int countCell_; // 空間の数
		int unitLevel_; // 最下位レベル
		ref_count_ptr<StgIntersectionCheckList>::unsync listCheck_;

		unsigned int _GetMortonNumber( float left, float top, float right, float bottom );
		unsigned int  _BitSeparate32( unsigned int  n );
		unsigned short _Get2DMortonNumber( unsigned short x, unsigned short y );
		unsigned int  _GetPointElem( float pos_x, float pos_y );
		void _WriteIntersectionCheckList(int indexSpace, ref_count_ptr<StgIntersectionCheckList>::unsync& listCheck, std::vector<std::vector<ref_count_ptr<StgIntersectionTarget>::unsync > > &listStack);
	public:
		StgIntersectionSpace();
		virtual ~StgIntersectionSpace();
		bool Initialize(int level, int left, int top, int right, int bottom);
		bool RegistTarget(int type, ref_count_ptr<StgIntersectionTarget>::unsync& target);
		bool RegistTargetA(ref_count_ptr<StgIntersectionTarget>::unsync& target){return RegistTarget(TYPE_A, target);}
		bool RegistTargetB(ref_count_ptr<StgIntersectionTarget>::unsync& target){return RegistTarget(TYPE_B, target);}
		void ClearTarget();
		ref_count_ptr<StgIntersectionCheckList>::unsync CreateIntersectionCheckList();
};

class StgIntersectionCheckList
{
		int count_;
		std::vector<ref_count_ptr<StgIntersectionTarget>::unsync > listTargetA_;
		std::vector<ref_count_ptr<StgIntersectionTarget>::unsync > listTargetB_;
	public:
		StgIntersectionCheckList(){count_ = 0;}
		virtual ~StgIntersectionCheckList(){}

		void Clear(){count_ = 0;}
		int GetCheckCount(){return count_;}
		void Add(ref_count_ptr<StgIntersectionTarget>::unsync& targetA, ref_count_ptr<StgIntersectionTarget>::unsync& targetB)
		{
			if(listTargetA_.size() <= count_)
			{
				listTargetA_.push_back(targetA);
				listTargetB_.push_back(targetB);
			}
			else
			{
				listTargetA_[count_] = targetA;
				listTargetB_[count_] = targetB;
			}
			count_++;
		}
		ref_count_ptr<StgIntersectionTarget>::unsync GetTargetA(int index){ref_count_ptr<StgIntersectionTarget>::unsync res = listTargetA_[index];listTargetA_[index]=NULL;return res;}
		ref_count_ptr<StgIntersectionTarget>::unsync GetTargetB(int index){ref_count_ptr<StgIntersectionTarget>::unsync res = listTargetB_[index];listTargetB_[index]=NULL;return res;}
};

class StgIntersectionObject
{
	protected:
		bool bIntersected_;//衝突判定
		int intersectedCount_;
		std::vector<ref_count_ptr<StgIntersectionTarget>::unsync > listRelativeTarget_;
		std::vector<DxCircle> listOrgCircle_;
		std::vector<DxWidthLine> listOrgLine_;
		std::vector<int> listIntersectedID_;

	public:
		StgIntersectionObject(){bIntersected_ = false; intersectedCount_ = 0;}
		virtual ~StgIntersectionObject(){}
		virtual void Intersect(ref_count_ptr<StgIntersectionTarget>::unsync ownTarget, ref_count_ptr<StgIntersectionTarget>::unsync otherTarget) = 0;
		void ClearIntersected(){bIntersected_ = false; intersectedCount_ = 0;}
		bool IsIntersected(){return bIntersected_;}
		void SetIntersected(){bIntersected_ = true; intersectedCount_++;}
		int GetIntersectedCount(){return intersectedCount_;}
		void ClearIntersectedIdList(){if(listIntersectedID_.size() > 0)listIntersectedID_.clear();}
		void AddIntersectedId(int id){listIntersectedID_.push_back(id);}
		std::vector<int>& GetIntersectedIdList(){return listIntersectedID_;}

		void ClearIntersectionRelativeTarget();
		void AddIntersectionRelativeTarget(ref_count_ptr<StgIntersectionTarget>::unsync target);
		ref_count_ptr<StgIntersectionTarget>::unsync GetIntersectionRelativeTarget(int index){return listRelativeTarget_[index];}

		void UpdateIntersectionRelativeTarget(int posX, int posY, double angle);
		void RegistIntersectionRelativeTarget(StgIntersectionManager* manager);
		int GetIntersectionRelativeTargetCount(){return listRelativeTarget_.size();}
		int GetDxScriptObjectID();

		virtual std::vector<ref_count_ptr<StgIntersectionTarget>::unsync > GetIntersectionTargetList(){return std::vector<ref_count_ptr<StgIntersectionTarget>::unsync >();}
};

/**********************************************************
//StgIntersectionTarget
**********************************************************/
class StgIntersectionTarget : public IStringInfo
{
	friend StgIntersectionManager;
	public:
		enum
		{
			SHAPE_CIRCLE = 0,
			SHAPE_LINE = 1,

			TYPE_PLAYER,
			TYPE_PLAYER_SHOT,
			TYPE_PLAYER_SPELL,
			TYPE_ENEMY,
			TYPE_ENEMY_SHOT,
		};

	protected:
		int mortonNo_;
		int typeTarget_;
		int shape_;
		ref_count_weak_ptr<StgIntersectionObject>::unsync obj_;

	public:
		StgIntersectionTarget(){mortonNo_ = -1 ; }
		virtual ~StgIntersectionTarget(){}
		virtual RECT GetIntersectionSapceRect() = 0;

		int GetTargetType(){return typeTarget_;}
		void SetTargetType(int type){typeTarget_ = type;}
		int GetShape(){return shape_;}
		ref_count_weak_ptr<StgIntersectionObject>::unsync GetObject(){return obj_;}
		void SetObject(ref_count_weak_ptr<StgIntersectionObject>::unsync obj){obj_ = obj;}

		int GetMortonNumber(){return mortonNo_;}
		void SetMortonNumber(int no){mortonNo_ = no;}
		void ClearObjectIntersectedIdList();

		virtual std::wstring GetInfoAsString();
};

class StgIntersectionTarget_Circle : public StgIntersectionTarget
{
	friend StgIntersectionManager;
		DxCircle circle_;
	
	protected:


	public:
		StgIntersectionTarget_Circle(){shape_ = SHAPE_CIRCLE;}
		virtual ~StgIntersectionTarget_Circle(){}
		virtual RECT GetIntersectionSapceRect()
		{
			DirectGraphics* graphics = DirectGraphics::GetBase();
			int screenWidth = graphics->GetScreenWidth();
			int screenHeight = graphics->GetScreenWidth();

			double x = circle_.GetX();
			double y = circle_.GetY();
			double r = circle_.GetR();
			RECT rect = {(int)(x - r), (int)(y - r), (int)(x + r), (int)(y + r)};
			rect.left = max(rect.left, 0);
			rect.left = min(rect.left, screenWidth);
			rect.top = max(rect.top, 0);
			rect.top = min(rect.top, screenHeight);

			rect.right = max(rect.right, 0);
			rect.right = min(rect.right, screenWidth);
			rect.bottom = max(rect.bottom, 0);
			rect.bottom = min(rect.bottom, screenHeight);
			return rect;
		}

		DxCircle& GetCircle(){return circle_;}
		void SetCircle(DxCircle& circle){circle_ = circle;}

};

class StgIntersectionTarget_Line : public StgIntersectionTarget
{
	friend StgIntersectionManager;
		DxWidthLine line_;

	protected:
		StgIntersectionTarget_Line(){shape_ = SHAPE_LINE;}

	public:
		virtual ~StgIntersectionTarget_Line(){}
		virtual RECT GetIntersectionSapceRect()
		{
			double x1 = line_.GetX1();
			double y1 = line_.GetY1();
			double x2 = line_.GetX2();
			double y2 = line_.GetY2();
			double width = line_.GetWidth();
			if(x1 > x2)
			{
				double tx = x1;
				x1 = x2;
				x2 = tx;
			}
			if(y1 > y2)
			{
				double ty = y1;
				y1 = y2;
				y2 = ty;
			}

			x1 -= width;
			x2 += width;
			y1 -= width;
			y2 += width;

			DirectGraphics* graphics = DirectGraphics::GetBase();
			int screenWidth = graphics->GetScreenWidth();
			int screenHeight = graphics->GetScreenWidth();
			x1 = min(x1, screenWidth);
			x1 = max(x1, 0);
			x2 = min(x2, screenWidth);
			x2 = max(x2, 0);

			y1 = min(y1, screenHeight);
			y1 = max(y1, 0);
			y2 = min(y2, screenHeight);
			y2 = max(y2, 0);

			//RECT rect = {x1 - width, y1 - width, x2 + width, y2 + width};
			RECT rect = {(int)x1, (int)y1, (int)x2, (int)y2};
			return rect;
		}

		DxWidthLine& GetLine(){return line_;}
		void SetLine(DxWidthLine& line){line_ = line;}

};

/**********************************************************
//StgIntersectionTargetPoint
**********************************************************/
class StgIntersectionTargetPoint
{
	private:
		POINT pos_;
		int idObject_;

	public:
		POINT& GetPoint(){return pos_;}
		void SetPoint(POINT& pos){pos_ = pos;}
		int GetObjectID(){return idObject_;}
		void SetObjectID(int id){idObject_ = id;}
};

#endif

