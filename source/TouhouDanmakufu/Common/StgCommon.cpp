#include"StgCommon.hpp"
#include"StgSystem.hpp"

/**********************************************************
//StgMoveObject
**********************************************************/
StgMoveObject::StgMoveObject(StgStageController* stageController)
{
	posX_ = 0;
	posY_ = 0;
	framePattern_ = 0;
	stageController_ = stageController;
}
StgMoveObject::~StgMoveObject()
{

}
void StgMoveObject::_Move()
{
	if(pattern_ == NULL)return;

	if(mapPattern_.size() > 0)
	{
		std::map<int, ref_count_ptr<StgMovePattern>::unsync >::iterator itr = mapPattern_.begin();
		int frame = itr->first;
		if(frame == framePattern_)
		{
			ref_count_ptr<StgMovePattern>::unsync pattern = itr->second;
			_AttachReservedPattern(pattern);
			mapPattern_.erase(frame);
		}
	}

	pattern_->Move();
	framePattern_++;
}
void StgMoveObject::_AttachReservedPattern(ref_count_ptr<StgMovePattern>::unsync pattern)
{
	//‘¬“xŒp‘±‚È‚Ç
	if(pattern_ == NULL)
		pattern_ = new StgMovePattern_Angle(this);

	int newMoveType = pattern->GetType();
	if(newMoveType == StgMovePattern::TYPE_ANGLE)
	{
		StgMovePattern_Angle* angPattern = (StgMovePattern_Angle*)pattern.GetPointer();
		if(angPattern->GetSpeed() == StgMovePattern::NO_CHANGE)
			angPattern->SetSpeed(pattern_->GetSpeed());
		if(angPattern->GetDirectionAngle() == StgMovePattern::NO_CHANGE)
			angPattern->SetDirectionAngle(pattern_->GetDirectionAngle());
	}
	else if(newMoveType == StgMovePattern::TYPE_XY)
	{
		StgMovePattern_XY* xyPattern = (StgMovePattern_XY*)pattern.GetPointer();

		double speed = pattern_->GetSpeed();
		double angle = pattern_->GetDirectionAngle();
		double speedX = speed * cos(Math::DegreeToRadian(angle));
		double speedY = speed * sin(Math::DegreeToRadian(angle));

		if(xyPattern->GetSpeedX() == StgMovePattern::NO_CHANGE)
			xyPattern->SetSpeedX(speedX);
		if(xyPattern->GetSpeedY() == StgMovePattern::NO_CHANGE)
			xyPattern->SetSpeedY(speedY);
	}

	//’u‚«Š·‚¦
	pattern_ = pattern;
}
double StgMoveObject::GetSpeed()
{
	if(pattern_ == NULL)return 0;
	double res = pattern_->GetSpeed();
	return res;
}
void StgMoveObject::SetSpeed(double speed)
{
	if(pattern_ == NULL || pattern_->GetType() != StgMovePattern::TYPE_ANGLE)
	{
		pattern_ = new StgMovePattern_Angle(this);
	}
	StgMovePattern_Angle* pattern = (StgMovePattern_Angle*)pattern_.GetPointer();
	pattern->SetSpeed(speed);
}
double StgMoveObject::GetDirectionAngle()
{
	if(pattern_ == NULL)return 0;
	double res = pattern_->GetDirectionAngle();
	return res;
}
void StgMoveObject::SetDirectionAngle(double angle)
{
	if(pattern_ == NULL || pattern_->GetType() != StgMovePattern::TYPE_ANGLE)
	{
		pattern_ = new StgMovePattern_Angle(this);
	}
	StgMovePattern_Angle* pattern = (StgMovePattern_Angle*)pattern_.GetPointer();
	pattern->SetDirectionAngle(angle);
}
void StgMoveObject::AddPattern(int frameDelay, ref_count_ptr<StgMovePattern>::unsync pattern)
{
	if(frameDelay == 0)
		_AttachReservedPattern(pattern);
	else
	{
		int frame = frameDelay + framePattern_;
		mapPattern_[frame] = pattern;
	}
}

