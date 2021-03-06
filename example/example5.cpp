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
    google::protobuf::Message* msg;

    /////////////////////////////////////////////////////////////////////////////////
    {
        printf("通过反射创建对象 >>> \n");

        kermit::refassist_t* reqdata = kermit::refassist_t::create("../example/addressbook.proto", "AddressBook.req_data");
  
        reqdata->set_attr("tid", tid);
        printf("  set data.tid = %d\n", tid);
        reqdata->set_attr("name", name);
        printf("  set data.name = %s\n", name.c_str());

        kermit::refassist_t* addressbook = kermit::refassist_t::create("../example/addressbook.proto", "AddressBook");
        addressbook->set_attr("syd", syd);
        printf("  set syd = %d\n", syd);

        addressbook->set_attr("data", reqdata->get_msg());
        std::string result = addressbook->get_bin();

        printf("-----------------\n  result length = %zu\n  ", result.length());
        for (std::string::size_type i = 0; i < result.length(); i++) {
            printf("%02x ", result[i]);
        }
        printf("\n-----------------\n");

        msg = addressbook->get_msg(true);

        delete addressbook;
    }

    {        
        kermit::refassist_t* addressbook = kermit::refassist_t::create(msg, true);
        int syd = 0;
        addressbook->get_attr("syd", syd);
        printf("  get syd = %d\n", syd);
        syd = 990;
        addressbook->set_attr("syd", syd);
        printf("  set syd = %d\n", syd);

        int syd_get = 0;
        addressbook->get_attr("syd", syd_get);
        printf("  get syd = %d\n", syd_get);

        delete addressbook;
    }

    return 0;
}