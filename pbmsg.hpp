#include <new>

#include "google/protobuf/compiler/importer.h"
#include "google/protobuf/dynamic_message.h"
#include "google/protobuf/message.h"

class file_error_collector_t : public google::protobuf::compiler::MultiFileErrorCollector {
public:
    std::string text_;

    void AddError(const std::string& filename, int line, int column, const std::string& message) override
    {
        text_ = filename + ":" + std::to_string(line) + ":" + std::to_string(column) + ":" + message + "\n";
    }
};

class pbmsg_t final {
public:
    static pbmsg_t* create(google::protobuf::Message* msg, bool trusted=false)
    {
        if(nullptr == msg) {
            return nullptr;
        }

        pbmsg_t* pbmsg = new (std::nothrow) pbmsg_t();
        if (nullptr == pbmsg) {
            return nullptr;
        }
        pbmsg->trusted_ = trusted;
        if (pbmsg->trusted_) {
            pbmsg->msg_ptr_ = msg;
        } else {
            pbmsg->msg_ptr_ = msg->New();
            pbmsg->msg_ptr_->CopyFrom(*msg);
        }

        pbmsg->desc_ptr_ = pbmsg->msg_ptr_->GetDescriptor();
        pbmsg->reflection_ptr_ = pbmsg->msg_ptr_->GetReflection();

        return pbmsg;
    }

    static pbmsg_t* create(const std::string& file, const std::string& msgtype)
    {
        pbmsg_t* pbmsg = new (std::nothrow) pbmsg_t();
        if (nullptr == pbmsg) {
            return nullptr;
        }

        if (pbmsg->import(file, msgtype) != 0) {
            delete pbmsg;
            return nullptr;
        }

        return pbmsg;
    }

    int import(const std::string& file, const std::string& msgtype, std::string* errmsg = nullptr)
    {
        file_error_collector_t error_collector;
        std::string parent, basename;
        split_file_name(file, parent, basename);
        google::protobuf::compiler::DiskSourceTree source_tree;
        source_tree.MapPath("", "./");
        source_tree.MapPath("", parent);
        static google::protobuf::compiler::Importer importer(&source_tree, &error_collector);
        file_desc_ptr_ = importer.Import(basename);
        if (nullptr == file_desc_ptr_) {
            return -2;
        }
        if (!error_collector.text_.empty()) {
            if (errmsg) {
                *errmsg = error_collector.text_;
            }
            return -3;
        }

        static google::protobuf::DynamicMessageFactory factory(file_desc_ptr_->pool());

        desc_ptr_ = file_desc_ptr_->pool()->FindMessageTypeByName(msgtype);
        if (nullptr == desc_ptr_) {
            if (errmsg) {
                *errmsg = "alloc importer failed!" + msgtype;
            }
            return -5;
        }

        msg_ptr_ = factory.GetPrototype(desc_ptr_)->New();

        reflection_ptr_ = msg_ptr_->GetReflection();

        return 0;
    }

    enum class attr_type {
        DOUBLE = 1,
        FLOAT = 2,
        INT64 = 3,
        UINT64 = 4,
        INT32 = 5,
        FIXED64 = 6,
        FIXED32 = 7,
        BOOL = 8,
        STRING = 9,
        GROUP = 10,
        MESSAGE = 11,
        BYTES = 12,
        UINT32 = 13,
        ENUM = 14,
        SFIXED32 = 15,
        SFIXED64 = 16,
        SINT32 = 17,
        SINT64 = 18,
        MAX_TYPE = 18,
    };

    int set_attr(const std::string& name, const int32_t& value, attr_type type = attr_type::INT32, std::string* errmsg = nullptr)
    {
        const google::protobuf::FieldDescriptor* field = desc_ptr_->FindFieldByName(name);

        if (field->is_repeated()) {
            if (errmsg) {
                *errmsg = name + " is a repeated field!";
            }
            return -1;
        }

        if (type == attr_type::ENUM) {
            reflection_ptr_->SetEnumValue(msg_ptr_, field, value);
        } else {
            reflection_ptr_->SetInt32(msg_ptr_, field, value);
        }
        return 0;
    }

