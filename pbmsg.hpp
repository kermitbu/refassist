#include "google/protobuf/compiler/importer.h"
#include "google/protobuf/dynamic_message.h"
#include "google/protobuf/message.h"
#include <cassert>
#include <new>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-value"

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
    void set(const Reflection* ref, Message* msg, const FieldDescriptor* field, T value, std::string& errmsg)
    {
        if (field->type() != FieldDescriptor::TYPE_ENUM) {
            errmsg = "value type and field type do not match, field type is " + std::string(field->type_name());
            assert(("value type and field type do not match", false));
            return;
        }
        ref->SetEnumValue(msg, field, value);
    }

    void set(const Reflection* ref, Message* msg, const FieldDescriptor* field, int idx, T value, std::string& errmsg)
    {
        if (field->type() != FieldDescriptor::TYPE_ENUM) {
            errmsg = "value type and field type do not match, field type is " + std::string(field->type_name());
            assert(("value type and field type do not match", false));
            return;
        }

        ref->SetRepeatedEnumValue(msg, field, idx, value);
    }

    void add(const Reflection* ref, Message* msg, const FieldDescriptor* field, T value, std::string& errmsg)
    {
        if (field->type() != FieldDescriptor::TYPE_ENUM) {
            errmsg = "value type and field type do not match, field type is " + std::string(field->type_name());
            assert(("value type and field type do not match", false));
            return;
        }

        ref->AddEnumValue(msg, field, value);
    }

    T get(const Reflection* ref, Message* msg, const FieldDescriptor* field, std::string& errmsg)
    {
        if (field->type() != FieldDescriptor::TYPE_ENUM) {
            errmsg = "value type and field type do not match, field type is " + std::string(field->type_name());
            assert(("value type and field type do not match", false));
            return T();
        }

        return T(ref->GetEnumValue(*msg, field));
    }

    T get(const Reflection* ref, Message* msg, const FieldDescriptor* field, int idx, std::string& errmsg)
    {
        if (field->type() != FieldDescriptor::TYPE_ENUM) {
            errmsg = "value type and field type do not match, field type is " + std::string(field->type_name());
            assert(("value type and field type do not match", false));
            return T();
        }

        return T(ref->GetRepeatedEnumValue(*msg, field, idx));
    }
};

/**
 * @brief 利用PB反射操作数据的封装
 * 
 */
class pbmsg_t final {
public:
    /**
     * @brief 通过已有的PB对象指针创建pbmsg_t对象
     * 
     * @param msg 一个PB对象的指针
     * @param trusted 是否托管PB对象指针，false会复制出一份，true直接使用外部传入的PB对象
     * @return pbmsg_t* 分配好的pbmsg_t指针
     * 
     * @attention 当trusted为true的时候，需要注意外部的msg生命周期应该比pbmsg_t生命周期长。
     */
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

    /**
     * @brief 通过proto文件创建分配pbmsg_t对象
     * 
     * @param file 带路径的PROTO文件名
     * @param msgtype PROTO文件中定义的消息名
     * @return pbmsg_t* 分配好的pbmsg_t指针
     * 
     * @note 嵌套的消息用'.'进行分隔, 例如msga.msgb表示在msga内部定义的msgb
     */
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

