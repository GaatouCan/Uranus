#include "PackageCodec.h"


IPackageCodecBase::IPackageCodecBase(ATcpSocket &socket)
    : mSocket(socket) {
}

ATcpSocket &IPackageCodecBase::GetSocket() const {
    return mSocket;
}
