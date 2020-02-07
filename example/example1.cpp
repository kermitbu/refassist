#include "pbmsg.hpp"
#include "testmsg.pb.h"
#include <cstdio>
#include <iostream>

using namespace std;

int main(int argc, char* argv[])
{
    printf("通过proto文件创建对象，之后进行赋值\n");
    {
        printf("通过反射创建对象 >>> \n");

        pbmsg_t* pbmsga = pbmsg_t::create("../example/protos/testmsg.proto", "TestMessageB");

        double doubleData = 100.32;
        pbmsga->set_attr("doubleData", doubleData);
        printf("  set doubleData= %lf\n", doubleData);

         float floatData = 200.01f;
        pbmsga->set_attr("floatData", floatData);
        printf("  set floatData= %f\n", floatData);

        int64_t int64Data = 100;
        pbmsga->set_attr("int64Data", int64Data);
        printf("  set int64Data= %ld\n", int64Data);

        uint64_t uint64Data = 200;
        pbmsga->set_attr("uint64Data", uint64Data);
        printf("  set uint64Data= %lu\n", uint64Data);

        int32_t int32Data = 300;
        pbmsga->set_attr("int32Data", int32Data);
        printf("  set int32Data= %u\n", int32Data);

        uint64_t fixed64Data = 400;
        pbmsga->set_attr("fixed64Data", fixed64Data);
        printf("  set fixed64Data= %lu\n", fixed64Data);

        uint32_t fixed32Data = 500;
        pbmsga->set_attr("fixed32Data", fixed32Data);
        printf("  set fixed32Data= %u\n", fixed32Data);

        bool boolData = true;
        pbmsga->set_attr("boolData", boolData);
        printf("  set boolData= %d\n", boolData);

        std::string stringData = "abc.com";
        pbmsga->set_attr("stringData", stringData);
        printf("  set boolData= %s\n", stringData.c_str());

        std::string bytesData = "byteabc.com";
        pbmsga->set_attr("bytesData", bytesData);
        printf("  set bytesData= %s\n", bytesData.c_str());

        int32_t enumData = 2;
        pbmsga->set_attr("enumData", enumData);
        printf("  set enumData= %d\n", enumData);
 
        uint32_t uint32Data = 3332;
        pbmsga->set_attr("uint32Data", uint32Data);
        printf("  set uint32Data= %u\n", uint32Data);

        int32_t sfixed32Data = 32;
        pbmsga->set_attr("sfixed32Data", sfixed32Data);
        printf("  set sfixed32Data= %d\n", sfixed32Data);

        int64_t sfixed64Data = 732;
        pbmsga->set_attr("sfixed64Data", sfixed64Data);
        printf("  set sfixed64Data= %ld\n", sfixed64Data);

        int32_t sint32Data = 563;
        pbmsga->set_attr("sint32Data", sint32Data);
        printf("  set sint32Data= %d\n", sint32Data);

        int64_t sint64Data = 16783;
        pbmsga->set_attr("sint64Data", sint64Data);
        printf("  set sint64Data= %ld\n", sint64Data);

        auto result = pbmsga->get_bin();
        printf("-----------------\n  result length = %zu\n  ", result.length());
        for (std::string::size_type i = 0; i < result.length(); i++) {
            printf("%02x ", result[i]);
        }
        printf("\n-----------------\n");

        delete pbmsga;
    }
    ///////////////////////////////////////////////////////////////////
    
    printf("通过PB源码创建的对象来创建对象，之后进行赋值, 再次获取出数值\n");
    {
        TestMessageB msg;
        pbmsg_t* pbmsga = pbmsg_t::create(&msg);

        double doubleData = 100.32;
        pbmsga->set_attr("doubleData", doubleData);
        printf("  set doubleData= %lf\n", doubleData);

        int64_t int64Data = 100;
        pbmsga->set_attr("int64Data", int64Data);
        printf("  set int64Data= %ld\n", int64Data);
        
        double get_doubleData = 0.0;
        pbmsga->get_attr("doubleData", get_doubleData);
        printf("  get doubleData= %lf\n", get_doubleData);

        int64_t get_int64Data = 0;
        pbmsga->get_attr("doubleData", get_int64Data);
        printf("  get doubleData= %ld\n", get_int64Data);

        delete pbmsga;
    }



    return 0;
}