#include "PackageCodec.h"


IPackageCodecBase::IPackageCodecBase(ATcpSocket &socket)
    : socket_(socket) {
}

ATcpSocket &IPackageCodecBase::GetSocket() const {
    return socket_;
}
