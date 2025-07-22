#include "Event.h"

int UDefaultEvent::GetEventType() const {
    return 1001;
}

void UDefaultEvent::SetEventParam(const nlohmann::json &data) {
    mData = data;
}

const nlohmann::json &UDefaultEvent::GetEventParam() const {
    return mData;
}
