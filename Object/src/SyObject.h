#ifndef JUOBJ_H
#define JUOBJ_H
#include <optional>
#include <stdexcept>
namespace SyLib
{
#define nullobj nullptr
    template <typename T>
    class Object
    {
    private:
        std::optional<T> va;

    public:
        Object(T p = T()) { va = p; }
        // getter
        T getOrthrow()
        {
            if (!va.has_value())
            {
                throw std::runtime_error("Object is null, no value available!");
            }
            return va.value();
        }
        // setter
        void set(T p = T()) { va = p; }
        // 删除对象
        void del() { va.reset(); }
        // 是否为null
        bool isNull() { return !va.has_value(); }
        // 是否有值
        bool isSome() { return va.has_value(); }
    };
}
#endif