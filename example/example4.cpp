#include "addressbook.pb.h"
#include "pbmsg.hpp"
#include <cstdio>
using namespace std;

int main(int argc, char* argv[])
{
    printf(">>>对比创建嵌套类>>> \n");
    int tid = 112;
    std::string name = "buxk";
    int syd = 100;
    {
        printf("通过源码创建对象>>> \n");
        AddressBook addressbook;
        addressbook.mutable_data()->set_tid(tid);
        printf("  set data.tid = %d\n", tid);
        addressbook.mutable_data()->set_name(name);
        printf("  set data.name = %s\n", name.c_str());
        addressbook.set_syd(syd);
        printf("  set syd = %d\n", syd);

        std::string result;
        addressbook.SerializeToString(&result);

        printf("-----------------\n  result length = %zu\n  ", result.length());
        for (std::string::size_type i = 0; i < result.length(); i++) {
            printf("%02x ", result[i]);
        }
        printf("\n\n");

        printf("通过反射创建对象 >>> \n");

        pbmsg_t* new_addressbook = pbmsg_t::create(&addressbook);
        int32_t syd_get = 440;
        new_addressbook->set_attr("syd", syd_get);
        printf("  get syd = %d\n", syd_get);

        pbmsg_t* data;
        new_addressbook->get_attr("data", &data);

    }
    return 0;
}