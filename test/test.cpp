#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "catch.hpp"
#include <string>
#include "refassist.hpp"
#include "testmsg.pb.h"

TEST_CASE( "split_file_name") {
    std::string parent, basename;
    
    SECTION( "linux 绝对路径" ) {
        std::string fullpath = "/mnt/d/CppWork/refassist/abc.proto";
        kermit::refassist_t::split_file_name(fullpath, parent, basename);
        REQUIRE( basename == "abc.proto" );
        REQUIRE( parent == "/mnt/d/CppWork/refassist/" );
    }

    SECTION( "linux 相对路径" ) {
        std::string fullpath = "./../../abc.proto";
        kermit::refassist_t::split_file_name(fullpath, parent, basename);
        REQUIRE( basename == "abc.proto" );
        REQUIRE( parent == "./../../" );
    }

    SECTION( "linux 不包含路径" ) {
        std::string fullpath = "abc.proto";
        kermit::refassist_t::split_file_name(fullpath, parent, basename);
        REQUIRE( basename == "abc.proto" );
        REQUIRE( parent == "" );
    }

    SECTION( "linux 不包含文件名" ) {
        std::string fullpath = "../../";
        kermit::refassist_t::split_file_name(fullpath, parent, basename);
        REQUIRE( basename == "" );
        REQUIRE( parent == "../../" );
    }
}


TEST_CASE( "设置值之后，序列化的二进制正确", "[set]") {
    kermit::refassist_t* pbmsga = kermit::refassist_t::create("../example/protos/testmsg.proto", "TestMessageB");
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


TEST_CASE( "设置值之后，获取数值正确", "[get]") {
    kermit::refassist_t* pbmsga = kermit::refassist_t::create("../example/protos/testmsg.proto", "TestMessageB");
    TestMessageB src_msg;
    double doubleData = 100.32, get_doubleData = 0.0;
    pbmsga->set_attr("doubleData", doubleData);
    pbmsga->get_attr("doubleData",get_doubleData);
    REQUIRE(doubleData == Approx(get_doubleData));

    float floatData = 200.01f, get_floatData=0.0f;
    pbmsga->set_attr("floatData", floatData);
    pbmsga->get_attr("floatData",get_floatData);
    REQUIRE(floatData == Approx(get_floatData));

    int64_t int64Data = 100, get_int64Data = 0;
    pbmsga->set_attr("int64Data", int64Data);
    pbmsga->get_attr("int64Data",get_int64Data);
    REQUIRE(int64Data == get_int64Data);

    uint64_t uint64Data = 200, get_uint64Data = 0;
    pbmsga->set_attr("uint64Data", uint64Data);
    pbmsga->get_attr("uint64Data",get_uint64Data);
    REQUIRE(uint64Data == get_uint64Data);

    int32_t int32Data = 300, get_int32Data = 0;
    pbmsga->set_attr("int32Data", int32Data);
    pbmsga->get_attr("int32Data",get_int32Data);
    REQUIRE(int32Data == get_int32Data);

    uint64_t fixed64Data = 400, get_fixed64Data = 0;
    pbmsga->set_attr("fixed64Data", fixed64Data);
    pbmsga->get_attr("fixed64Data",get_fixed64Data);
    REQUIRE(fixed64Data == get_fixed64Data);

    uint32_t fixed32Data = 500, get_fixed32Data = 0;
    pbmsga->set_attr("fixed32Data", fixed32Data);
    pbmsga->get_attr("fixed32Data",get_fixed32Data);
    REQUIRE(fixed32Data == get_fixed32Data);

    bool boolData = true, get_boolData = false;
    pbmsga->set_attr("boolData", boolData);
    pbmsga->get_attr("boolData",get_boolData);
    REQUIRE(boolData == get_boolData);

    std::string stringData = "abc.com", get_stringData = "";
    pbmsga->set_attr("stringData", stringData);
    pbmsga->get_attr("stringData",get_stringData);
    REQUIRE(stringData == get_stringData);

    std::string bytesData = "byteabc.com", get_bytesData = "";
    pbmsga->set_attr("bytesData", bytesData);
    pbmsga->get_attr("bytesData",get_bytesData);
    REQUIRE(bytesData == get_bytesData);

    TestMessageB_MT  enumData = TestMessageB_MT::TestMessageB_MT_REQ, get_enumData = TestMessageB_MT::TestMessageB_MT_REQ;
    pbmsga->set_attr("enumData", enumData);
    pbmsga->get_attr("enumData",get_enumData);
    REQUIRE(enumData == get_enumData);

    uint32_t uint32Data = 3332, get_uint32Data = 0;
    pbmsga->set_attr("uint32Data", uint32Data);
    pbmsga->get_attr("uint32Data",get_uint32Data);
    REQUIRE(uint32Data == get_uint32Data);

    int32_t sfixed32Data = 32, get_sfixed32Data = 0;
    pbmsga->set_attr("sfixed32Data", sfixed32Data);
    pbmsga->get_attr("sfixed32Data",get_sfixed32Data);
    REQUIRE(sfixed32Data == get_sfixed32Data);

    int64_t sfixed64Data = 732, get_sfixed64Data = 0;
    pbmsga->set_attr("sfixed64Data", sfixed64Data);
    pbmsga->get_attr("sfixed64Data",get_sfixed64Data);
    REQUIRE(sfixed64Data == get_sfixed64Data);

    int32_t sint32Data = 563, get_sint32Data = 0;
    pbmsga->set_attr("sint32Data", sint32Data);
    pbmsga->get_attr("sint32Data",get_sint32Data);
    REQUIRE(sint32Data == get_sint32Data);

    int64_t sint64Data = 16783, get_sint64Data = 0;
    pbmsga->set_attr("sint64Data", sint64Data);
    pbmsga->get_attr("sint64Data",get_sint64Data);
    REQUIRE(sint64Data == get_sint64Data);
}
