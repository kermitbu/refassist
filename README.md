# pbmsg

#### Description

pbmsg is a wrapper around protobuf reflection usage, making reflection easier to use.


#### Get the example code
```bash
git clone https://gitee.com/kermitbuxk/pbmsg.git
git submodule init
git submodule update
```

#### Compile example code
```bash
cd protobuf
sh autogen.sh && ./configure && make
cd -

mkdir build && cd build
cmake ../build && make
```

#### Instructions

```cpp
#include "pbmsg.hpp"

// alloc a pbmsg
auto pbmsg = pbmsg_t::create("../example/addressbook.proto", "Person");

// set attr
pbmsg->set_attr("name", std::string("kermit"));
pbmsg->set_attr("id", 1203);

// get pb binary
auto result = pbmsg->get_bin();

// get pb message
auto msg = pbmsg->get_msg(true);

// get attr
std::string name;
pbmsg->get_attr("name", name);
int32_t id = 0;
pbmsg->get_attr("id", id);

// clear attr
pbmsg->clear_attr("id", id);

// dealloc buffer
delete msg;
delete pbmsg;
```

#### Contribution

1.  Fork the repository
2.  Create Feat_xxx branch
3.  Commit your code
4.  Create Pull Request
