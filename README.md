# pbmsg

#### 介绍

pbmsg是一个对protobuf反射用法的一个封装，使反射更易用。


#### 编译说明

```bash
# 编译protobuf
cd protobuf
sh autogen.sh && ./configure && make
cd -

mkdir tmp && cd tmp
cmake ../build && make
```

#### 使用说明

```
// 根据proto文件创建一个对象
auto pbmsg = pbmsg_t::create("../example/addressbook.proto", "Person");

// 设置属性值
pbmsg->set_attr("name", std::string("kermit"));
pbmsg->set_attr("id", 1203);

// 获取序列化后的二进制流
auto result = pbmsg->get_bin();

// 获取属性值
std::string name;
pbmsg->get_attr("name", name);
int32_t id = 0;
pbmsg->get_attr("id", id);
```

#### 参与贡献

1.  Fork 本仓库
2.  新建 Feat_xxx 分支
3.  提交代码
4.  新建 Pull Request