    /**
     * @brief 导入PROTO文件，并创建指定的消息
     * 
     * @param file 带路径的PROTO文件名
     * @param msgtype PROTO文件中定义的消息名
     * @param errmsg 错误信息
     * @return int 导入是否成功
     * 
     * @note 当返回值不为0时，把错误信息填入到errmsg中，如不关注错误信息则可以忽略errmsg参数
     */
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
            if (errmsg) {
                *errmsg = "import proto file failed!";
            }
            return -2;
        }

        static google::protobuf::DynamicMessageFactory factory;

        desc_ptr_ = file_desc_ptr_->pool()->FindMessageTypeByName(msgtype);
        if (nullptr == desc_ptr_) {
            if (errmsg) {
                *errmsg = msgtype + " was not found!";
            }
            return -3;
        }

        msg_ptr_ = factory.GetPrototype(desc_ptr_)->New();
        if (nullptr == msg_ptr_) {
            if (errmsg) {
                *errmsg = "alloc msg failed!";
            }
            return -4;
        }

        reflection_ptr_ = msg_ptr_->GetReflection();

        return 0;
    }

    /**
     * @brief 设置指定域的数据
     * 
     * @tparam T1 
     * @param name 指定设置域的名 
     * @param value 指定设置域的值
     * @param errmsg 错误信息
     * @return int 设置参数是否成功
     * 
     * @note 当返回值不为0时，把错误信息填入到errmsg中，如不关注错误信息则可以忽略errmsg参数
     * @attention 传入T类型为Message*或pbmsg_t*的时候，会产生独立副本，需要使用者管理其生命周期
     * 
     */
    template <typename T1>
    int set_attr(const std::string& name, const T1& value, std::string* errmsg = nullptr)
    {
        int ret = 0;
        do {
            const google::protobuf::FieldDescriptor* field = desc_ptr_->FindFieldByName(name);
            if (nullptr == field) {
                error_msg_ = name + " is a non-existent field!";
                ret = -1;
                break;
            }

            if (field->is_repeated()) {
                error_msg_ = name + " is a repeated field!";
                ret = -2;
                break;
            }

            field_oper_t<T1>().set(reflection_ptr_, msg_ptr_, field, value, error_msg_);
            if (!error_msg_.empty()) {
                ret = -3;
                break;
            }
        } while (0);

        if (errmsg) {
            *errmsg = error_msg_;
        }

        return ret;
    }

    /**
     * @brief 获取指定域的数据
     * 
     * @tparam T1 
     * @param name 指定获取域的名 
     * @param value 指定获取域的值
     * @param errmsg 错误信息
     * @return int 获取域是否成功
     * 
     * @note 当返回值不为0时，把错误信息填入到errmsg中，如不关注错误信息则可以忽略errmsg参数
     * @attention 传入T类型为Message*或pbmsg_t*的时候，会产生独立副本，需要使用者管理其生命周期
     */
    template <typename T1>
    int get_attr(const std::string& name, T1& value, std::string* errmsg = nullptr)
    {
        int ret = 0;
        do {
            const google::protobuf::FieldDescriptor* field = desc_ptr_->FindFieldByName(name);
            if (nullptr == field) {
                error_msg_ = name + " is a non-existent field!";
                ret = -1;
                break;
            }

            if (field->is_repeated()) {
                error_msg_ = name + " is a repeated field!";
                ret = -2;
                break;
            }

            value = field_oper_t<T1>().get(reflection_ptr_, msg_ptr_, field, error_msg_);
            if (!error_msg_.empty()) {
                ret = -3;
                break;
            }
        } while (0);

        if (errmsg) {
            *errmsg = error_msg_;
        }

        return ret;
    }

    /**
     * @brief 为repeated数据添加一条新记录
     * 
     * @tparam T1 
     * @param name 指定添加数据的域名 
     * @param value 指定添加数据的值
     * @param errmsg 错误信息
     * @return int 指定域添加数据是否成功
     * 
     * @note 当返回值不为0时，把错误信息填入到errmsg中，如不关注错误信息则可以忽略errmsg参数
     * @attention 传入T类型为Message*或pbmsg_t*的时候，会产生独立副本，需要使用者管理其生命周期
     */
    template <typename T1>
    int add_attr(const std::string& name, const T1& value, std::string* errmsg = nullptr)
    {
        int ret = 0;
        do {
            const google::protobuf::FieldDescriptor* field = desc_ptr_->FindFieldByName(name);
            if (nullptr == field) {
                error_msg_ = name + " is a non-existent field!";
                ret = -1;
                break;
            }

            if (!field->is_repeated()) {
                error_msg_ = name + " is't a repeated field!";
                ret = -2;
                break;
            }

            field_oper_t<T1>().add(reflection_ptr_, msg_ptr_, field, value, error_msg_);
            if (!error_msg_.empty()) {
                ret = -3;
                break;
            }
        } while (0);

        if (errmsg) {
            *errmsg = error_msg_;
        }

        return ret;
    }

    /**
     * @brief 修改repeated数据中指定索引的记录
     * 
     * @tparam T1 
     * @param name 指定数据的域名 
     * @param idx 指定记录的索引
     * @param value 指定数据的值
     * @param errmsg 错误信息
     * @return int 设置指定索引的数据是否成功
     * 
     * @note 当返回值不为0时，把错误信息填入到errmsg中，如不关注错误信息则可以忽略errmsg参数
     * @attention 传入T类型为Message*或pbmsg_t*的时候，会产生独立副本，需要使用者管理其生命周期
     */
    template <typename T1>
    int set_attr(const std::string& name, int idx, const T1& value, std::string* errmsg = nullptr)
    {
        int ret = 0;
        do {
            const google::protobuf::FieldDescriptor* field = desc_ptr_->FindFieldByName(name);
            if (nullptr == field) {
                error_msg_ = name + " is a non-existent field!";
                ret = -1;
                break;
            }

            if (!field->is_repeated()) {
                error_msg_ = name + " is't a repeated field!";
                ret = -2;
                break;
            }
            int maxsize = reflection_ptr_->FieldSize(*msg_ptr_, field);
            if (idx >= maxsize) {
                error_msg_ = name + " out-of-bounds access, max idx is " + std::to_string(maxsize);
                ret = -3;
                break;
            }

            field_oper_t<T1>().set(reflection_ptr_, msg_ptr_, field, idx, value, error_msg_);
            if (!error_msg_.empty()) {
                ret = -4;
                break;
            }
        } while (0);

        if (errmsg) {
            *errmsg = error_msg_;
        }

        return ret;
    }

    /**
     * @brief 获取repeated数据中指定索引的记录
     * 
     * @tparam T1 
     * @param name 指定数据的域名 
     * @param idx 指定记录的索引
     * @param value 获取到的数据的值
     * @param errmsg 错误信息
     * @return int 获取指定索引的数据是否成功
     * 
     * @note 当返回值不为0时，把错误信息填入到errmsg中，如不关注错误信息则可以忽略errmsg参数
     * @attention 传入T类型为Message*或pbmsg_t*的时候，会产生独立副本，需要使用者管理其生命周期
     */
    template <typename T1>
    int get_attr(const std::string& name, int idx, T1& value, std::string* errmsg = nullptr)
    {
        int ret = 0;
        do {
            const google::protobuf::FieldDescriptor* field = desc_ptr_->FindFieldByName(name);
            if (nullptr == field) {
                error_msg_ = name + " is a non-existent field!";
                ret = -1;
                break;
            }

            if (!field->is_repeated()) {
                error_msg_ = name + " is't a repeated field!";
                ret = -2;
                break;
            }
            int maxsize = reflection_ptr_->FieldSize(*msg_ptr_, field);
            if (idx >= maxsize) {
                error_msg_ = name + " out-of-bounds access, max idx is " + std::to_string(maxsize);
                ret = -3;
                break;
            }

            value = field_oper_t<T1>().get(reflection_ptr_, msg_ptr_, field, idx, error_msg_);
            if (!error_msg_.empty()) {
                ret = -4;
                break;
            }
        } while (0);

        if (errmsg) {
            *errmsg = error_msg_;
        }

        return ret;
    }

    int get_attr_size(const std::string& name, std::string* errmsg = nullptr)
    {
        int ret = 0;
        do {
            const google::protobuf::FieldDescriptor* field = desc_ptr_->FindFieldByName(name);
            if (nullptr == field) {
                error_msg_ = name + " is a non-existent field!";
                ret = -1;
                break;
            }

            if (!field->is_repeated()) {
                error_msg_ = name + " is't a repeated field!";
                ret = -2;
                break;
            }

            ret = reflection_ptr_->FieldSize(*msg_ptr_, field);

        } while (0);

        if (errmsg) {
            *errmsg = error_msg_;
        }
        return ret;
    }

    /**
     * @brief 清理指定域的数据
     * 
     * @param name 指定数据的域名
     * @param errmsg 错误信息
     * @return int 清理是否成功
     * 
     * @note 当返回值不为0时，把错误信息填入到errmsg中，如不关注错误信息则可以忽略errmsg参数
     */
    int clear_attr(const std::string& name, std::string* errmsg = nullptr)
    {
        int ret = 0;
        do {
            const google::protobuf::FieldDescriptor* field = desc_ptr_->FindFieldByName(name);
            if (nullptr == field) {
                error_msg_ = name + " is a non-existent field!";
                ret = -1;
                break;
            }

            reflection_ptr_->ClearField(msg_ptr_, field);
        } while (0);

        if (errmsg) {
            *errmsg = error_msg_;
        }
        return ret;
    }

    /**
     * @brief 获取一个PB消息的指针
     * 
     * @param standalone 是否获取有独立生命周期的PB指针
     * @return google::protobuf::Message* 获取到的消息指针
     * 
     * @note standalone为true时获取到独立于pbmsg_t的PB指针，需要调用者主动调用delete进行释放
     */
    google::protobuf::Message* get_msg(bool standalone = false)
    {
        google::protobuf::Message* ret = msg_ptr_;

        if (standalone) {
            ret = msg_ptr_->New();
            ret->CopyFrom(*msg_ptr_);
        }
        return ret;
    }

    /**
     * @brief 获取到一个PB序列化之后的二进制数据
     * 
     * @return std::string 序列化的二进制序列
     */
    std::string get_bin()
    {
        std::string str;
        msg_ptr_->SerializeToString(&str);

        return str;
    }

    /**
     * @brief 使用一个PB二进制数据设置pbmsg_t
     * 
     * @param pbdata 序列化的PB二进制数据
     * @return int 设置是否成功
     */
    int set_bin(std::string& pbdata)
    {
        return msg_ptr_->ParseFromString(pbdata) ? 0 : -1;
    }

    pbmsg_t() = default;

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
    std::string error_msg_;
    const google::protobuf::FileDescriptor* file_desc_ptr_{ nullptr };
    const google::protobuf::Descriptor* desc_ptr_{ nullptr };
    const google::protobuf::Reflection* reflection_ptr_{ nullptr };
    google::protobuf::Message* msg_ptr_{ nullptr };
};

