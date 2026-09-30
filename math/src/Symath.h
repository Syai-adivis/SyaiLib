#ifndef CCNSL_MATH
#define CCNSL_MATH
#include <iostream>
#include <string>
#include <cmath>
#include <limits>
#include <iomanip>
namespace SyLib
{

    class MathUtils
    {
    public:
        // 新型累乘器
        template <typename T>
        T fac(T c1)
        {
            T c = c1;
            if (c <= 0 || std::fmod(c, 1) != 0)
            {
                std::cerr << "[Error] Factorial requires a positive integer!" << std::endl;
                return std::numeric_limits<T>::quiet_NaN();
            }
            else
            {
                T s = 1;
                for (int i = 2; i <= c; i++)
                {
                    s *= i;
                }
                return s;
            }
        }

        // 单位转换函数
        template <typename T>
        T kmToMile(T km) { return km / 1.60634; }
        template <typename T>
        T kmToNm(T km) { return km / 1.852; }
        template <typename T>
        T mileToKm(T mile) { return mile * 1.60634; }
        template <typename T>
        T nMileToKm(T nm) { return nm * 1.852; }
        template <typename T>
        T cToF(T c) { return c * 1.8 + 32; }
        template <typename T>
        T fToC(T f) { return (f - 32) / 1.8; }
        void mult(int mua)
        {
            for (int i = 1; i <= mua; i++)
            {
                for (int j = 1; j <= i; j++)
                {
                    std::cout << i << "*" << j << "=" << i * j;
                    if (i * j < 10)
                        std::cout << "  ";
                    else
                        std::cout << " ";
                }
                std::cout << std::endl;
            }
        }
    } mt;
}
#endif
