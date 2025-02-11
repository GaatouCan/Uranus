#include "../include/UniqueID.h"
#include "../include/utils.h"

#include <spdlog/fmt/fmt.h>
#include <random>

std::string UniqueID::ToString() const {
    return fmt::format("{}-{}", time, random);
}

UniqueID & UniqueID::FromString(const std::string &str) {
    const std::vector<std::string> res = utils::SplitString(str, '-');

    if (res.size() < 2)
        return *this;

    time = std::stoll(res[0]);
    random = std::stoll(res[1]);

    return *this;
}

UniqueID UniqueID::RandomGenerate() {
    std::random_device random_device;
    std::mt19937 generator(random_device());
    std::uniform_int_distribution distribution(100000, 999999);

    const int64_t number = distribution(generator);

    return {
        utils::UnixTime(),
        number
    };
}

bool UniqueID::operator<(const UniqueID &other) const {
    if (time < other.time) {
        return true;
    }
    if (time == other.time) {
        return random < other.random;
    }
    return false;
}

bool UniqueID::operator==(const UniqueID &other) const {
    return time == other.time && random == other.random;
}

bool UniqueID::operator!=(const UniqueID &other) const {
    return !(*this == other);
}
