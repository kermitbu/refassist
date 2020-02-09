#include "pbmsg.hpp"
#include <cstdio>
using namespace std;

int main(int argc, char* argv[])
{
    // 在一个程序中创建来自两个proto文件的结构，静态成员不影响。
    {
        printf("通过反射创建对象 >>> \n");
        kermit::refassist_t* pbmsga = kermit::refassist_t::create("../example/protos/testmsg.proto", "TestMessageB");

        int64_t int64data = 234;

        pbmsga->add_attr("int64repeatedData", int64data);
        printf("  add syd = %ld\n", int64data);

        pbmsga->set_attr("int64repeatedData", 1, int64data);
        printf("  set syd = %ld\n", int64data);

        std::string result = pbmsga->get_bin();
        printf("-----------------\n  result length = %zu\n  ", result.length());
        for (std::string::size_type i = 0; i < result.length(); i++) {
            printf("%02x ", result[i]);
        }
        printf("\n-----------------\n");

        delete pbmsga;
    }

    return 0;
}