    int get_attr(const std::string& name, int32_t& value, attr_type type = attr_type::INT32, std::string* errmsg = nullptr)
    {
        const google::protobuf::FieldDescriptor* field = desc_ptr_->FindFieldByName(name);
        
        if (type == attr_type::ENUM) {
            value = reflection_ptr_->GetEnumValue(*msg_ptr_, field);
        } else {
            value = reflection_ptr_->GetInt32(*msg_ptr_, field);
        }
        return 0;
    }

    int set_attr(const std::string& name, const uint32_t& value, std::string* errmsg = nullptr)
    {
        const google::protobuf::FieldDescriptor* field = desc_ptr_->FindFieldByName(name);

        if (field->is_repeated()) {
            if (errmsg) {
                *errmsg = name + " is a repeated field!";
            }
            return -1;
        }

        reflection_ptr_->SetUInt32(msg_ptr_, field, value);

        return 0;
    }

    int get_attr(const std::string& name, uint32_t& value, std::string* errmsg = nullptr)
    {
        const google::protobuf::FieldDescriptor* field = desc_ptr_->FindFieldByName(name);
        value = reflection_ptr_->GetUInt32(*msg_ptr_, field);
        return 0;
    }

    int set_attr(const std::string& name, const int64_t& value, std::string* errmsg = nullptr)
    {
        const google::protobuf::FieldDescriptor* field = desc_ptr_->FindFieldByName(name);

        if (field->is_repeated()) {
            if (errmsg) {
                *errmsg = name + " is a repeated field!";
            }
            return -1;
        }

        reflection_ptr_->SetInt64(msg_ptr_, field, value);

        return 0;
    }

    int get_attr(const std::string& name, int64_t& value, std::string* errmsg = nullptr)
    {
        const google::protobuf::FieldDescriptor* field = desc_ptr_->FindFieldByName(name);
        value = reflection_ptr_->GetInt64(*msg_ptr_, field);
        return 0;
    }

    int set_attr(const std::string& name, const uint64_t& value, std::string* errmsg = nullptr)
    {
        const google::protobuf::FieldDescriptor* field = desc_ptr_->FindFieldByName(name);

        if (field->is_repeated()) {
            if (errmsg) {
                *errmsg = name + " is a repeated field!";
            }
            return -1;
        }

        reflection_ptr_->SetUInt64(msg_ptr_, field, value);

        return 0;
    }

    int get_attr(const std::string& name, uint64_t& value, std::string* errmsg = nullptr)
    {
        const google::protobuf::FieldDescriptor* field = desc_ptr_->FindFieldByName(name);
        value = reflection_ptr_->GetUInt64(*msg_ptr_, field);
        return 0;
    }

    int set_attr(const std::string& name, const float& value, std::string* errmsg = nullptr)
    {
        const google::protobuf::FieldDescriptor* field = desc_ptr_->FindFieldByName(name);

        if (field->is_repeated()) {
            if (errmsg) {
                *errmsg = name + " is a repeated field!";
            }
            return -1;
        }

        reflection_ptr_->SetFloat(msg_ptr_, field, value);

        return 0;
    }

    int get_attr(const std::string& name, float& value, std::string* errmsg = nullptr)
    {
        const google::protobuf::FieldDescriptor* field = desc_ptr_->FindFieldByName(name);
        value = reflection_ptr_->GetFloat(*msg_ptr_, field);
        return 0;
    }

    int set_attr(const std::string& name, const double& value, std::string* errmsg = nullptr)
    {
        const google::protobuf::FieldDescriptor* field = desc_ptr_->FindFieldByName(name);

        if (field->is_repeated()) {
            if (errmsg) {
                *errmsg = name + " is a repeated field!";
            }
            return -1;
        }

        reflection_ptr_->SetDouble(msg_ptr_, field, value);

        return 0;
    }

    int get_attr(const std::string& name, double& value, std::string* errmsg = nullptr)
    {
        const google::protobuf::FieldDescriptor* field = desc_ptr_->FindFieldByName(name);
        value = reflection_ptr_->GetDouble(*msg_ptr_, field);
        return 0;
    }

