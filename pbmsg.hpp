#include <new>
#include <cassert>
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

using google::protobuf::FieldDescriptor;
using google::protobuf::Message;
using google::protobuf::Reflection;

template <typename T>
struct field_oper_t {
    void set(const Reflection* ref, Message* msg, const FieldDescriptor* field, T value)
    {
        assert(field->type() == FieldDescriptor::TYPE_ENUM);   
        ref->SetEnumValue(msg, field, value);
    }

    void set(const Reflection* ref, Message* msg, const FieldDescriptor* field, int idx, T value)
    {
        assert(field->type() == FieldDescriptor::TYPE_ENUM);   
        ref->SetRepeatedEnumValue(msg, field, idx, value);
    }

    void add(const Reflection* ref, Message* msg, const FieldDescriptor* field, T value)
    {
        assert(field->type() == FieldDescriptor::TYPE_ENUM);  
        ref->AddEnumValue(msg, field, value);
    }

    T get(const Reflection* ref, Message* msg, const FieldDescriptor* field)
    {
        assert(field->type() == FieldDescriptor::TYPE_ENUM);  
        return ref->GetEnumValue(*msg, field);
    }

    T get(const Reflection* ref, Message* msg, const FieldDescriptor* field, int idx)
    {
        assert(field->type() == FieldDescriptor::TYPE_ENUM); 
        return ref->GetRepeatedEnumValue(*msg, field, idx);
    }
};
class pbmsg_t final {
public:
    static pbmsg_t* create(google::protobuf::Message* msg, bool trusted = false)
    {
        if (nullptr == msg) {
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
        static google::protobuf::compiler::DiskSourceTree source_tree;
        source_tree.MapPath("", "./");
        source_tree.MapPath("", parent);
        static google::protobuf::compiler::Importer importer(&source_tree, &error_collector);
        file_desc_ptr_ = importer.Import(basename);
        if (!error_collector.text_.empty()) {
            if (errmsg) {
                *errmsg = error_collector.text_;
            }
            return -1;
        }
        if (nullptr == file_desc_ptr_) {
            return -2;
        }

        static google::protobuf::DynamicMessageFactory factory;

        desc_ptr_ = file_desc_ptr_->pool()->FindMessageTypeByName(msgtype);
        if (nullptr == desc_ptr_) {
            if (errmsg) {
                *errmsg = "alloc importer failed!" + msgtype;
            }
            return -3;
        }

        msg_ptr_ = factory.GetPrototype(desc_ptr_)->New();

        reflection_ptr_ = msg_ptr_->GetReflection();

        return 0;
    }

    template <typename T1>
    int set_attr(const std::string& name, const T1& value, std::string* errmsg = nullptr)
    {
        const google::protobuf::FieldDescriptor* field = desc_ptr_->FindFieldByName(name);
        if (nullptr == field) {
            if (errmsg) {
                *errmsg = name + " is a nonexistent field!";
            }
            return -1;
        }

        if (field->is_repeated()) {
            if (errmsg) {
                *errmsg = name + " is a repeated field!";
            }
            return -2;
        }

        field_oper_t<T1>().set(reflection_ptr_, msg_ptr_, field, value);

        return 0;
    }

    template <typename T1>
    int get_attr(const std::string& name, T1& value, std::string* errmsg = nullptr)
    {
        const google::protobuf::FieldDescriptor* field = desc_ptr_->FindFieldByName(name);
        if (nullptr == field) {
            if (errmsg) {
                *errmsg = name + " is a nonexistent field!";
            }
            return -1;
        }

        value = field_oper_t<T1>().get(reflection_ptr_, msg_ptr_, field);

        return 0;
    }

    template <typename T1>
    int add_attr(const std::string& name, const int64_t& value, std::string* errmsg = nullptr)
    {
        const google::protobuf::FieldDescriptor* field = desc_ptr_->FindFieldByName(name);
        if (nullptr == field) {
            if (errmsg) {
                *errmsg = name + " is a nonexistent field!";
            }
            return -1;
        }

        if (!field->is_repeated()) {
            if (errmsg) {
                *errmsg = name + " is't a repeated field!";
            }
            return -2;
        }

        field_oper_t<T1>().add(reflection_ptr_, msg_ptr_, field);

        return 0;
    }

    template <typename T1>
    int set_attr(const std::string& name, int idx, const T1& value, std::string* errmsg = nullptr)
    {
        const google::protobuf::FieldDescriptor* field = desc_ptr_->FindFieldByName(name);
        if (nullptr == field) {
            if (errmsg) {
                *errmsg = name + " is a nonexistent field!";
            }
            return -1;
        }

        if (!field->is_repeated()) {
            if (errmsg) {
                *errmsg = name + " is't a repeated field!";
            }
            return -2;
        }

        int maxsize = reflection_ptr_->FieldSize(*msg_ptr_, field);
        if (idx >= maxsize) {
            if (errmsg) {
                *errmsg = name + " out-of-bounds access, max idx is " + std::to_string(maxsize);
            }
            return -3;
        }

        field_oper_t<T1>().set(reflection_ptr_, msg_ptr_, field, idx, value);

        return 0;
    }

    template <typename T1>
    int get_attr(const std::string& name, int idx, T1& value, std::string* errmsg = nullptr)
    {
        const google::protobuf::FieldDescriptor* field = desc_ptr_->FindFieldByName(name);
        if (nullptr == field) {
            if (errmsg) {
                *errmsg = name + " is a nonexistent field!";
            }
            return -1;
        }

        if (!field->is_repeated()) {
            if (errmsg) {
                *errmsg = name + " is't a repeated field!";
            }
            return -2;
        }

        int maxsize = reflection_ptr_->FieldSize(*msg_ptr_, field);
        if (idx >= maxsize) {
            if (errmsg) {
                *errmsg = name + " out-of-bounds access, max idx is " + std::to_string(maxsize);
            }
            return -3;
        }

        value = field_oper_t<T1>().get(reflection_ptr_, msg_ptr_, field, idx);
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
    bool trusted_{ false };
    const google::protobuf::FileDescriptor* file_desc_ptr_{ nullptr };
    const google::protobuf::Descriptor* desc_ptr_{ nullptr };
    const google::protobuf::Reflection* reflection_ptr_{ nullptr };
    google::protobuf::Message* msg_ptr_{ nullptr };
};

template <>
struct field_oper_t<int32_t> {
    void set(const Reflection* ref, Message* msg, const FieldDescriptor* field, int32_t value)
    {
        if (field->type() == FieldDescriptor::TYPE_ENUM) {
            ref->SetEnumValue(msg, field, value);
        } else {
            ref->SetInt32(msg, field, value);
        }
    }

    void set(const Reflection* ref, Message* msg, const FieldDescriptor* field, int idx, int32_t value)
    {
        if (field->type() == FieldDescriptor::TYPE_ENUM) {
            ref->SetRepeatedEnumValue(msg, field, idx, value);
        } else {
            ref->SetRepeatedInt32(msg, field, idx, value);
        }
    }

    void add(const Reflection* ref, Message* msg, const FieldDescriptor* field, int32_t value)
    {
        if (field->type() == FieldDescriptor::TYPE_ENUM) {
            ref->AddEnumValue(msg, field, value);
        } else {
            ref->AddInt32(msg, field, value);
        }
    }

    int32_t get(const Reflection* ref, Message* msg, const FieldDescriptor* field)
    {
        if (field->type() == FieldDescriptor::TYPE_ENUM) {
            return ref->GetEnumValue(*msg, field);
        }
        return ref->GetInt32(*msg, field);
    }

    int32_t get(const Reflection* ref, Message* msg, const FieldDescriptor* field, int idx)
    {
        if (field->type() == FieldDescriptor::TYPE_ENUM) {
            return ref->GetRepeatedEnumValue(*msg, field, idx);
        }
        return ref->GetRepeatedInt32(*msg, field, idx);
    }
};

template <>
struct field_oper_t<uint32_t> {
    void set(const Reflection* ref, Message* msg, const FieldDescriptor* field, uint32_t value)
    {
        ref->SetUInt32(msg, field, value);
    }

    void set(const Reflection* ref, Message* msg, const FieldDescriptor* field, int idx, uint32_t value)
    {
        ref->SetRepeatedUInt32(msg, field, idx, value);
    }

    void add(const Reflection* ref, Message* msg, const FieldDescriptor* field, uint32_t value)
    {
        ref->AddUInt32(msg, field, value);
    }

    uint32_t get(const Reflection* ref, Message* msg, const FieldDescriptor* field)
    {
        return ref->GetUInt32(*msg, field);
    }

    uint32_t get(const Reflection* ref, Message* msg, const FieldDescriptor* field, int idx)
    {
        return ref->GetRepeatedUInt32(*msg, field, idx);
    }
};

template <>
struct field_oper_t<int64_t> {
    void set(const Reflection* ref, Message* msg, const FieldDescriptor* field, int64_t value)
    {
        ref->SetInt64(msg, field, value);
    }

    void set(const Reflection* ref, Message* msg, const FieldDescriptor* field, int idx, int64_t value)
    {
        ref->SetRepeatedInt64(msg, field, idx, value);
    }

    void add(const Reflection* ref, Message* msg, const FieldDescriptor* field, int64_t value)
    {
        ref->AddInt64(msg, field, value);
    }

    int64_t get(const Reflection* ref, Message* msg, const FieldDescriptor* field)
    {
        return ref->GetInt64(*msg, field);
    }

    int64_t get(const Reflection* ref, Message* msg, const FieldDescriptor* field, int idx)
    {
        return ref->GetRepeatedInt64(*msg, field, idx);
    }
};

template <>
struct field_oper_t<uint64_t> {
    void set(const Reflection* ref, Message* msg, const FieldDescriptor* field, uint64_t value)
    {
        ref->SetUInt64(msg, field, value);
    }

    void set(const Reflection* ref, Message* msg, const FieldDescriptor* field, int idx, uint64_t value)
    {
        ref->SetRepeatedUInt64(msg, field, idx, value);
    }

    void add(const Reflection* ref, Message* msg, const FieldDescriptor* field, uint64_t value)
    {
        ref->AddUInt64(msg, field, value);
    }

    uint64_t get(const Reflection* ref, Message* msg, const FieldDescriptor* field)
    {
        return ref->GetUInt64(*msg, field);
    }

    uint64_t get(const Reflection* ref, Message* msg, const FieldDescriptor* field, int idx)
    {
        return ref->GetRepeatedUInt64(*msg, field, idx);
    }
};

template <>
struct field_oper_t<float> {
    void set(const Reflection* ref, Message* msg, const FieldDescriptor* field, float value)
    {
        ref->SetFloat(msg, field, value);
    }

    void set(const Reflection* ref, Message* msg, const FieldDescriptor* field, int idx, float value)
    {
        ref->SetRepeatedFloat(msg, field, idx, value);
    }

    void add(const Reflection* ref, Message* msg, const FieldDescriptor* field, float value)
    {
        ref->AddFloat(msg, field, value);
    }

    float get(const Reflection* ref, Message* msg, const FieldDescriptor* field)
    {
        return ref->GetFloat(*msg, field);
    }

    float get(const Reflection* ref, Message* msg, const FieldDescriptor* field, int idx)
    {
        return ref->GetRepeatedFloat(*msg, field, idx);
    }
};

template <>
struct field_oper_t<double> {
    void set(const Reflection* ref, Message* msg, const FieldDescriptor* field, double value)
    {
        ref->SetDouble(msg, field, value);
    }

    void set(const Reflection* ref, Message* msg, const FieldDescriptor* field, int idx, double value)
    {
        ref->SetRepeatedDouble(msg, field, idx, value);
    }

    void add(const Reflection* ref, Message* msg, const FieldDescriptor* field, double value)
    {
        ref->AddDouble(msg, field, value);
    }

    double get(const Reflection* ref, Message* msg, const FieldDescriptor* field)
    {
        return ref->GetDouble(*msg, field);
    }

    double get(const Reflection* ref, Message* msg, const FieldDescriptor* field, int idx)
    {
        return ref->GetRepeatedDouble(*msg, field, idx);
    }
};

template <>
struct field_oper_t<bool> {
    void set(const Reflection* ref, Message* msg, const FieldDescriptor* field, bool value)
    {
        ref->SetBool(msg, field, value);
    }

    void set(const Reflection* ref, Message* msg, const FieldDescriptor* field, int idx, bool value)
    {
        ref->SetRepeatedBool(msg, field, idx, value);
    }

    void add(const Reflection* ref, Message* msg, const FieldDescriptor* field, bool value)
    {
        ref->AddBool(msg, field, value);
    }

    bool get(const Reflection* ref, Message* msg, const FieldDescriptor* field)
    {
        return ref->GetBool(*msg, field);
    }

    bool get(const Reflection* ref, Message* msg, const FieldDescriptor* field, int idx)
    {
        return ref->GetRepeatedBool(*msg, field, idx);
    }
};

template <>
struct field_oper_t<std::string> {
    void set(const Reflection* ref, Message* msg, const FieldDescriptor* field, std::string value)
    {
        ref->SetString(msg, field, value);
    }

    void set(const Reflection* ref, Message* msg, const FieldDescriptor* field, int idx, std::string value)
    {
        ref->SetRepeatedString(msg, field, idx, value);
    }

    void add(const Reflection* ref, Message* msg, const FieldDescriptor* field, std::string value)
    {
        ref->AddString(msg, field, value);
    }

    std::string get(const Reflection* ref, Message* msg, const FieldDescriptor* field)
    {
        return ref->GetString(*msg, field);
    }

    std::string get(const Reflection* ref, Message* msg, const FieldDescriptor* field, int idx)
    {
        return ref->GetRepeatedString(*msg, field, idx);
    }
};

//////////////////////
template <>
struct field_oper_t<Message*> {
    void set(const Reflection* ref, Message* msg, const FieldDescriptor* field, Message* value)
    {
        ref->SetAllocatedMessage(msg, value, field);
    }

    void set(const Reflection* ref, Message* msg, const FieldDescriptor* field, int idx, Message* value)
    {
    }

    void add(const Reflection* ref, Message* msg, const FieldDescriptor* field, Message* value)
    {
        ref->AddAllocatedMessage(msg, field, value);
    }

    Message* get(const Reflection* ref, Message* msg, const FieldDescriptor* field)
    {
        return ref->MutableMessage(msg, field);
    }

    Message* get(const Reflection* ref, Message* msg, const FieldDescriptor* field, int idx)
    {
        return ref->MutableRepeatedMessage(msg, field, idx);
    }
};

template <>
struct field_oper_t<pbmsg_t*> {
    void set(const Reflection* ref, Message* msg, const FieldDescriptor* field, pbmsg_t* value)
    {
        ref->SetAllocatedMessage(msg, value->get_msg(), field);
    }

    void set(const Reflection* ref, Message* msg, const FieldDescriptor* field, int idx, pbmsg_t* value)
    {
    }

    void add(const Reflection* ref, Message* msg, const FieldDescriptor* field, pbmsg_t* value)
    {
        // ref->AddAllocatedMessage(msg, field, value);
    }

    pbmsg_t* get(const Reflection* ref, Message* msg, const FieldDescriptor* field)
    {
        return pbmsg_t::create(ref->MutableMessage(msg, field));
    }

    pbmsg_t* get(const Reflection* ref, Message* msg, const FieldDescriptor* field, int idx)
    {
        return pbmsg_t::create(ref->MutableRepeatedMessage(msg, field, idx));
    }
};
