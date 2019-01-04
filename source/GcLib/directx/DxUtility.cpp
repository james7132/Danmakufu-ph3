#include"DxUtility.hpp"

using namespace gstd;
using namespace directx;

/**********************************************************
//ColorAccess
**********************************************************/
int ColorAccess::GetColorA(D3DCOLOR& color)
{
	return gstd::BitAccess::GetByte(color, BIT_ALPHA);
}
D3DCOLOR& ColorAccess::SetColorA(D3DCOLOR& color, int alpha)
{
	if(alpha > 255)alpha = 255;
	if(alpha < 0)alpha = 0;
	return gstd::BitAccess::SetByte(color, BIT_ALPHA, (unsigned char)alpha);
}
int ColorAccess::GetColorR(D3DCOLOR color)
{
	return gstd::BitAccess::GetByte(color, BIT_RED);
}
D3DCOLOR& ColorAccess::SetColorR(D3DCOLOR& color, int red)
{
	if(red > 255)red = 255;
	if(red < 0)red = 0;
	return gstd::BitAccess::SetByte(color, BIT_RED, (unsigned char)red);
}
int ColorAccess::GetColorG(D3DCOLOR& color)
{
	return gstd::BitAccess::GetByte(color, BIT_GREEN);
}
D3DCOLOR& ColorAccess::SetColorG(D3DCOLOR& color, int green)
{
	if(green > 255)green = 255;
	if(green < 0)green = 0;
	return gstd::BitAccess::SetByte(color, BIT_GREEN, (unsigned char)green);
}
int ColorAccess::GetColorB(D3DCOLOR& color)
{
	return gstd::BitAccess::GetByte(color, BIT_BLUE);
}
D3DCOLOR& ColorAccess::SetColorB(D3DCOLOR& color, int blue)
{
	if(blue > 255)blue = 255;
	if(blue < 0)blue = 0;
	return gstd::BitAccess::SetByte(color, BIT_BLUE, (unsigned char)blue);
}
D3DCOLORVALUE ColorAccess::SetColor(D3DCOLORVALUE value, D3DCOLOR color)
{
	float a = (float)GetColorA(color)/255.0f;
	float r = (float)GetColorR(color)/255.0f;
	float g = (float)GetColorG(color)/255.0f;
	float b = (float)GetColorB(color)/255.0f;
	value.r *= r;value.g *=g;value.b *= b;value.a*=a;
	return value;
}
D3DMATERIAL9 ColorAccess::SetColor(D3DMATERIAL9 mat, D3DCOLOR color)
{
	float a = (float)GetColorA(color)/255.0f;
	float r = (float)GetColorR(color)/255.0f;
	float g = (float)GetColorG(color)/255.0f;
	float b = (float)GetColorB(color)/255.0f;
	mat.Diffuse.r *= r;mat.Diffuse.g *=g;mat.Diffuse.b *= b;mat.Diffuse.a*=a;
	mat.Specular.r *= r;mat.Specular.g *=g;mat.Specular.b *= b;mat.Specular.a*=a;
	mat.Ambient.r *= r;mat.Ambient.g *=g;mat.Ambient.b *= b;mat.Ambient.a*=a;
	mat.Emissive.r *= r;mat.Emissive.g *= g;mat.Emissive.b *=b;mat.Emissive.a*=a;
	return mat;
}
D3DCOLOR& ColorAccess::ApplyAlpha(D3DCOLOR& color, double alpha)
{
	color = SetColorA(color, GetColorA(color) * alpha);
	color = SetColorR(color, GetColorR(color) * alpha);
	color = SetColorG(color, GetColorG(color) * alpha);
	color = SetColorB(color, GetColorB(color) * alpha);
	return color;
}
D3DCOLOR& ColorAccess::SetColorHSV(D3DCOLOR& color, int hue, int saturation, int value)
{
    float f;
    int i, p, q, t;
    
    i = (int)floor(hue / 60.0f) % 6;
    f = (float)(hue / 60.0f) - (float)floor(hue / 60.0f);
    p = (int)gstd::Math::Round(value * (1.0f - (saturation / 255.0f)));
    q = (int)gstd::Math::Round(value * (1.0f - (saturation / 255.0f) * f));
    t = (int)gstd::Math::Round(value * (1.0f - (saturation / 255.0f) * (1.0f - f)));
    
	int red = 0;
	int green = 0;
	int blue = 0;
    switch(i){
        case 0 : red = value;	green = t;		blue = p; break;
        case 1 : red = q;		green = value;	blue = p; break;
        case 2 : red = p;		green = value;	blue = t; break;
        case 3 : red = p;		green = q;		blue = value; break;
        case 4 : red = t;		green = p;		blue = value; break;
        case 5 : red = value;	green = p;		blue = q; break;
    }

	SetColorR(color, red);
	SetColorG(color, green);
	SetColorB(color, blue);
    return color;
}

