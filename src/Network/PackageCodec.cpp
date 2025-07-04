#include "PackageCodec.h"


IPackageCodec::IPackageCodec(ATcpSocket &socket)
    : mSocket(socket) {
}

ATcpSocket &IPackageCodec::GetSocket() const {
    return mSocket;
}
