#include "manager.h"

IManager::IManager(UGameWorld *world)
    : world_(world) {
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
    return world_;
}
