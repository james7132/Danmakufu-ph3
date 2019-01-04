#ifndef __DIRECTX_UTILITY__
#define __DIRECTX_UTILITY__

#include"DxConstant.hpp"

namespace directx
{
	/**********************************************************
	//ColorAccess
	**********************************************************/
	class ColorAccess
	{
		public: 
			enum
			{
				BIT_ALPHA = 24,
				BIT_RED = 16,
				BIT_GREEN = 8,
				BIT_BLUE = 0,
			};
			static int GetColorA(D3DCOLOR& color);
			static D3DCOLOR& SetColorA(D3DCOLOR& color, int alpha);
			static int GetColorR(D3DCOLOR color);
			static D3DCOLOR& SetColorR(D3DCOLOR& color, int red);
			static int GetColorG(D3DCOLOR& color);
			static D3DCOLOR& SetColorG(D3DCOLOR& color, int green);
			static int GetColorB(D3DCOLOR& color);
			static D3DCOLOR& SetColorB(D3DCOLOR& color, int blue);

			static D3DCOLORVALUE SetColor(D3DCOLORVALUE value, D3DCOLOR color);
			static D3DMATERIAL9 SetColor(D3DMATERIAL9 mat, D3DCOLOR color);
			static D3DCOLOR& ApplyAlpha(D3DCOLOR& color, double alpha);

			static D3DCOLOR& SetColorHSV(D3DCOLOR& color, int hue, int saturation, int value);
	};


	/**********************************************************
	//衝突判定用図形
	**********************************************************/
	class DxCircle
	{
		private:
			double x_;
			double y_;
			double r_;
		public:
			DxCircle(){x_ = 0; y_ = 0; r_ = 0;}
			DxCircle(double x, double y, double r){x_ = x; y_ = y; r_ = r;}
			virtual ~DxCircle(){}
			double GetX(){return x_;}
			void SetX(float x){x_ = x;}
			double GetY(){return y_;}
			void SetY(float y){y_ = y;}
			double GetR(){return r_;}
			void SetR(float r){r_ = r;}
	};

	class DxWidthLine
	{
		//幅のある線分
		private:
			double posX1_;
			double posY1_;
			double posX2_;
			double posY2_;
			double width_;
		public:
			DxWidthLine(){posX1_ = 0 ; posY1_ = 0 ; posX2_ = 0 ; posY2_ = 0 ; width_ = 0;}
			DxWidthLine(double x1, double y1, double x2, double y2, double width){posX1_ = x1 ; posY1_ = y1 ; posX2_ = x2 ; posY2_ = y2 ; width_ = width;}
			virtual ~DxWidthLine(){}
			double GetX1(){return posX1_;}
			double GetY1(){return posY1_;}
			double GetX2(){return posX2_;}
			double GetY2(){return posY2_;}
			double GetWidth(){return width_;}
	};

	class DxLine3D
	{
		private:
			D3DXVECTOR3 vertex_[2];
		public:
			DxLine3D(){};
			DxLine3D(const D3DXVECTOR3 &p1,const D3DXVECTOR3 &p2)
			{
				vertex_[0] = p1;
				vertex_[1] = p2;
			}

			D3DXVECTOR3& GetPosition(int index){return vertex_[index];}
			D3DXVECTOR3& GetPosition1(){return vertex_[0];}
			D3DXVECTOR3& GetPosition2(){return vertex_[1];}

	};

	class DxTriangle
	{
		private:
			D3DXVECTOR3 vertex_[3];
			D3DXVECTOR3 normal_;

			void _Compute()
			{
				D3DXVECTOR3 lv[3];
				lv[0] = vertex_[1] - vertex_[0];
				lv[0] = *D3DXVec3Normalize(&D3DXVECTOR3(), &lv[0]);

				lv[1] = vertex_[2] - vertex_[1];
				lv[1] = *D3DXVec3Normalize(&D3DXVECTOR3(), &lv[1]);

				lv[2] = vertex_[0] - vertex_[2];
				lv[2] = *D3DXVec3Normalize(&D3DXVECTOR3(), &lv[2]);

				D3DXVECTOR3 cross = *D3DXVec3Cross(&D3DXVECTOR3() ,&lv[0], &lv[1]);
				normal_ = *D3DXVec3Normalize(&D3DXVECTOR3(), &cross);
			}
		public:
			DxTriangle(){}
			DxTriangle(const D3DXVECTOR3 &p1,const D3DXVECTOR3 &p2,const D3DXVECTOR3 &p3)
			{
				vertex_[0] = p1;
				vertex_[1] = p2;
				vertex_[2] = p3;
				_Compute();
			}

			D3DXVECTOR3& GetPosition(int index){return vertex_[index];}
			D3DXVECTOR3& GetPosition1(){return vertex_[0];}
			D3DXVECTOR3& GetPosition2(){return vertex_[1];}
			D3DXVECTOR3& GetPosition3(){return vertex_[2];}
	};

	/**********************************************************
	//DxMath
	**********************************************************/
	class DxMath
	{
		public:
			static D3DXVECTOR2 Normalize(const D3DXVECTOR2 &v) 
			{
				return *D3DXVec2Normalize(&D3DXVECTOR2(),&v);
			}
			static D3DXVECTOR3 Normalize(const D3DXVECTOR3 &v) 
			{
				return *D3DXVec3Normalize(&D3DXVECTOR3(),&v);
			}
			static float DotProduct(const D3DXVECTOR2 &v1, const D3DXVECTOR2 &v2) 
			{
				return D3DXVec2Dot(&v1, &v2);
			}
			static float DotProduct(const D3DXVECTOR3 &v1, const D3DXVECTOR3 &v2)
			{
				return D3DXVec3Dot(&v1, &v2);
			}
			static float CrossProduct(const D3DXVECTOR2 &v1,const D3DXVECTOR2 &v2) 
			{
				return D3DXVec2CCW(&v1, &v2);
			}
			static D3DXVECTOR3 CrossProduct(const D3DXVECTOR3 &v1,const D3DXVECTOR3 &v2) 
			{
				return *D3DXVec3Cross(&D3DXVECTOR3() ,&v1, &v2);
			}

			//ベクトルと行列の積
			static D3DXVECTOR4 VectMatMulti(D3DXVECTOR4 v, D3DMATRIX& mat);

			//衝突判定：点－多角形
			static bool IsIntersected(D3DXVECTOR2& pos, std::vector<D3DXVECTOR2>& list);

			//衝突判定：円-円
			static bool IsIntersected(DxCircle& circle1, DxCircle& circle2);

			//衝突判定：円-直線
			static bool IsIntersected(DxCircle& circle, DxWidthLine& line);

			//衝突判定：直線-直線
			static bool IsIntersected(DxWidthLine& line1, DxWidthLine& line2);

			//衝突判定：直線：三角
			static bool IsIntersected(DxLine3D& line, std::vector<DxTriangle>& triangles, std::vector<D3DXVECTOR3>& out);
	};

	struct RECT_D
	{
		double left;
		double top;
		double right;
		double bottom;
	};

	inline RECT_D GetRectD(RECT& rect)
	{
		RECT_D res = {(double)rect.left, (double)rect.top, (double)rect.right, (double)rect.bottom};
		return res;
	}

	inline void SetRectD(RECT_D* rect, double left, double top, double right, double bottom)
	{
		rect->left = left;
		rect->top = top;
		rect->right = right;
		rect->bottom = bottom;
	}
}


#endif
