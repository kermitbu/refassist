#include "addressbook.pb.h"
#include "pbmsg.hpp"
#include <cstdio>
using namespace std;

int main(int argc, char* argv[])
{
    printf("创建内置的结构体，并把内置结构设置到外部结构中\n");
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
    }

    /////////////////////////////////////////////////////////////////////////////////
    {
        printf("通过反射创建对象 >>> \n");

        pbmsg_t* reqdata = pbmsg_t::create("../example/addressbook.proto", "AddressBook.req_data");
  
        reqdata->set_attr("tid", tid);
        printf("  set data.tid = %d\n", tid);
        reqdata->set_attr("name", name);
        printf("  set data.name = %s\n", name.c_str());

        pbmsg_t* addressbook = pbmsg_t::create("../example/addressbook.proto", "AddressBook");
        addressbook->set_attr("syd", syd);
        printf("  set syd = %d\n", syd);

        addressbook->set_attr("data", reqdata);
        std::string result = addressbook->get_bin();

        printf("-----------------\n  result length = %zu\n  ", result.length());
        for (std::string::size_type i = 0; i < result.length(); i++) {
            printf("%02x ", result[i]);
        }
        printf("\n-----------------\n");
                 
        // reqdata 已经被添加到addressbook中了，不需要再次进行释放;
        delete addressbook;

    }
    return 0;
}