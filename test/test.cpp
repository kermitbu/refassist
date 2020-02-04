#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "catch.hpp"
#include <string>
#include "pbmsg.hpp"

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


TEST_CASE( "set_attr") {

}
