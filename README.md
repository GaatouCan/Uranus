## Uranus框架

Hello，这里是Uranus C++网络框架！

### 关于Service

service是提供具体业务的服务类，通过继承ICoreService和IExtendService提供服务，编译为动态库文件由主程序加载。

各个Service之间通过数据包通信，Service不会也不能直接调用Server里面的任何函数，只通过IContext提交数据包，IContext接收到发送给该Service的数据包时会调用IService::OnPackage(IPackage *pkg)方法。虽然IService提供了GetServer()和GetModule()的接口，但还是不推荐使用，业务上有任何需要调用Server中某个模块的功能的话，还是改写IService提供一层封装比较好。总之，IContext扮演一个负责IService和UServer通信的中介者角色。

ICoreService作为最主要的核心服务父类之外，IService有另外2个分支，IPlayerAgent和IExtendService。IPlayerAgent作为玩家连接的抽象，管理玩家身上所有可以独立的数据。只需实现一次，路径为`./agent/agent.dll`。由Gateway模块管理，当有新玩家链接到来时便创建一个agent。

ICoreService则表现为最典型的服务，全局唯一，通过扫描`./service`路径下所有dll，随UServer启动而启动加载。其动态库句柄由自身对应的Context维护，加载前先加载句柄，释放后也释放句柄。

IExtendSerivce则相反，可有多个实例，手动加载。动态库句柄由Service模块单独管理并维护。UServiceModule启动时会加载所有`./service/extend/`下的动态库的句柄，但不会创建实例，在UServiceModule完全启动后，可使用命令从维护的句柄表中创建服务实例，服务在Initial时可读取创建参数，允许在服务Initial之后才决定唯一的服务名称，当然其服务id和ICoreService一样，必须全局唯一。