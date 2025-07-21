#include "Module.h"

IModuleBase::IModuleBase()
    : server_(nullptr),
      state_(EModuleState::CREATED) {
}

void IModuleBase::SetUpServer(UServer *server) {
    server_ = server;
}

void IModuleBase::Initial() {
    state_ = EModuleState::INITIALIZED;
}

void IModuleBase::Start() {
    state_ = EModuleState::RUNNING;
}

void IModuleBase::Stop() {
    state_ = EModuleState::STOPPED;
}

UServer *IModuleBase::GetServer() const {
    return server_;
}

EModuleState IModuleBase::GetState() const {
    return state_;
}
