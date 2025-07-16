#include "Module.h"


IModuleBase::IModuleBase(UServer *server)
    : mServer(server),
      mState(EModuleState::CREATED) {
}

void IModuleBase::Initial() {
    mState = EModuleState::INITIALIZED;
}

void IModuleBase::Start() {
    mState = EModuleState::RUNNING;
}

void IModuleBase::Stop() {
    mState = EModuleState::STOPPED;
}

UServer *IModuleBase::GetServer() const {
    return mServer;
}

EModuleState IModuleBase::GetState() const {
    return mState;
}
