## Uranus框架

Hello，这里是Uranus C++网络框架！

### 关于Service

Service是提供具体业务的服务类，通过继承IService基类提供服务，编译为动态库文件由主程序加载。

Service由IContext托管，每个Service创建之前，都会创建并分配一个Context，Context管理数据包池和调度队列。等Context所需资源创建完毕后，再创建Services实例并托管给Context，当Service收到别的地方发送给它的数据包时，实际是发送给了托管它的Context，Context将数据包放入调度队列中，让这些数据包按照顺序被Service消费。

各个Service之间通过数据包通信，IContext在调度数据包时，会调用

```c++
IService::OnPackage(const std::shared_ptr<IPackage> &pkg)
```

通知服务处理数据。

向别的服务发送数据时，调用

```
void IService::PostPackage(const std::shared_ptr<IPackage> &pkg) const;
void IService::PostPackage(const std::string &name, const std::shared_ptr<IPackage> &pkg) const;
```

两个方法其中之一发送数据包，其中第一个方法，将目标服务的ServceID通过`IPackage::SetTarget(uint32_t id)`写入Package头部数据里面。

虽然IService提供了GetServer()和GetModule()的接口，但还是不推荐使用，业务上有任何需要调用Server中某个模块的功能的话，还是改写IService提供一层封装比较好。

IService有一个特殊子类IPlayerA gent。IPlayerAgent作为玩家连接的抽象，管理玩家身上所有可以独立的数据。只需实现一次，编译的动态库存放路径为`./agent/agent.dll`。由Gateway模块管理，当有新玩家链接到来时便创建一个agent。

其它IService的子类则放在`./core`和`./extend`目录下。

其中core下的跟随Server初始化而初始化，Server Start而Start，全局唯一实例，生命周期同Server，即核心服务在服务器运行周期内始终唯一直不变。

Extend下的服务由核心服务或命令调用而启动，可同一服务创建多个实例（服务名必须不同，保证唯一），可通过命令或其内部判断到生命末期而关闭服务。