template <>
struct field_oper_t<int32_t> {
    void set(const Reflection* ref, Message* msg, const FieldDescriptor* field, int32_t value, std::string& errmsg)
    {
        switch (field->type()) {
        case FieldDescriptor::TYPE_ENUM:
            ref->SetEnumValue(msg, field, value);
            break;
        case FieldDescriptor::TYPE_INT32:
        case FieldDescriptor::TYPE_SINT32:
        case FieldDescriptor::TYPE_SFIXED32:
            ref->SetInt32(msg, field, value);
            break;
        default:
            errmsg = "value type and field type do not match, field type is " + std::string(field->type_name());
            assert(("value type and field type do not match", false));
            break;
        }
    }

    void set(const Reflection* ref, Message* msg, const FieldDescriptor* field, int idx, int32_t value, std::string& errmsg)
    {
        switch (field->type()) {
        case FieldDescriptor::TYPE_ENUM:
            ref->SetRepeatedEnumValue(msg, field, idx, value);
            break;
        case FieldDescriptor::TYPE_INT32:
        case FieldDescriptor::TYPE_SINT32:
        case FieldDescriptor::TYPE_SFIXED32:
            ref->SetRepeatedInt32(msg, field, idx, value);
            break;
        default:
            errmsg = "value type and field type do not match, field type is " + std::string(field->type_name());
            assert(("value type and field type do not match", false));
            break;
        }
    }

