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
    static pbmsg_t* create(const google::protobuf::Message* msg)
    {
        pbmsg_t* pbmsg = new (std::nothrow) pbmsg_t();
        if (nullptr == pbmsg) {
            return nullptr;
        }
    
        pbmsg->msg_ptr_ = msg->New();
        pbmsg->msg_ptr_->CopyFrom(*msg);
        pbmsg->desc_ptr_ = msg->GetDescriptor();
        pbmsg->reflection_ptr_ = msg->GetReflection();

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
        importer_ptr_ = new (std::nothrow) google::protobuf::compiler::Importer(&source_tree, &error_collector);
        if (nullptr == importer_ptr_) {
            if (errmsg) {*errmsg = "alloc importer failed!";}
            return -1;
        }

        file_desc_ptr_ = importer_ptr_->Import(basename);
        if (nullptr == file_desc_ptr_) {
            return -2;
        }
        if (!error_collector.text_.empty()) {
            if (errmsg) {*errmsg = error_collector.text_;}
            return -3;
        }

        factory_ptr_ = new (std::nothrow) google::protobuf::DynamicMessageFactory(file_desc_ptr_->pool());
        if (nullptr == importer_ptr_) {
            if (errmsg) {*errmsg = "alloc message factory failed!";}
            return -4;
        }

        desc_ptr_ = file_desc_ptr_->pool()->FindMessageTypeByName(msgtype);
        if (nullptr == desc_ptr_) {
            if (errmsg) {*errmsg = "alloc importer failed!"+ msgtype;}
            return -5;
        }

        msg_ptr_ = factory_ptr_->GetPrototype(desc_ptr_)->New();

        reflection_ptr_ = msg_ptr_->GetReflection();

        return 0;
    }

    int set_attr(const std::string& name, const int32_t& value, std::string* errmsg = nullptr)
    {
        const google::protobuf::FieldDescriptor* field = desc_ptr_->FindFieldByName(name);

        if (field->is_repeated()) {
            if (errmsg) {*errmsg = name + " is a repeated field!";}
            return -1;
        }

        reflection_ptr_->SetInt32(msg_ptr_, field, value);

        return 0;
    }

    int get_attr(const std::string& name, int32_t& value, std::string* errmsg = nullptr)
    {
        const google::protobuf::FieldDescriptor* field = desc_ptr_->FindFieldByName(name);
        value = reflection_ptr_->GetInt32(*msg_ptr_, field);
        return 0;
    }


    int set_attr(const std::string& name, const uint32_t& value, std::string* errmsg = nullptr)
    {
        const google::protobuf::FieldDescriptor* field = desc_ptr_->FindFieldByName(name);

        if (field->is_repeated()) {
            if (errmsg) {*errmsg = name + " is a repeated field!";}
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
            if (errmsg) {*errmsg = name + " is a repeated field!";}
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
            if (errmsg) {*errmsg = name + " is a repeated field!";}
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
            if (errmsg) {*errmsg = name + " is a repeated field!";}
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
            if (errmsg) {*errmsg = name + " is a repeated field!";}
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
            if (errmsg) {*errmsg = name + " is a repeated field!";}
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
            if (errmsg) {*errmsg = name + " is a repeated field!";}
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

    int set_attr(const std::string& name, pbmsg_t* value, std::string* errmsg = nullptr)
    {
        google::protobuf::Message* submsg = value->get_msg();
        const google::protobuf::FieldDescriptor* field = desc_ptr_->FindFieldByName(name);

        if (field->is_repeated()) {
            if (errmsg) {*errmsg = name + " is a repeated field!";}
            return -1;
        }

        reflection_ptr_->SetAllocatedMessage(msg_ptr_, submsg, field);

        return 0;
    }

    int get_attr(const std::string& name, pbmsg_t** value, std::string* errmsg = nullptr)
    {
        const google::protobuf::FieldDescriptor* field = desc_ptr_->FindFieldByName(name);
        auto& msg = reflection_ptr_->GetMessage(*msg_ptr_, field);
        *value = pbmsg_t::create(&msg);
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

    pbmsg_t(const pbmsg_t& pbmsg)
    {
        msg_ptr_ = pbmsg.msg_ptr_->New();
        msg_ptr_->CopyFrom(*pbmsg.msg_ptr_);
        desc_ptr_ = pbmsg.msg_ptr_->GetDescriptor();
        reflection_ptr_ = pbmsg.msg_ptr_->GetReflection();
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
