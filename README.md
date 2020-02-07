# pbmsg

#### 介绍

pbmsg是一个对protobuf反射用法的一个封装，使反射更易用。


#### 获取示例代码和编译

```bash
# 获取代码
git clone https://gitee.com/kermitbuxk/pbmsg.git
git submodule init
git submodule update

# 编译protobuf
cd protobuf
sh autogen.sh && ./configure && make
cd -

# 编译示例代码
mkdir build && cd build
cmake ../build && make
```

#### 使用说明

```cpp
// 包含pbmsg.hpp头文件
#include "pbmsg.hpp"

// 根据proto文件创建一个对象
auto pbmsg = pbmsg_t::create("../example/addressbook.proto", "Person");

// 设置属性值
pbmsg->set_attr("name", std::string("kermit"));
pbmsg->set_attr("id", 1203);

// 获取序列化后的二进制流
auto result = pbmsg->get_bin();

// 获取独立生命周期的的Message的指针
// 如果传入false则是获取内部的Message，不需要外部释放
auto msg = pbmsg->get_msg(true);

// 获取属性值
std::string name;
pbmsg->get_attr("name", name);
int32_t id = 0;
pbmsg->get_attr("id", id);

// 清理属性值
pbmsg->clear_attr("id", id);

// 释放分配的空间
delete msg;
delete pbmsg;
```

#### 参与贡献

1.  Fork 本仓库
2.  新建 Feat_xxx 分支
3.  提交代码
4.  新建 Pull Request