    void add(const Reflection* ref, Message* msg, const FieldDescriptor* field, int32_t value, std::string& errmsg)
    {
        switch (field->type()) {
        case FieldDescriptor::TYPE_ENUM:
            ref->AddEnumValue(msg, field, value);
            break;
        case FieldDescriptor::TYPE_INT32:
        case FieldDescriptor::TYPE_SINT32:
        case FieldDescriptor::TYPE_SFIXED32:
            ref->AddInt32(msg, field, value);
            break;
        default:
            errmsg = "value type and field type do not match, field type is " + std::string(field->type_name());
            assert(("value type and field type do not match", false));
            break;
        }
    }

    int32_t get(const Reflection* ref, Message* msg, const FieldDescriptor* field, std::string& errmsg)
    {
        int32_t ret = 0;
        switch (field->type()) {
        case FieldDescriptor::TYPE_ENUM:
            ret = ref->GetEnumValue(*msg, field);
            break;
        case FieldDescriptor::TYPE_INT32:
        case FieldDescriptor::TYPE_SINT32:
        case FieldDescriptor::TYPE_SFIXED32:
            ret = ref->GetInt32(*msg, field);
            break;
        default:
            errmsg = "value type and field type do not match, field type is " + std::string(field->type_name());
            assert(("value type and field type do not match", false));
            break;
        }
        return ret;
    }

    int32_t get(const Reflection* ref, Message* msg, const FieldDescriptor* field, int idx, std::string& errmsg)
    {
        int32_t ret = 0;
        switch (field->type()) {
        case FieldDescriptor::TYPE_ENUM:
            ret = ref->GetRepeatedEnumValue(*msg, field, idx);
            break;
        case FieldDescriptor::TYPE_INT32:
        case FieldDescriptor::TYPE_SINT32:
        case FieldDescriptor::TYPE_SFIXED32:
            ret = ref->GetRepeatedInt32(*msg, field, idx);
            break;
        default:
            errmsg = "value type and field type do not match, field type is " + std::string(field->type_name());
            assert(("value type and field type do not match", false));
            break;
        }

        return ret;
    }
};

template <>
struct field_oper_t<uint32_t> {
    void set(const Reflection* ref, Message* msg, const FieldDescriptor* field, uint32_t value, std::string& errmsg)
    {
        switch (field->type()) {
        case FieldDescriptor::TYPE_ENUM:
            ref->SetEnumValue(msg, field, int(value));
            break;
        case FieldDescriptor::TYPE_UINT32:
        case FieldDescriptor::TYPE_FIXED32:
            ref->SetUInt32(msg, field, value);
            break;
        default:
            errmsg = "value type and field type do not match, field type is " + std::string(field->type_name());
            assert(("value type and field type do not match", false));
            break;
        }
    }

    void set(const Reflection* ref, Message* msg, const FieldDescriptor* field, int idx, uint32_t value, std::string& errmsg)
    {
        switch (field->type()) {
        case FieldDescriptor::TYPE_ENUM:
            ref->SetRepeatedEnumValue(msg, field, idx, int(value));
            break;
        case FieldDescriptor::TYPE_UINT32:
        case FieldDescriptor::TYPE_FIXED32:

            ref->SetRepeatedUInt32(msg, field, idx, value);
            break;
        default:
            errmsg = "value type and field type do not match, field type is " + std::string(field->type_name());
            assert(("value type and field type do not match", false));
            break;
        }
    }

    void add(const Reflection* ref, Message* msg, const FieldDescriptor* field, uint32_t value, std::string& errmsg)
    {
        switch (field->type()) {
        case FieldDescriptor::TYPE_ENUM:
            ref->AddEnumValue(msg, field, int(value));
            break;
        case FieldDescriptor::TYPE_UINT32:
        case FieldDescriptor::TYPE_FIXED32:
            ref->AddUInt32(msg, field, value);
            break;
        default:
            errmsg = "value type and field type do not match, field type is " + std::string(field->type_name());
            assert(("value type and field type do not match", false));
            break;
        }
    }

    uint32_t get(const Reflection* ref, Message* msg, const FieldDescriptor* field, std::string& errmsg)
    {
        uint32_t ret = 0;
        switch (field->type()) {
        case FieldDescriptor::TYPE_ENUM:
            ret = uint32_t(ref->GetEnumValue(*msg, field));
            break;
        case FieldDescriptor::TYPE_UINT32:
        case FieldDescriptor::TYPE_FIXED32:
            ret = ref->GetUInt32(*msg, field);
            break;
        default:
            errmsg = "value type and field type do not match, field type is " + std::string(field->type_name());
            assert(("value type and field type do not match", false));
            break;
        }
        return ret;
    }

