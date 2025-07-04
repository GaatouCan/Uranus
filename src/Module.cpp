#include "Module.h"


IModule::IModule(UServer *server)
    : mServer(server),
      mState(EModuleState::CREATED) {
}

void IModule::Initial() {
    mState = EModuleState::INITIALIZED;
}

void IModule::Start() {
    mState = EModuleState::RUNNING;
}

void IModule::Stop() {
    mState = EModuleState::STOPPED;
}

UServer *IModule::GetServer() const {
    return mServer;
}

EModuleState IModule::GetState() const {
    return mState;
}
