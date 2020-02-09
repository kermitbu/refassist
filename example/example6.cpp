#include "pbmsg.hpp"
#include <cstdio>
using namespace std;

int main(int argc, char* argv[])
{
    // 在一个程序中创建来自两个proto文件的结构，静态成员不影响。
    {
        printf("通过反射创建对象 >>> \n");
        kermit::refassist_t* pbmsga = kermit::refassist_t::create("../example/protos/testmsg.proto", "TestMessageA");
        double price = 1238.33;
        pbmsga->set_attr("price", price);
        printf("  set syd = %lf\n", price);

        int32_t id = 123;
        pbmsga->set_attr("id", id);
        printf("  set id = %d\n", id);
        
        std::string name = "kermit";
        pbmsga->set_attr("name", name);
        printf("  set name = %s\n", name.c_str());

        std::string result = pbmsga->get_bin();
        printf("-----------------\n  result length = %zu\n  ", result.length());
        for (std::string::size_type i = 0; i < result.length(); i++) {
            printf("%02x ", result[i]);
        }
        printf("\n-----------------\n");

        delete pbmsga;
    }

  {
     printf("在新生命周期创建创建另外一个文件中定义的结构对象 >>> \n");

       auto pbmsga2 = kermit::refassist_t::create("../example/addressbook.proto", "Person");

        std::string name = "kermit";
        pbmsga2->set_attr("name", name);
        printf("  set name= %s\n", name.c_str());
        
        std::string email = "kermit.bu@qq.com";
        pbmsga2->set_attr("email", email);
        printf("  set email= %s\n", email.c_str());
        int32_t id = 123;
        pbmsga2->set_attr("id", id);
        printf("  set id = %d\n", id);

        auto result = pbmsga2->get_bin();
        printf("-----------------\n  result length = %zu\n  ", result.length());
        for (std::string::size_type i = 0; i < result.length(); i++) {
            printf("%02x ", result[i]);
        }
        printf("\n-----------------\n");

        delete pbmsga2;
    }

    // {        
    //     kermit::refassist_t* addressbook = kermit::refassist_t::create(msg, true);
    //     int syd = 0;
    //     addressbook->get_attr("syd", syd);
    //     printf("  get syd = %d\n", syd);
    //     delete addressbook;
    // }

    return 0;
}