    uint32_t get(const Reflection* ref, Message* msg, const FieldDescriptor* field, int idx, std::string& errmsg)
    {
        uint32_t ret = 0;
        switch (field->type()) {
        case FieldDescriptor::TYPE_ENUM:
            ret = uint32_t(ref->GetRepeatedEnumValue(*msg, field, idx));
            break;
        case FieldDescriptor::TYPE_UINT32:
        case FieldDescriptor::TYPE_FIXED32:
            ret = ref->GetRepeatedUInt32(*msg, field, idx);
            break;
        default:
            errmsg = "value type and field type do not match, field type is " + std::string(field->type_name());
            assert(("value type and field type do not match", false));
            break;
        }
        return ret;
    }
};

template <>
struct field_oper_t<int64_t> {
    void set(const Reflection* ref, Message* msg, const FieldDescriptor* field, int64_t value, std::string& errmsg)
    {
        switch (field->type()) {
        case FieldDescriptor::TYPE_INT64:
        case FieldDescriptor::TYPE_SINT64:
        case FieldDescriptor::TYPE_SFIXED64:
            ref->SetInt64(msg, field, value);
            break;
        default:
            errmsg = "value type and field type do not match, field type is " + std::string(field->type_name());
            assert(("value type and field type do not match", false));
            break;
        }
    }

    void set(const Reflection* ref, Message* msg, const FieldDescriptor* field, int idx, int64_t value, std::string& errmsg)
    {
        switch (field->type()) {
        case FieldDescriptor::TYPE_INT64:
        case FieldDescriptor::TYPE_SINT64:
        case FieldDescriptor::TYPE_SFIXED64:
            ref->SetRepeatedInt64(msg, field, idx, value);
            break;
        default:
            errmsg = "value type and field type do not match, field type is " + std::string(field->type_name());
            assert(("value type and field type do not match", false));
            break;
        }
    }

    void add(const Reflection* ref, Message* msg, const FieldDescriptor* field, int64_t value, std::string& errmsg)
    {
        switch (field->type()) {
        case FieldDescriptor::TYPE_INT64:
        case FieldDescriptor::TYPE_SINT64:
        case FieldDescriptor::TYPE_SFIXED64:
            ref->AddInt64(msg, field, value);
            break;
        default:
            errmsg = "value type and field type do not match, field type is " + std::string(field->type_name());
            assert(("value type and field type do not match", false));
            break;
        }
    }

    int64_t get(const Reflection* ref, Message* msg, const FieldDescriptor* field, std::string& errmsg)
    {
        int64_t ret = 0;
        switch (field->type()) {
        case FieldDescriptor::TYPE_INT64:
        case FieldDescriptor::TYPE_SINT64:
        case FieldDescriptor::TYPE_SFIXED64:
            ret = ref->GetInt64(*msg, field);
            break;
        default:
            errmsg = "value type and field type do not match, field type is " + std::string(field->type_name());
            assert(("value type and field type do not match", false));
            break;
        }
        return ret;
    }

    int64_t get(const Reflection* ref, Message* msg, const FieldDescriptor* field, int idx, std::string& errmsg)
    {
        int64_t ret = 0;
        switch (field->type()) {
        case FieldDescriptor::TYPE_INT64:
        case FieldDescriptor::TYPE_SINT64:
        case FieldDescriptor::TYPE_SFIXED64:
            ret = ref->GetRepeatedInt64(*msg, field, idx);
            break;
        default:
            errmsg = "value type and field type do not match, field type is " + std::string(field->type_name());
            assert(("value type and field type do not match", false));
            break;
        }
        return ret;
    }
};

template <>
struct field_oper_t<uint64_t> {
    void set(const Reflection* ref, Message* msg, const FieldDescriptor* field, uint64_t value, std::string& errmsg)
    {
        switch (field->type()) {
        case FieldDescriptor::TYPE_UINT64:
        case FieldDescriptor::TYPE_FIXED64:
            ref->SetUInt64(msg, field, value);
            break;
        default:
            errmsg = "value type and field type do not match, field type is " + std::string(field->type_name());
            assert(("value type and field type do not match", false));
            break;
        }
    }

    void set(const Reflection* ref, Message* msg, const FieldDescriptor* field, int idx, uint64_t value, std::string& errmsg)
    {
        switch (field->type()) {
        case FieldDescriptor::TYPE_UINT64:
        case FieldDescriptor::TYPE_FIXED64:
            ref->SetRepeatedUInt64(msg, field, idx, value);
            break;
        default:
            errmsg = "value type and field type do not match, field type is " + std::string(field->type_name());
            assert(("value type and field type do not match", false));
            break;
        }
    }

    void add(const Reflection* ref, Message* msg, const FieldDescriptor* field, uint64_t value, std::string& errmsg)
    {
        switch (field->type()) {
        case FieldDescriptor::TYPE_UINT64:
        case FieldDescriptor::TYPE_FIXED64:
            ref->AddUInt64(msg, field, value);
            break;
        default:
            errmsg = "value type and field type do not match, field type is " + std::string(field->type_name());
            assert(("value type and field type do not match", false));
            break;
        }
    }

    uint64_t get(const Reflection* ref, Message* msg, const FieldDescriptor* field, std::string& errmsg)
    {
        uint64_t ret = 0;
        switch (field->type()) {
        case FieldDescriptor::TYPE_UINT64:
        case FieldDescriptor::TYPE_FIXED64:
            ret = ref->GetUInt64(*msg, field);
            break;
        default:
            errmsg = "value type and field type do not match, field type is " + std::string(field->type_name());
            assert(("value type and field type do not match", false));
            break;
        }
        return ret;
    }