    int set_attr(const std::string& name, const bool& value, std::string* errmsg = nullptr)
    {
        const google::protobuf::FieldDescriptor* field = desc_ptr_->FindFieldByName(name);

        if (field->is_repeated()) {
            if (errmsg) {
                *errmsg = name + " is a repeated field!";
            }
            return -1;
        }

        reflection_ptr_->SetBool(msg_ptr_, field, value);

        return 0;
    }

    int get_attr(const std::string& name, bool& value, std::string* errmsg = nullptr)
    {
        const google::protobuf::FieldDescriptor* field = desc_ptr_->FindFieldByName(name);
        value = reflection_ptr_->GetBool(*msg_ptr_, field);
        return 0;
    }

    int set_attr(const std::string& name, const std::string& value, std::string* errmsg = nullptr)
    {
        const google::protobuf::FieldDescriptor* field = desc_ptr_->FindFieldByName(name);

        if (field->is_repeated()) {
            if (errmsg) {
                *errmsg = name + " is a repeated field!";
            }
            return -1;
        }

        reflection_ptr_->SetString(msg_ptr_, field, value);

        return 0;
    }

    int get_attr(const std::string& name, std::string& value, std::string* errmsg = nullptr)
    {
        const google::protobuf::FieldDescriptor* field = desc_ptr_->FindFieldByName(name);
        value = reflection_ptr_->GetString(*msg_ptr_, field);
        return 0;
    }

    int set_enum_attr(const std::string& name, const int32_t& value, std::string* errmsg = nullptr)
    {
        const google::protobuf::FieldDescriptor* field = desc_ptr_->FindFieldByName(name);

        if (field->is_repeated()) {
            if (errmsg) {
                *errmsg = name + " is a repeated field!";
            }
            return -1;
        }

        reflection_ptr_->SetEnumValue(msg_ptr_, field, value);

        return 0;
    }

    int get_enum_attr(const std::string& name, int32_t& value, std::string* errmsg = nullptr)
    {
        const google::protobuf::FieldDescriptor* field = desc_ptr_->FindFieldByName(name);
        value = reflection_ptr_->GetEnumValue(*msg_ptr_, field);
        return 0;
    }

    int set_attr(const std::string& name, pbmsg_t* value, std::string* errmsg = nullptr)
    {
        google::protobuf::Message* submsg = value->get_msg();
        const google::protobuf::FieldDescriptor* field = desc_ptr_->FindFieldByName(name);

        if (field->is_repeated()) {
            if (errmsg) {
                *errmsg = name + " is a repeated field!";
            }
            return -1;
        }

        reflection_ptr_->SetAllocatedMessage(msg_ptr_, submsg, field);

        return 0;
    }

    int get_attr(const std::string& name, pbmsg_t** value, std::string* errmsg = nullptr)
    {
        const google::protobuf::FieldDescriptor* field = desc_ptr_->FindFieldByName(name);
        auto msg = reflection_ptr_->MutableMessage(msg_ptr_, field);
        *value = pbmsg_t::create(msg);
        return 0;
    }

    google::protobuf::Message* get_msg(bool standalone = false)
    {
        google::protobuf::Message* ret = msg_ptr_;

        if (standalone) {
            ret = msg_ptr_->New();
            ret->CopyFrom(*msg_ptr_);
        }
        return ret;
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

    pbmsg_t(const pbmsg_t& pbmsg)
    {
        msg_ptr_ = pbmsg.msg_ptr_->New();
        msg_ptr_->CopyFrom(*pbmsg.msg_ptr_);
        desc_ptr_ = pbmsg.msg_ptr_->GetDescriptor();
        reflection_ptr_ = pbmsg.msg_ptr_->GetReflection();
    }

    ~pbmsg_t()
    {
        if (!trusted_ && msg_ptr_ != nullptr) {
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
    bool trusted_{false};
    const google::protobuf::FileDescriptor* file_desc_ptr_{ nullptr };
    const google::protobuf::Descriptor* desc_ptr_{ nullptr };
    const google::protobuf::Reflection* reflection_ptr_{ nullptr };
    google::protobuf::Message* msg_ptr_{ nullptr };
};
