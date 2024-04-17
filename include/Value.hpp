#include <iostream>
#include <cstdint>
#include <cstring>
#include <boost/variant2/variant.hpp>
#include <string>
#include <utility>
#include <vector>

namespace tcp {

enum class ValueType{
    INT32 = 0,
    INT64,
    STRING,
    STRING_LIST,
};
//meta: n(8byte):[opt + data](0), [opt + data](1)
//vector: [n(4byte) + s_0\0 + s_1\0 + ... + s_n\0]
class Value {
  private:
    boost::variant2::variant<
        int32_t,
        int64_t, 
        std::string,
        std::vector<std::string> > value_;
  public:
    template<typename T>
    void set(T&& v) {
        value_ = std::forward<T>(v);
    }
    template<typename T>
    T& get() {
        return boost::variant2::get<T>(value_);
    }
    void output() {
        auto type = (ValueType)value_.index();
        if(type == ValueType::INT32) {
            std::cout << boost::variant2::get<int32_t>(value_) << std::endl;
        } else if(type == ValueType::INT64) {
            std::cout << boost::variant2::get<int64_t>(value_) << std::endl;
        } else if(type == ValueType::STRING) {
            auto& value = boost::variant2::get<std::string>(value_);
            std::cout << value << std::endl;
        } else if(type == ValueType::STRING_LIST) {
            auto& vec = boost::variant2::get<std::vector<std::string>>(value_);
            for(int i = 0; i < vec.size(); i++) {
                auto& value = vec[i];
                std::cout << value << std::endl;
            }
        }
    }
    size_t size() {
        auto type = (ValueType)value_.index();
        if(type == ValueType::INT32) {
            return 12;
        } else if(type == ValueType::INT64) {
            return 16;
        } else if(type == ValueType::STRING) {
            auto& value = boost::variant2::get<std::string>(value_);
            return 8 + value.size();
        } else if(type == ValueType::STRING_LIST) {
            auto& vec = boost::variant2::get<std::vector<std::string>>(value_);
            int need = 12;
            for(int i = 0; i < vec.size(); i++) {
                need += vec[i].size() + 1;
            }
            return need;
        }
        return 0;
    }
    void parseFrom(const char* data, size_t len) {
        int64_t opt;
        std::memcpy(&opt, data, sizeof(opt));
        int32_t x32;
        int64_t x64;
        int32_t n;
        switch ((ValueType)opt) {
            case ValueType::INT32:
                std::memcpy(&x32, data + 8, sizeof(x32));
                value_ = x32;
                break;
            case ValueType::INT64:
                std::memcpy(&x64, data + 8, sizeof(x64));
                value_ = x64;
                break;
            case ValueType::STRING:
                value_ = std::string(data + 8, len - 8);
                break;
            case ValueType::STRING_LIST:
                std::memcpy(&n, data + 8, sizeof(n));
                std::vector<std::string> v;
                v.reserve(n);
                int lst = 12;
                for(int i = 0; i < n; i++) {
                    int len = strlen(data + lst);
                    std::cout << "i: " << i << " len: " << len << std::endl;
                    v.push_back(std::string(data + lst, len));
                    lst += len + 1;
                }
                value_ = std::move(v);
                break;
        }
    }
    bool parseTo(char* data, int len) {
        // *((int64_t*)data) = (int64_t)value_.index();
        int64_t opt = value_.index();
        std::memcpy(data, &opt, sizeof(opt));
        int need, lst;
        auto type = (ValueType)value_.index();
        if(type == ValueType::INT32) {
            if (len < 12) return false;
            int32_t& x32 = boost::variant2::get<int32_t>(value_);
            std::memcpy(data + 8, &x32, sizeof(x32));
        } else if(type == ValueType::INT64) {
            if (len < 16) return false;
            int64_t& x64 = boost::variant2::get<int64_t>(value_);
            std::memcpy(data + 8, &x64, sizeof(x64));
        } else if(type == ValueType::STRING) {
            auto& value = boost::variant2::get<std::string>(value_);
            if (len < 8 + value.size()) return false;
            for(int i = 0; i < value.size(); i++) {
                data[8 + i] = value[i];
            }
        } else if(type == ValueType::STRING_LIST) {
            auto& vec = boost::variant2::get<std::vector<std::string>>(value_);
            need = 12;
            for(int i = 0; i < vec.size(); i++) {
                need += vec[i].size() + 1;
            }
            if (len < need) return false;
            int32_t x = vec.size();
            std::memcpy(data + 8, &x, sizeof(x));
            // *((int32_t*)(data + 8)) = int32_t(vec.size());
            int lst = 12;
            for(int i = 0; i < vec.size(); i++) {
                auto& value = vec[i];
                for(int i = 0; i < value.size(); i++) {
                    data[lst + i] = value[i];
                }
                data[lst + value.size()] = 0;
                lst += value.size() + 1;
            }
        }
        return true;
    }
};

}