    uint64_t get(const Reflection* ref, Message* msg, const FieldDescriptor* field, int idx, std::string& errmsg)
    {
        uint64_t ret = 0;
        switch (field->type()) {
        case FieldDescriptor::TYPE_UINT64:
        case FieldDescriptor::TYPE_FIXED64:
            ret = ref->GetRepeatedUInt64(*msg, field, idx);
            break;
        default:
            errmsg = "value type and field type do not match, field type is " + std::string(field->type_name());
            assert(("value type and field type do not match", false));
            break;
        }
        return ret;
    }
};

template <>
struct field_oper_t<float> {
    void set(const Reflection* ref, Message* msg, const FieldDescriptor* field, float value, std::string& errmsg)
    {
        switch (field->type()) {
        case FieldDescriptor::TYPE_FLOAT:
            ref->SetFloat(msg, field, value);
            break;
        default:
            errmsg = "value type and field type do not match, field type is " + std::string(field->type_name());
            assert(("value type and field type do not match", false));
            break;
        }
    }

    void set(const Reflection* ref, Message* msg, const FieldDescriptor* field, int idx, float value, std::string& errmsg)
    {
        switch (field->type()) {
        case FieldDescriptor::TYPE_FLOAT:
            ref->SetRepeatedFloat(msg, field, idx, value);
            break;
        default:
            errmsg = "value type and field type do not match, field type is " + std::string(field->type_name());
            assert(("value type and field type do not match", false));
            break;
        }
    }

    void add(const Reflection* ref, Message* msg, const FieldDescriptor* field, float value, std::string& errmsg)
    {
        switch (field->type()) {
        case FieldDescriptor::TYPE_FLOAT:
            ref->AddFloat(msg, field, value);
            break;
        default:
            errmsg = "value type and field type do not match, field type is " + std::string(field->type_name());
            assert(("value type and field type do not match", false));
            break;
        }
    }

    float get(const Reflection* ref, Message* msg, const FieldDescriptor* field, std::string& errmsg)
    {
        float ret = 0;
        switch (field->type()) {
        case FieldDescriptor::TYPE_FLOAT:
            ret = ref->GetFloat(*msg, field);
            break;
        default:
            errmsg = "value type and field type do not match, field type is " + std::string(field->type_name());
            assert(("value type and field type do not match", false));
            break;
        }
        return ret;
    }

    float get(const Reflection* ref, Message* msg, const FieldDescriptor* field, int idx, std::string& errmsg)
    {
        float ret = 0;
        switch (field->type()) {
        case FieldDescriptor::TYPE_FLOAT:

            ret = ref->GetRepeatedFloat(*msg, field, idx);
            break;
        default:
            errmsg = "value type and field type do not match, field type is " + std::string(field->type_name());
            assert(("value type and field type do not match", false));
            break;
        }
        return ret;
    }
};

template <>
struct field_oper_t<double> {
    void set(const Reflection* ref, Message* msg, const FieldDescriptor* field, double value, std::string& errmsg)
    {
        switch (field->type()) {
        case FieldDescriptor::TYPE_DOUBLE:
            ref->SetDouble(msg, field, value);
            break;
        default:
            errmsg = "value type and field type do not match, field type is " + std::string(field->type_name());
            assert(("value type and field type do not match", false));
            break;
        }
    }

    void set(const Reflection* ref, Message* msg, const FieldDescriptor* field, int idx, double value, std::string& errmsg)
    {
        switch (field->type()) {
        case FieldDescriptor::TYPE_DOUBLE:
            ref->SetRepeatedDouble(msg, field, idx, value);
            break;
        default:
            errmsg = "value type and field type do not match, field type is " + std::string(field->type_name());
            assert(("value type and field type do not match", false));
            break;
        }
    }

    void add(const Reflection* ref, Message* msg, const FieldDescriptor* field, double value, std::string& errmsg)
    {
        switch (field->type()) {
        case FieldDescriptor::TYPE_DOUBLE:
            ref->AddDouble(msg, field, value);
            break;
        default:
            errmsg = "value type and field type do not match, field type is " + std::string(field->type_name());
            assert(("value type and field type do not match", false));
            break;
        }
    }

    double get(const Reflection* ref, Message* msg, const FieldDescriptor* field, std::string& errmsg)
    {
        double ret = 0;
        switch (field->type()) {
        case FieldDescriptor::TYPE_DOUBLE:
            ret = ref->GetDouble(*msg, field);
            break;
        default:
            errmsg = "value type and field type do not match, field type is " + std::string(field->type_name());
            assert(("value type and field type do not match", false));
            break;
        }
        return ret;
    }

    double get(const Reflection* ref, Message* msg, const FieldDescriptor* field, int idx, std::string& errmsg)
    {
        double ret = 0;
        switch (field->type()) {
        case FieldDescriptor::TYPE_DOUBLE:

            ret = ref->GetRepeatedDouble(*msg, field, idx);
            break;
        default:
            errmsg = "value type and field type do not match, field type is " + std::string(field->type_name());
            assert(("value type and field type do not match", false));
            break;
        }
        return ret;
    }
};