/**********************************************************
//StgMovePattern
**********************************************************/
//StgMovePattern
StgMovePattern::StgMovePattern(StgMoveObject* target)
{
	target_ = target;
	idShotData_ = NO_CHANGE;
	frameWork_ = 0;
	typeMove_ = TYPE_OTHER;
}
ref_count_ptr<StgMoveObject>::unsync StgMovePattern::_GetMoveObject(int id)
{
	StgStageController* controller = _GetStageController();
	ref_count_ptr<DxScriptObjectBase>::unsync base = controller->GetMainRenderObject(id);
	if(base == NULL || base->IsDeleted())return NULL;

	return ref_count_ptr<StgMoveObject>::unsync::DownCast(base);
}

//StgMovePattern_Angle
StgMovePattern_Angle::StgMovePattern_Angle(StgMoveObject* target) : StgMovePattern(target)
{
	typeMove_ = TYPE_ANGLE;
	speed_ = 0;
	angDirection_ = 0;
	acceleration_ = 0;
	maxSpeed_ = 0;
	angularVelocity_ = 0;
	idRalativeID_ = DxScript::ID_INVALID;
}
void StgMovePattern_Angle::Move()
{
	if(frameWork_ == 0)
		_Activate();
	double angle = angDirection_;

	if(acceleration_ != 0)
	{
		speed_ += acceleration_;
		if(acceleration_ > 0)
			speed_ = min(speed_, maxSpeed_);
		if(acceleration_ < 0)
			speed_ = max(speed_, maxSpeed_);
	}
	if(angularVelocity_ != 0)
	{
		angDirection_ += angularVelocity_;
	}

	double sx = speed_ * cos(Math::DegreeToRadian(angDirection_));
	double sy = speed_ * sin(Math::DegreeToRadian(angDirection_));
	double px = target_->GetPositionX() + sx;
	double py = target_->GetPositionY() + sy;

	target_->SetPositionX(px);
	target_->SetPositionY(py);

	frameWork_++;
}
void StgMovePattern_Angle::_Activate()
{
	if(idRalativeID_ != DxScript::ID_INVALID)
	{
		ref_count_ptr<StgMoveObject>::unsync obj = _GetMoveObject(idRalativeID_);
		if(obj != NULL)
		{
			double px = target_->GetPositionX();
			double py = target_->GetPositionY();
			double tx = obj->GetPositionX();
			double ty = obj->GetPositionY();
			double angle = Math::RadianToDegree(atan2(ty - py, tx - px));
			angDirection_ += angle;
		}
	}

}

//StgMovePattern_XY
StgMovePattern_XY::StgMovePattern_XY(StgMoveObject* target) : StgMovePattern(target)
{
	typeMove_ = TYPE_XY;
	speedX_ = 0;
	speedY_ = 0;
	accelerationX_ = 0;
	accelerationY_ = 0;
	maxSpeedX_ = INT_MAX;
	maxSpeedY_ = INT_MAX;
}
void StgMovePattern_XY::Move()
{
	if(frameWork_ == 0)
		_Activate();

	if(accelerationX_ != 0)
	{
		speedX_ += accelerationX_;
		if(accelerationX_ > 0)
			speedX_ = min(speedX_, maxSpeedX_);
		if(accelerationX_ < 0)
			speedX_ = max(speedX_, maxSpeedX_);
	}
	if(accelerationY_ != 0)
	{
		speedY_ += accelerationY_;
		if(accelerationY_ > 0)
			speedY_ = min(speedY_, maxSpeedY_);
		if(accelerationY_ < 0)
			speedY_ = max(speedY_, maxSpeedY_);
	}

	double px = target_->GetPositionX() + speedX_;
	double py = target_->GetPositionY() + speedY_;

	target_->SetPositionX(px);
	target_->SetPositionY(py);

	frameWork_++;
}
double StgMovePattern_XY::GetSpeed()
{
	double res = pow(speedX_ * speedX_ + speedY_ * speedY_, 0.5);
	return res;
}
double StgMovePattern_XY::GetDirectionAngle()
{
	double res = Math::RadianToDegree(atan2(speedY_, speedX_));
	return res;
}