/**********************************************************
//DxMath
**********************************************************/
D3DXVECTOR4 DxMath::VectMatMulti(D3DXVECTOR4 v, D3DMATRIX& mat)
{
	float x,y,z;

	x=v.x; 
	y=v.y; 
	z=v.z;

	v.x=(x*mat._11)+(y*mat._21)+(z*mat._31)+mat._41;
	v.y=(x*mat._12)+(y*mat._22)+(z*mat._32)+mat._42;
	v.z=(x*mat._13)+(y*mat._23)+(z*mat._33)+mat._43;
	v.w=(x*mat._14)+(y*mat._24)+(z*mat._34)+mat._44;

	return v;
}
bool DxMath::IsIntersected(D3DXVECTOR2& pos, std::vector<D3DXVECTOR2>& list)
{
	if(list.size() <= 2)return false;

	bool res = true;
	for(int iPos = 0 ; iPos < list.size() ; iPos++)
	{
		int p1 = iPos;
		int p2 = iPos + 1;
		if(p2 >= list.size())p2 %= list.size();

		double cross_x = ((double)list[p2].x - (double)list[p1].x) * ((double)pos.y - (double)list[p1].y);
		double corss_y = ((double)list[p2].y - (double)list[p1].y) * ((double)pos.x - (double)list[p1].x);
		if(cross_x - corss_y < 0)res = false;
	}
	return res;
}
bool DxMath::IsIntersected(DxCircle& circle1, DxCircle& circle2)
{
	double r1 = pow(circle1.GetX() - circle2.GetX(), 2) + pow(circle1.GetY() - circle2.GetY(), 2);
	double r2 = pow(circle1.GetR() + circle2.GetR(), 2);
	return r1 <= r2;
}
bool DxMath::IsIntersected(DxCircle& circle, DxWidthLine& line)
{
	//先端もしくは終端が円内にあるかを調べる
	{
		double radius = circle.GetR();
		double dist1 = pow(pow(circle.GetX()-line.GetX1(),2) + pow(circle.GetY()-line.GetY1(),2), 0.5);
		double dist2 = pow(pow(circle.GetX()-line.GetX2(),2) + pow(circle.GetY()-line.GetY2(),2), 0.5);
		if(radius >= dist1 || radius >= dist2)
			return true;
	}

	//線分内に円があるかを調べる
	{
		double lx1 = line.GetX2() - line.GetX1();
		double ly1 = line.GetY2() - line.GetY1();
		double cx1 = circle.GetX() - line.GetX1();
		double cy1 = circle.GetY() - line.GetY1();
		double inner1 = lx1 * cx1 + ly1 * cy1;

		double lx2 = line.GetX1() - line.GetX2();
		double ly2 = line.GetY1() - line.GetY2();
		double cx2 = circle.GetX() - line.GetX2();
		double cy2 = circle.GetY() - line.GetY2();
		double inner2 = lx2 * cx2 + ly2 * cy2;

		if(inner1 < 0 || inner2 < 0)
			return false;
	}

	double ux1 = line.GetX2() - line.GetX1();
	double uy1 = line.GetY2() - line.GetY1();
	double px = circle.GetX() - line.GetX1();
	double py = circle.GetY() - line.GetY1();
	double u = pow(pow((double)ux1,(double)2)+pow((double)uy1,(double)2),0.5);//直線の距離
	double ux2 = ux1 / u;//直線の単位ベクトルx
	double uy2 = uy1 / u;//直線の単位ベクトルz
	double d = px*ux2 + py*uy2;//直線の単位ベクトルと始点から点までベクトルの内積
	double qx = d * ux2;
	double qy = d * uy2;
	double rx = px - qx;//点から直線までの最短距離ベクトルx
	double ry = py - qy;//点から直線までの最短距離ベクトルz
	double e = pow(pow((double)rx,(double)2)+pow((double)ry,(double)2),0.5);//直線のと点の距離
	double r = line.GetWidth() + circle.GetR();

	bool res = e < r;
	return res;
}
bool DxMath::IsIntersected(DxWidthLine& line1, DxWidthLine& line2)
{

	return false;
}
bool DxMath::IsIntersected(DxLine3D& line, std::vector<DxTriangle>& triangles, std::vector<D3DXVECTOR3>& out)
{
	out.clear();

	for(int iTri = 0; iTri < triangles.size() ; iTri++)
	{
		DxTriangle& tri = triangles[iTri];
		D3DXPLANE plane;//3角形の面
		D3DXPlaneFromPoints( &plane, &tri.GetPosition(0), &tri.GetPosition(1), &tri.GetPosition(2));

		D3DXVECTOR3 vOut;// 面と視線の交点の座標
		if( D3DXPlaneIntersectLine( &vOut, &plane, &line.GetPosition(0), &line.GetPosition(1) ))
		{
			// 内外判定
			D3DXVECTOR3 vN[3];
			D3DXVECTOR3 vv1, vv2, vv3;
			vv1 = tri.GetPosition(0) - vOut;
			vv2 = tri.GetPosition(1) - vOut;
			vv3 = tri.GetPosition(2) - vOut;
			D3DXVec3Cross( &vN[0], &vv1, &vv3 );
			D3DXVec3Cross( &vN[1], &vv2, &vv1 );
			D3DXVec3Cross( &vN[2], &vv3, &vv2 );
			if( D3DXVec3Dot( &vN[0], &vN[1] ) < 0 || D3DXVec3Dot( &vN[0], &vN[2] ) < 0 )
				continue;
			else
			{// 内側(3角形に接触)
				out.push_back(vOut);
			}
		}
	}//for(int i=0;i<tris.size();i++)

	bool res = out.size() > 0;
	return res;
}