template <>
struct field_oper_t<bool> {
    void set(const Reflection* ref, Message* msg, const FieldDescriptor* field, bool value, std::string& errmsg)
    {
        switch (field->type()) {
        case FieldDescriptor::TYPE_BOOL:
            ref->SetBool(msg, field, value);
            break;
        default:
            errmsg = "value type and field type do not match, field type is " + std::string(field->type_name());
            assert(("value type and field type do not match", false));
            break;
        }
    }

    void set(const Reflection* ref, Message* msg, const FieldDescriptor* field, int idx, bool value, std::string& errmsg)
    {
        switch (field->type()) {
        case FieldDescriptor::TYPE_BOOL:
            ref->SetRepeatedBool(msg, field, idx, value);
            break;
        default:
            errmsg = "value type and field type do not match, field type is " + std::string(field->type_name());
            assert(("value type and field type do not match", false));
            break;
        }
    }

    void add(const Reflection* ref, Message* msg, const FieldDescriptor* field, bool value, std::string& errmsg)
    {
        switch (field->type()) {
        case FieldDescriptor::TYPE_BOOL:
            ref->AddBool(msg, field, value);
            break;
        default:
            errmsg = "value type and field type do not match, field type is " + std::string(field->type_name());
            assert(("value type and field type do not match", false));
            break;
        }
    }

    bool get(const Reflection* ref, Message* msg, const FieldDescriptor* field, std::string& errmsg)
    {
        bool ret = false;
        switch (field->type()) {
        case FieldDescriptor::TYPE_BOOL:
            ret = ref->GetBool(*msg, field);
            break;
        default:
            errmsg = "value type and field type do not match, field type is " + std::string(field->type_name());
            assert(("value type and field type do not match", false));
            break;
        }
        return ret;
    }

    bool get(const Reflection* ref, Message* msg, const FieldDescriptor* field, int idx, std::string& errmsg)
    {
        bool ret = false;
        switch (field->type()) {
        case FieldDescriptor::TYPE_BOOL:
            ret = ref->GetRepeatedBool(*msg, field, idx);
            break;
        default:
            errmsg = "value type and field type do not match, field type is " + std::string(field->type_name());
            assert(("value type and field type do not match", false));
            break;
        }
        return ret;
    }
};

template <>
struct field_oper_t<std::string> {
    void set(const Reflection* ref, Message* msg, const FieldDescriptor* field, std::string value, std::string& errmsg)
    {
        switch (field->type()) {
        case FieldDescriptor::TYPE_STRING:
        case FieldDescriptor::TYPE_BYTES:
            ref->SetString(msg, field, value);
            break;
        default:
            errmsg = "value type and field type do not match, field type is " + std::string(field->type_name());
            assert(("value type and field type do not match", false));
            break;
        }
    }

    void set(const Reflection* ref, Message* msg, const FieldDescriptor* field, int idx, std::string value, std::string& errmsg)
    {
        switch (field->type()) {
        case FieldDescriptor::TYPE_STRING:
        case FieldDescriptor::TYPE_BYTES:
            ref->SetRepeatedString(msg, field, idx, value);
            break;
        default:
            errmsg = "value type and field type do not match, field type is " + std::string(field->type_name());
            assert(("value type and field type do not match", false));
            break;
        }
    }

    void add(const Reflection* ref, Message* msg, const FieldDescriptor* field, std::string value, std::string& errmsg)
    {
        switch (field->type()) {
        case FieldDescriptor::TYPE_STRING:
        case FieldDescriptor::TYPE_BYTES:
            ref->AddString(msg, field, value);
            break;
        default:
            errmsg = "value type and field type do not match, field type is " + std::string(field->type_name());
            assert(("value type and field type do not match", false));
            break;
        }
    }

    std::string get(const Reflection* ref, Message* msg, const FieldDescriptor* field, std::string& errmsg)
    {
        std::string ret;
        switch (field->type()) {
        case FieldDescriptor::TYPE_STRING:
        case FieldDescriptor::TYPE_BYTES:
            ret = ref->GetString(*msg, field);
            break;
        default:
            errmsg = "value type and field type do not match, field type is " + std::string(field->type_name());
            assert(("value type and field type do not match", false));
            break;
        }
        return ret;
    }

    std::string get(const Reflection* ref, Message* msg, const FieldDescriptor* field, int idx, std::string& errmsg)
    {
        std::string ret;
        switch (field->type()) {
        case FieldDescriptor::TYPE_STRING:
        case FieldDescriptor::TYPE_BYTES:
            ret = ref->GetRepeatedString(*msg, field, idx);
            break;
        default:
            errmsg = "value type and field type do not match, field type is " + std::string(field->type_name());
            assert(("value type and field type do not match", false));
            break;
        }
        return ret;
    }
};

//////////////////////
template <>
struct field_oper_t<Message*> {
    void set(const Reflection* ref, Message* msg, const FieldDescriptor* field, Message* value, std::string& errmsg)
    {
        Message* new_val = value->New();
        new_val->CopyFrom(*value);
        switch (field->type()) {
        case FieldDescriptor::TYPE_MESSAGE:
            ref->SetAllocatedMessage(msg, new_val, field);
            break;
        default:
            errmsg = "value type and field type do not match, field type is " + std::string(field->type_name());
            assert(("value type and field type do not match", false));
            break;
        }
    }

