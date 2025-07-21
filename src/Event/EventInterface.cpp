#include "EventInterface.h"

int UDefaultEvent::GetEventType() const {
    return 1001;
}

void UDefaultEvent::SetEventParam(const nlohmann::json &data) {
    data_ = data;
}

const nlohmann::json &UDefaultEvent::GetEventParam() const {
    return data_;
}
