#include "addressbook.pb.h"
#include "pbmsg.hpp"
#include <cstdio>
#include <iostream>

using namespace std;

int main(int argc, char* argv[])
{
    printf("通过已有PB对象创建，之后进行赋值");
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

        printf("通过PB对象创建对象 >>> \n");

        auto pb = pbmsg_t::create(&person);
        result = pb->get_bin();
        printf("-----------------\n  result length = %zu\n  ", result.length());
        for (std::string::size_type i = 0; i < result.length(); i++) {
            printf("%02x ", result[i]);
        }
        printf("\n-----------------\n");

        name = "kermit";
        pb->set_attr("name", name);
        printf("  set new name= %s\n", name.c_str());
        email = "38272702@qq.com";
        pb->set_attr("email", email);
        printf("  set new email= %s\n", email.c_str());
        id = 990;
        pb->set_attr("id", id);
        printf("  set new id = %d\n", id);

        result = pb->get_bin();
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

        printf("-----------------\n");
     
        delete pb;
    }


    return 0;
}