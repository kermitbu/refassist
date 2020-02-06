#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "catch.hpp"
#include <string>
#include "pbmsg.hpp"
#include "testmsg.pb.h"

TEST_CASE( "split_file_name") {
    std::string parent, basename;
    
    SECTION( "linux 绝对路径" ) {
        std::string fullpath = "/mnt/d/CppWork/pbmsg/abc.proto";
        pbmsg_t::split_file_name(fullpath, parent, basename);
        REQUIRE( basename == "abc.proto" );
        REQUIRE( parent == "/mnt/d/CppWork/pbmsg/" );
    }

    SECTION( "linux 相对路径" ) {
        std::string fullpath = "./../../abc.proto";
        pbmsg_t::split_file_name(fullpath, parent, basename);
        REQUIRE( basename == "abc.proto" );
        REQUIRE( parent == "./../../" );
    }

    SECTION( "linux 不包含路径" ) {
        std::string fullpath = "abc.proto";
        pbmsg_t::split_file_name(fullpath, parent, basename);
        REQUIRE( basename == "abc.proto" );
        REQUIRE( parent == "" );
    }

    SECTION( "linux 不包含文件名" ) {
        std::string fullpath = "../../";
        pbmsg_t::split_file_name(fullpath, parent, basename);
        REQUIRE( basename == "" );
        REQUIRE( parent == "../../" );
    }
}


TEST_CASE( "设置值之后，序列化的二进制正确", "[set]") {
    pbmsg_t* pbmsga = pbmsg_t::create("../example/protos/testmsg.proto", "TestMessageB");
    TestMessageB src_msg;
    double doubleData = 100.32;
    pbmsga->set_attr("doubleData", doubleData);
    src_msg.set_doubledata(doubleData);

        float floatData = 200.01f;
    pbmsga->set_attr("floatData", floatData);
    src_msg.set_floatdata(floatData);

    int64_t int64Data = 100;
    pbmsga->set_attr("int64Data", int64Data);
    src_msg.set_int64data(int64Data);

    uint64_t uint64Data = 200;
    pbmsga->set_attr("uint64Data", uint64Data);
    src_msg.set_uint64data(uint64Data);

    int32_t int32Data = 300;
    pbmsga->set_attr("int32Data", int32Data);
    src_msg.set_int32data(int32Data);

    uint64_t fixed64Data = 400;
    pbmsga->set_attr("fixed64Data", fixed64Data);
    src_msg.set_fixed64data(fixed64Data);

    uint32_t fixed32Data = 500;
    pbmsga->set_attr("fixed32Data", fixed32Data);
    src_msg.set_fixed32data(fixed32Data);

    bool boolData = true;
    pbmsga->set_attr("boolData", boolData);
    src_msg.set_booldata(boolData);

    std::string stringData = "abc.com";
    pbmsga->set_attr("stringData", stringData);
    src_msg.set_stringdata(stringData);

    std::string bytesData = "byteabc.com";
    pbmsga->set_attr("bytesData", bytesData);
    src_msg.set_bytesdata(bytesData);

    TestMessageB_MT  enumData = TestMessageB_MT::TestMessageB_MT_REQ;
    pbmsga->set_attr("enumData", enumData);
    src_msg.set_enumdata(enumData);

    uint32_t uint32Data = 3332;
    pbmsga->set_attr("uint32Data", uint32Data);
    src_msg.set_uint32data(uint32Data);

    int32_t sfixed32Data = 32;
    pbmsga->set_attr("sfixed32Data", sfixed32Data);
    src_msg.set_sfixed32data(sfixed32Data);

    int64_t sfixed64Data = 732;
    pbmsga->set_attr("sfixed64Data", sfixed64Data);
    src_msg.set_sfixed64data(sfixed64Data);

    int32_t sint32Data = 563;
    pbmsga->set_attr("sint32Data", sint32Data);
    src_msg.set_sint32data(sint32Data);

    int64_t sint64Data = 16783;
    pbmsga->set_attr("sint64Data", sint64Data);
    src_msg.set_sint64data(sint64Data);
   
    std::string result;
    src_msg.SerializeToString(&result);

    REQUIRE(result == pbmsga->get_bin());

}
