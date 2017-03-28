#include "PPositionTracker.hpp"

PPositionTracker::PPositionTracker()
{
    setAgent(Agent::PositionTracker);
    mEvent.posTracker_p.type = PEvent::PositionTracker_Parameters::PositionTracker_Event::PosHasChanged;
    mEvent.posTracker_p.pos.phi=0.f;
    mEvent.posTracker_p.pos.x=0.f;
    mEvent.posTracker_p.pos.y=0.f;

}

PPositionTracker::~PPositionTracker()
{
    //dtor
}
void PPositionTracker::handleCommand(const PCommand& command)
{
    switch(command.posTracker_p.type)
    {
        case PCommand::PositionTracker_Parameters::PositionTracker_Command::UpdatePosition:
        {
            updatePos(command);
            break;
        }
        case PCommand::PositionTracker_Parameters::PositionTracker_Command::ResetPosition:
        {
            mEvent.posTracker_p.pos.x=0.f;
            mEvent.posTracker_p.pos.y=0.f;
            break;
        }
        case PCommand::PositionTracker_Parameters::PositionTracker_Command::UpdateAngle:
        {
            mEvent.posTracker_p.pos.phi = command.posTracker_p.angleGyro * 2*M_PI/360.f;
            break;
        }
    }
}

void PPositionTracker::updatePos(const PCommand &command)
{
    const PCommand::PositionTracker_Parameters::TraveledDist &dist = command.posTracker_p.traveledDist;
    PEvent::PositionTracker_Parameters::Position &pos = mEvent.posTracker_p.pos;

    if(dist.leftDist != dist.rightDist)
    {
        mAngle=(dist.leftDist-dist.rightDist)/mDistInterWheel;
        mRadius=(dist.leftDist+dist.rightDist)/(2.f*mAngle);

        mDxp=mRadius*(1-cos(mAngle));
        mDyp=mRadius*sin(mAngle);

        pos.phi-=mAngle;
        mDx=mDxp*cos(pos.phi)-mDyp*sin(pos.phi);

        if(sin(pos.phi)!=0)
            mDy=-(mDx*cos(pos.phi)-mDxp)/sin(pos.phi);
        else if(pos.phi < M_PI)
            mDy=mDyp;
        else
            mDy=-mDyp;

    }
    else
    {
        mDx = -sin(mAngle)*dist.leftDist;
        mDy = cos(mAngle)*dist.leftDist;
    }
    pos.x+=mDx;
    pos.y+=mDy;
    pushEvent(mEvent);
}

void PPositionTracker::preRun()
{

}

void PPositionTracker::run()
{

}

void PPositionTracker::postRun()
{

}
