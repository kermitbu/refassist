#include <new>

#include "google/protobuf/compiler/importer.h"
#include "google/protobuf/dynamic_message.h"
#include "google/protobuf/message.h"

class pbmsg_t final {
public:
    static pbmsg_t* create(const google::protobuf::Message* msg_ptr_)
    {
		return nullptr;
    }

    static pbmsg_t* create(const std::string& file, const std::string& msg_name)
    {
        pbmsg_t* pbmsg = new (std::nothrow) pbmsg_t();
        if (nullptr == pbmsg) {
            return nullptr;
        }

        if (pbmsg->import(file, msg_name) != 0) {
            delete pbmsg;
            return nullptr;
        }

        return pbmsg;
    }

    int import(const std::string& file, const std::string& msg_name)
    {

        std::string parent, basename;
        split_file_name(file, parent, basename);
        google::protobuf::compiler::DiskSourceTree source_tree;
        source_tree.MapPath("", "./");
        source_tree.MapPath("", parent);
        importer_ptr_ = new (std::nothrow) google::protobuf::compiler::Importer(&source_tree, NULL);
        if (nullptr == importer_ptr_) {
            return -1;
        }

        file_desc_ptr_ = importer_ptr_->Import(basename);

        desc_ptr_ = file_desc_ptr_->pool()->FindMessageTypeByName(msg_name);

        factory_ptr_ = new (std::nothrow) google::protobuf::DynamicMessageFactory(file_desc_ptr_->pool());
        if (nullptr == importer_ptr_) {
            return -2;
        }

        msg_ptr_ = factory_ptr_->GetPrototype(desc_ptr_)->New();

        reflection_ptr_ = msg_ptr_->GetReflection();

        return 0;
    }


    int set_attr(const std::string& name, const int32_t& value, std::string errmsg = "")
    {
        const google::protobuf::FieldDescriptor* field = desc_ptr_->FindFieldByName(name);

        if (field->is_repeated()) {
            errmsg = name + " is a repeated field!";
            return -1;
        }

        reflection_ptr_->SetInt32(msg_ptr_, field, value);

        return 0;
    }

    int get_attr(const std::string& name, int32_t& value, std::string errmsg = "")
    {
        const google::protobuf::FieldDescriptor* field = desc_ptr_->FindFieldByName(name);
        value = reflection_ptr_->GetInt32(*msg_ptr_, field);
        return 0;
    }

    int set_attr(const std::string& name, const std::string& value, std::string errmsg = "")
    {
        const google::protobuf::FieldDescriptor* field = desc_ptr_->FindFieldByName(name);

        if (field->is_repeated()) {
            errmsg = name + " is't a repeated field!";
            return -1;
        }

        reflection_ptr_->SetString(msg_ptr_, field, value);

        return 0;
    }

    int set_attr(const std::string& name, pbmsg_t* value, std::string errmsg = "")
    {
        google::protobuf::Message* submsg = value->get_msg();
        const google::protobuf::FieldDescriptor* field = desc_ptr_->FindFieldByName(name);

        if (field->is_repeated()) {
            errmsg = name + " is't a repeated field!";
            return -1;
        }

        reflection_ptr_->SetAllocatedMessage(msg_ptr_, submsg, field);

        return 0;
    }

    int get_attr(const std::string& name, std::string& value, std::string errmsg = "")
    {
        const google::protobuf::FieldDescriptor* field = desc_ptr_->FindFieldByName(name);
        value = reflection_ptr_->GetString(*msg_ptr_, field);
        return 0;
    }

    google::protobuf::Message* get_msg()
    {
        return msg_ptr_;
    }

    std::string get_bin()
    {
        std::string str;
        msg_ptr_->SerializeToString(&str);

        return str;
    }

    int set_bin(std::string& pbdata)
    {
        return msg_ptr_->ParseFromString(pbdata) ? 0 : -1;
    }


    pbmsg_t()
    {
    }

    ~pbmsg_t()
    {
        if (importer_ptr_ != nullptr) {
            delete importer_ptr_;
        }

        if (msg_ptr_ != nullptr) {
            delete msg_ptr_;
        }
    }
private:
	static void split_file_name(const std::string& fullpath, std::string& parent, std::string& basename)
    {
        std::string file = fullpath;
        bool is_replaced = false;
        for (std::string::size_type pos = 0; (pos = file.find("/", pos)) != std::string::npos; pos++) {
            file.replace(pos, 1, "\\");
            is_replaced = true;
        }
        basename = file.substr(file.find_last_of("\\") + 1);

        if (is_replaced) {
            parent = fullpath.substr(0, fullpath.find_last_of("/") + 1);
        } else {
            parent = file.substr(0, file.find_last_of("\\") + 1);
        }
    }

private:
    google::protobuf::compiler::Importer* importer_ptr_{ nullptr };
    const google::protobuf::FileDescriptor* file_desc_ptr_{ nullptr };
    google::protobuf::MessageFactory* factory_ptr_{ nullptr };
    const google::protobuf::Descriptor* desc_ptr_{ nullptr };
    const google::protobuf::Reflection* reflection_ptr_{ nullptr };
    google::protobuf::Message* msg_ptr_{ nullptr };
};
