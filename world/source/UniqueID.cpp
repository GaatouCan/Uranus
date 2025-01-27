#include "../include/UniqueID.h"
#include "../include/utils.h"

#include <spdlog/fmt/fmt.h>
#include <absl/random/random.h>

std::string FUniqueID::ToString() const {
    return fmt::format("{}-{}", time, random);
}

FUniqueID & FUniqueID::FromString(const std::string &str) {
    const std::vector<std::string> res = utils::SplitString(str, '-');
    time = std::stoll(res[0]);
    random = std::stoull(res[1]);
    return *this;
}

FUniqueID FUniqueID::RandomGenerate() {
    static absl::BitGen gen;
    const uint64_t number = absl::Uniform(gen, 100000, 999999);
    return {
        utils::UnixTime(),
        number
    };
}

bool FUniqueID::operator<(const FUniqueID &other) const {
    if (time < other.time) {
        return true;
    }
    if (time == other.time) {
        return random < other.random;
    }
    return false;
}

bool FUniqueID::operator==(const FUniqueID &other) const {
    return time == other.time && random == other.random;
}

bool FUniqueID::operator!=(const FUniqueID &other) const {
    return !(*this == other);
}
