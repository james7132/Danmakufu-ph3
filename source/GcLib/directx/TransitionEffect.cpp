#include"TransitionEffect.hpp"

using namespace gstd;
using namespace directx;

/**********************************************************
//TransitionEffect
**********************************************************/
TransitionEffect::TransitionEffect()
{

}
TransitionEffect::~TransitionEffect()
{

}

/**********************************************************
//TransitionEffect_FadeOut
**********************************************************/
void TransitionEffect_FadeOut::Work()
{
	if(sprite_ == NULL)return;
	alpha_ -= diffAlpha_;
	alpha_ = max(alpha_, 0);
	sprite_->SetAlpha((int)alpha_);
}
void TransitionEffect_FadeOut::Render()
{
	if(sprite_ == NULL)return;

	DirectGraphics* graphics = DirectGraphics::GetBase();
	graphics->SetBlendMode(DirectGraphics::MODE_BLEND_ALPHA);
	graphics->SetZBufferEnable(false);
	graphics->SetZWriteEnalbe(false);
	sprite_->Render();
}
bool TransitionEffect_FadeOut::IsEnd()
{
	bool res = (alpha_ <= 0);
	return res;
}
void TransitionEffect_FadeOut::Initialize(int frame, gstd::ref_count_ptr<Texture> texture)
{
	diffAlpha_ = 255.0 / frame;
	alpha_ = 255.0;
	DirectGraphics* graphics = DirectGraphics::GetBase();
	int width = graphics->GetScreenWidth();
	int height = graphics->GetScreenHeight();
	RECT_D rect ={0., 0., (double)width, (double)height};

	sprite_ = new Sprite2D();
	sprite_->SetTexture(texture);
	sprite_->SetVertex(rect, rect, D3DCOLOR_ARGB((int)alpha_, 255, 255, 255));
}

/**********************************************************
//TransitionEffectTask
**********************************************************/
TransitionEffectTask::TransitionEffectTask()
{
}
TransitionEffectTask::~TransitionEffectTask()
{
}
void TransitionEffectTask::SetTransition(gstd::ref_count_ptr<TransitionEffect> effect)
{
	effect_ = effect;
}
void TransitionEffectTask::Work()
{
	if(effect_ != NULL)
	{
		effect_->Work();
	}
}
void TransitionEffectTask::Render()
{
	if(effect_ != NULL)effect_->Render();
}
