# pbmsg

#### 介绍

pbmsg是一个对protobuf反射用法的一个封装，使反射更易用。


#### 编译说明

```bash
mkdir tmp && cd tmp
cmake ../build && make
```

#### 使用说明

```
auto pbmsg = pbmsg_t::create("../example/addressbook.proto", "Person");
pbmsg->set_attr("name", std::string("abc"));
pbmsg->set_attr("id", 234);
auto result = pbmsg->get_bin();
```

#### 参与贡献

1.  Fork 本仓库
2.  新建 Feat_xxx 分支
3.  提交代码
4.  新建 Pull Request
