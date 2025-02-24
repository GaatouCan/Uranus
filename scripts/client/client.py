import socket
import time

from package import Package
from protobuf.gen import login_pb2
from protobuf.gen import appearance_pb2

SERVER_HOST = "localhost"
SERVER_PORT = 8080

client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
client.connect((SERVER_HOST, SERVER_PORT))

# Login
req = login_pb2.ClientLoginRequest()

req.player_id = 1020144
req.role = login_pb2.UserRole.PLAYER
req.token = 'nnnnnn'
req.override = False

pkg = Package()
pkg.set_id(1001)
pkg.set_data(req.SerializeToString())

client.send(pkg.encode())

# Login Result
r_pkg = Package()
r_pkg.decodeWithSocket(client)

res = login_pb2.LoginResponse()
res.ParseFromString(r_pkg.get_raw_data())

print(res)

time.sleep(2)

req = appearance_pb2.AppearanceRequest()

req.operate_type = appearance_pb2.OperateType.ACTIVE_AVATAR
req.index = 1
req.parameter = 1

pkg = Package()
pkg.set_id(2001)
pkg.set_data(req.SerializeToString())

client.send(pkg.encode())

r_pkg = Package()
r_pkg.decodeWithSocket(client)

res = appearance_pb2.AppearanceResponse()
res.ParseFromString(r_pkg.get_raw_data())

print(res)

client.close()