//StgMovePattern_Line
StgMovePattern_Line::StgMovePattern_Line(StgMoveObject* target) : StgMovePattern(target)
{
	typeMove_ = TYPE_NONE;
	speed_ = 0;
	angDirection_ = 0;
	weight_ = 0;
	maxSpeed_ = 0;
	frameStop_ = 0;
}
void StgMovePattern_Line::Move()
{
	if(typeLine_ == TYPE_SPEED || typeLine_ == TYPE_FRAME)
	{
		double sx = speed_ * cos(Math::DegreeToRadian(angDirection_));
		double sy = speed_ * sin(Math::DegreeToRadian(angDirection_));
		double px = target_->GetPositionX() + sx;
		double py = target_->GetPositionY() + sy;

		target_->SetPositionX(px);
		target_->SetPositionY(py);
		frameStop_--;
		if(frameStop_ <= 0)
		{
			typeLine_ = TYPE_NONE;
			speed_ = 0;
		}
	}
	else if(typeLine_ == TYPE_WEIGHT)
	{
		double nx = target_->GetPositionX();
		double ny = target_->GetPositionY();
		double dist = pow(pow(toX_ - nx, 2) + pow(toY_ - ny, 2), 0.5);
		if(dist < 1)
		{
			typeLine_ = TYPE_NONE;
			speed_ = 0;
		}
		else
		{
			speed_ = dist / weight_;
			if(speed_ > maxSpeed_)
				speed_ = maxSpeed_;
			double px = target_->GetPositionX() + speed_*cos(Math::DegreeToRadian(angDirection_));
			double py = target_->GetPositionY() + speed_*sin(Math::DegreeToRadian(angDirection_));
			target_->SetPositionX(px);
			target_->SetPositionY(py);
		}

	}
}
void StgMovePattern_Line::SetAtSpeed(double tx, double ty, double speed)
{
	typeLine_ = TYPE_SPEED;
	toX_ = tx;
	toY_ = ty;
	double nx = target_->GetPositionX();
	double ny = target_->GetPositionY();
	double dist = pow(pow(tx - nx, 2) + pow(ty - ny, 2), 0.5);
	speed_ = speed;
	angDirection_ = Math::RadianToDegree(atan2(ty - ny, tx - nx));
	frameStop_ = dist / speed;
}
void StgMovePattern_Line::SetAtFrame(double tx, double ty, double frame)
{
	typeLine_ = TYPE_FRAME;
	toX_ = tx;
	toY_ = ty;
	double nx = target_->GetPositionX();
	double ny = target_->GetPositionY();
	double dist = pow(pow(tx - nx, 2) + pow(ty - ny, 2), 0.5);
	speed_ = dist / frame;
	angDirection_ = Math::RadianToDegree(atan2(ty - ny, tx - nx));
	frameStop_ = frame;
}
void StgMovePattern_Line::SetAtWait(double tx, double ty, double weight, double maxSpeed)
{
	typeLine_ = TYPE_WEIGHT;
	toX_ = tx;
	toY_ = ty;
	weight_ = weight;
	maxSpeed_ = maxSpeed;
	double nx = target_->GetPositionX();
	double ny = target_->GetPositionY();
	double dist = pow(pow(tx - nx, 2) + pow(ty - ny, 2), 0.5);
	speed_ = maxSpeed_;
	angDirection_ = Math::RadianToDegree(atan2(ty - ny, tx - nx));
}
