#include "LoginAuth.h"
#include "Server.h"
#include "Package.h"
#include "Gateway/Gateway.h"
#include "Network/Network.h"
#include "Network/Connection.h"

#include <spdlog/spdlog.h>


ULoginAuth::ULoginAuth(UServer *server)
    : IModuleBase(server) {
}

ULoginAuth::~ULoginAuth() {
    Stop();
}

void ULoginAuth::Initial() {
    if (mState != EModuleState::CREATED)
        return;

    GetServer()->GetServerHandler()->InitLoginAuth(this);

    mState = EModuleState::INITIALIZED;
}

void ULoginAuth::Start() {
    if (mState != EModuleState::INITIALIZED)
        return;

    mState = EModuleState::RUNNING;
}

void ULoginAuth::Stop() {
    if (mState == EModuleState::STOPPED)
        return;

    mState = EModuleState::STOPPED;
}

bool ULoginAuth::VerifyAddress(const asio::ip::tcp::endpoint &endpoint) {
    // TODO
    return true;
}

void ULoginAuth::OnPlayerLogin(const int64_t cid, const std::shared_ptr<IPackageBase> &pkg) {
    if (mState != EModuleState::RUNNING)
        return; {
        std::unique_lock lock(mMutex);
        if (const auto iter = mLoginMap.find(cid); iter != mLoginMap.end()) {
            if (std::chrono::system_clock::now() - iter->second < std::chrono::seconds(1))
                return;
        }

        mLoginMap[cid] = std::chrono::system_clock::now();
    }

    const auto [token, pid] = mHandler->ParseLoginRequest(pkg);
    if (token.empty() || pid == 0) {
        SPDLOG_WARN("{:<20} - fd[{}] Parse Login Request Failed.", __FUNCTION__, cid);
        return;
    }

    auto *network = GetServer()->GetModule<UNetwork>();
    const auto *gateway = GetServer()->GetModule<UGateway>();

    if (network == nullptr || gateway == nullptr) {
        return;
    }

    if (const auto exist = gateway->GetConnectionID(pid); exist != 0 && gateway->GetState() == EModuleState::RUNNING) {
        std::string addr = "UNKNOWN";
        if (const auto conn = network->FindConnection(exist)) {
            addr = conn->RemoteAddress().to_string();
        }

        const auto response = network->BuildPackage();

        mHandler->OnRepeatLogin(pid, addr, response);

        response->SetSource(SERVER_SOURCE_ID);
        response->SetTarget(CLIENT_TARGET_ID);

        network->OnLoginFailure(cid, response);
    }

    // TODO: Real Login Verify Logic

    // FIXME: Delete This When Finish Real Login Logic
    OnLoginSuccess(cid, pid);
}

void ULoginAuth::OnLoginSuccess(const int64_t cid, const int64_t pid) { {
        std::unique_lock lock(mMutex);
        mLoginMap.erase(cid);
    }

    const auto *network = GetServer()->GetModule<UNetwork>();
    auto *gateway = GetServer()->GetModule<UGateway>();

    if (network == nullptr || gateway == nullptr) {
        return;
    }

    // Send Login Success Response To Client
    const auto response = network->BuildPackage();

    mHandler->OnLoginSuccess(pid, response);

    response->SetSource(SERVER_SOURCE_ID);
    response->SetTarget(CLIENT_TARGET_ID);

    network->OnLoginSuccess(cid, pid, response);

    // Create Player Agent In Gateway
    gateway->OnPlayerLogin(pid, cid);
}
