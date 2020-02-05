#include "addressbook.pb.h"
#include "pbmsg.hpp"
#include <cstdio>
#include <iostream>

using namespace std;

int main(int argc, char* argv[])
{
    printf(">>>对比创建基础类>>> \n");
    int id = 233;
    std::string name = "buxk";
    std::string email = "kermit.bu@qq.com";
    {
        printf("通过源码创建对象>>> \n");
        Person person;
        person.set_name(name);
        printf("  set name= %s\n", name.c_str());
        person.set_email(email);
        printf("  set email= %s\n", email.c_str());
        person.set_id(id);
        printf("  set id = %d\n", id);

        std::string result;
        person.SerializeToString(&result);

        printf("-----------------\n  result length = %zu\n  ", result.length());
        for (std::string::size_type i = 0; i < result.length(); i++) {
            printf("%02x ", result[i]);
        }
        printf("\n\n");
    }

    /////////////////////////////////////////////////////////////////////////////////
    {
        printf("通过反射创建对象 >>> \n");

        auto pb = pbmsg_t::create("../example/addressbook.proto", "Person");

        pb->set_attr("name", name);
        printf("  set name= %s\n", name.c_str());
        pb->set_attr("email", email);
        printf("  set email= %s\n", email.c_str());
        pb->set_attr("id", id);
        printf("  set id = %d\n", id);

        auto result = pb->get_bin();
        printf("-----------------\n  result length = %zu\n  ", result.length());
        for (std::string::size_type i = 0; i < result.length(); i++) {
            printf("%02x ", result[i]);
        }
        printf("\n-----------------\n");

        std::string name_get;
        pb->get_attr("name", name_get);
        printf("  get name= %s\n", name_get.c_str());

        std::string email_get;
        pb->get_attr("email", email_get);
        printf("  get email= %s\n", email_get.c_str());

        int32_t id_get;
        pb->get_attr("id", id_get);
        printf("  get id= %d\n", id_get);
        
        ///////////////////////////////////////////
        auto newpb = pbmsg_t::create("../example/addressbook.proto", "Person");
        newpb->set_bin(result);

        std::string new_pb_name_get;
        pb->get_attr("name", new_pb_name_get);
        printf("  newpb get name= %s\n", new_pb_name_get.c_str());

        std::string new_pb_email_get;
        pb->get_attr("email", new_pb_email_get);
        printf("  newpb get email= %s\n", new_pb_email_get.c_str());

        int32_t new_pb_id_get;
        pb->get_attr("id", new_pb_id_get);
        printf("  newpb get id= %d\n", new_pb_id_get);

    }


    return 0;
}