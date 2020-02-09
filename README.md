# refassist

#### Description

refassist is a wrapper around protobuf reflection usage, making reflection easier to use.


#### Get the example code
```bash
git clone https://github.com/kermitbu/refassist.git
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
#include "refassist.hpp"

// alloc a refassist
auto refassist = kermit::refassist_t::create("../example/addressbook.proto", "Person");

// set attr
refassist->set_attr("name", std::string("kermit"));
refassist->set_attr("id", 1203);

// get pb binary
auto result = refassist->get_bin();

// get pb message
auto msg = refassist->get_msg(true);

// get attr
std::string name;
refassist->get_attr("name", name);
int32_t id = 0;
refassist->get_attr("id", id);

// clear attr
refassist->clear_attr("id", id);

// dealloc buffer
delete msg;
delete refassist;
```

#### Contribution

1.  Fork the repository
2.  Create Feat_xxx branch
3.  Commit your code
4.  Create Pull Request
