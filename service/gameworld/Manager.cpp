#include "Manager.h"

IManager::IManager(UGameWorld *world)
    : mWorld(world) {
}

IManager::~IManager() {
}

void IManager::Initial() {
}

void IManager::BeginPlay() {
}

void IManager::EndPlay() {
}

UGameWorld *IManager::GetWorld() const {
    return mWorld;
}
