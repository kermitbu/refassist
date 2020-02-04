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
    }

    /////////////////////////////////////////////////////////////////////////////////
    {
        printf("通过反射创建对象 >>> \n");

        auto reqdata = pbmsg_t::create("../example/addressbook.proto", "AddressBook.req_data");
        char errmsg[256];

        reqdata->set_attr("tid", tid, errmsg);
        printf("  set data.tid = %d\n", tid);
        reqdata->set_attr("name", name);
        printf("  set data.name = %s\n", name.c_str());

        auto addressbook = pbmsg_t::create("../example/addressbook.proto", "AddressBook");
        addressbook->set_attr("syd", syd);
        printf("  set syd = %d\n", syd);

        addressbook->set_attr("data", reqdata->to_pb(), errmsg);
        std::string result = addressbook->get_bin();

        printf("-----------------\n  result length = %zu\n  ", result.length());
        for (std::string::size_type i = 0; i < result.length(); i++) {
            printf("%02x ", result[i]);
        }
        printf("\n-----------------\n");
#if 0
        std::string name_get;
        addressbook->get_attr("data.name", name_get);
        printf("  get data.name= %s\n", name_get.c_str());

        int32_t tid_get;
        addressbook->get_attr("data.tid", tid_get);
        printf("  get data.tid= %d\n", tid_get);

        int32_t syd_get;
        addressbook->get_attr("syd_get", syd_get);
        printf("  get syd_get= %d\n", syd_get);
        #endif
    }
    return 0;
}