    void set(const Reflection* ref, Message* msg, const FieldDescriptor* field, int idx, Message* value, std::string& errmsg)
    {
        Message* new_val = value->New();
        new_val->CopyFrom(*value);
        switch (field->type()) {
        case FieldDescriptor::TYPE_MESSAGE:
            ref->MutableRepeatedMessage(msg, field, idx)->CopyFrom(*new_val);
            break;
        default:
            errmsg = "value type and field type do not match, field type is " + std::string(field->type_name());
            assert(("value type and field type do not match", false));
            break;
        }
    }

    void add(const Reflection* ref, Message* msg, const FieldDescriptor* field, Message* value, std::string& errmsg)
    {
        Message* new_val = value->New();
        new_val->CopyFrom(*value);
        switch (field->type()) {
        case FieldDescriptor::TYPE_MESSAGE:
            ref->AddAllocatedMessage(msg, field, new_val);
            break;
        default:
            errmsg = "value type and field type do not match, field type is " + std::string(field->type_name());
            assert(("value type and field type do not match", false));
            break;
        }
    }

    Message* get(const Reflection* ref, Message* msg, const FieldDescriptor* field, std::string& errmsg)
    {
        Message* src = nullptr;
        Message* ret = nullptr;
        switch (field->type()) {
        case FieldDescriptor::TYPE_MESSAGE:
            src = ref->MutableMessage(msg, field);
            break;
        default:
            errmsg = "value type and field type do not match, field type is " + std::string(field->type_name());
            assert(("value type and field type do not match", false));
            break;
        }

        if (src != nullptr) {
            ret = src->New();
            ret->CopyFrom(*src);
        }

        return ret;
    }

    Message* get(const Reflection* ref, Message* msg, const FieldDescriptor* field, int idx, std::string& errmsg)
    {
        Message* src = nullptr;
        Message* ret = nullptr;
        switch (field->type()) {
        case FieldDescriptor::TYPE_MESSAGE:
            src = ref->MutableRepeatedMessage(msg, field, idx);
            break;
        default:
            errmsg = "value type and field type do not match, field type is " + std::string(field->type_name());
            assert(("value type and field type do not match", false));
            break;
        }

        if (src != nullptr) {
            ret = src->New();
            ret->CopyFrom(*src);
        }

        return ret;
    }
};

template <>
struct field_oper_t<pbmsg_t*> {
    void set(const Reflection* ref, Message* msg, const FieldDescriptor* field, pbmsg_t* value, std::string& errmsg)
    {
        switch (field->type()) {
        case FieldDescriptor::TYPE_MESSAGE:
            ref->SetAllocatedMessage(msg, value->get_msg(true), field);
            break;
        default:
            errmsg = "value type and field type do not match, field type is " + std::string(field->type_name());
            assert(("value type and field type do not match", false));
            break;
        }
    }

    void set(const Reflection* ref, Message* msg, const FieldDescriptor* field, int idx, pbmsg_t* value, std::string& errmsg)
    {
        switch (field->type()) {
        case FieldDescriptor::TYPE_MESSAGE:
            ref->MutableRepeatedMessage(msg, field, idx)->CopyFrom(*value->get_msg(true));
            break;
        default:
            errmsg = "value type and field type do not match, field type is " + std::string(field->type_name());
            assert(("value type and field type do not match", false));
            break;
        }
    }

    void add(const Reflection* ref, Message* msg, const FieldDescriptor* field, pbmsg_t* value, std::string& errmsg)
    {
        switch (field->type()) {
        case FieldDescriptor::TYPE_MESSAGE:
            ref->AddAllocatedMessage(msg, field, value->get_msg(true));
            break;
        default:
            errmsg = "value type and field type do not match, field type is " + std::string(field->type_name());
            assert(("value type and field type do not match", false));
            break;
        }
    }

    pbmsg_t* get(const Reflection* ref, Message* msg, const FieldDescriptor* field, std::string& errmsg)
    {
        pbmsg_t* ret = nullptr;
        switch (field->type()) {
        case FieldDescriptor::TYPE_MESSAGE:
            ret = pbmsg_t::create(ref->MutableMessage(msg, field));
            break;
        default:
            errmsg = "value type and field type do not match, field type is " + std::string(field->type_name());
            assert(("value type and field type do not match", false));
            break;
        }
        return ret;
    }

    pbmsg_t* get(const Reflection* ref, Message* msg, const FieldDescriptor* field, int idx, std::string& errmsg)
    {
        pbmsg_t* ret = nullptr;
        switch (field->type()) {
        case FieldDescriptor::TYPE_MESSAGE:
            ret = pbmsg_t::create(ref->MutableRepeatedMessage(msg, field, idx));
            break;
        default:
            errmsg = "value type and field type do not match, field type is " + std::string(field->type_name());
            assert(("value type and field type do not match", false));
            break;
        }
        return ret;
    }
};

#pragma GCC diagnostic pop