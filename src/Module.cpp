#include "Module.h"

IModuleBase::IModuleBase()
    : Server(nullptr),
      State(EModuleState::CREATED) {
}

void IModuleBase::SetUpServer(UServer *server) {
    Server = server;
}

void IModuleBase::Initial() {
    State = EModuleState::INITIALIZED;
}

void IModuleBase::Start() {
    State = EModuleState::RUNNING;
}

void IModuleBase::Stop() {
    State = EModuleState::STOPPED;
}

UServer *IModuleBase::GetServer() const {
    return Server;
}

EModuleState IModuleBase::GetState() const {
    return State;
}
