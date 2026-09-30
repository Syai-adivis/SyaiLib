#ifndef REALNUM_H
#define REALNUM_H

#include <iostream>
#include <string>
#include <algorithm>
#include <cmath>
#include <cctype>
#include <vector>
#include <cstdint>
#include <stdexcept>
#include <cstring>

namespace SyLib
{

    class Realnum
    {
    private:
        bool is_negative;                         // true负数 false正数
        std::vector<std::uint8_t> integer_digits; // 整数部分，高位在前，存数字0~9
        std::vector<std::uint8_t> decimal_digits; // 小数部分，[0]是小数点后第一位
        size_t max_decimal;
        // ========= 内部辅助工具函数 =========
        // 去除整数部分前导零；空向量变成 {0}
        void trim_integer_leading(std::vector<std::uint8_t> &vec) const
        {
            auto it = vec.begin();
            while (it != vec.end() && *it == 0)
            {
                ++it;
            }
            if (it == vec.end())
            {
                vec.assign({0});
            }
            else
            {
                vec.assign(it, vec.end());
            }
        }

        // 去除小数部分尾部零；全部为零则清空vector
        void trim_decimal_trailing(std::vector<std::uint8_t> &vec) const
        {
            while (!vec.empty() && vec.back() == 0)
            {
                vec.pop_back();
            }
        }

        // 标准化：修剪零、处理负零
        void normalize()
        {
            trim_integer_leading(integer_digits);
            trim_decimal_trailing(decimal_digits);
            // -0.0 强制转为正0
            if (integer_digits.size() == 1 && integer_digits[0] == 0 && decimal_digits.empty())
            {
                is_negative = false;
            }
        }

        // 无符号加法，两个数字向量(高位在前)返回相加结果高位在前
        std::vector<std::uint8_t> add_unsigned(const std::vector<std::uint8_t> &a, const std::vector<std::uint8_t> &b) const
        {
            std::vector<std::uint8_t> res;
            int carry = 0;
            int i = static_cast<int>(a.size()) - 1;
            int j = static_cast<int>(b.size()) - 1;
            while (i >= 0 || j >= 0 || carry > 0)
            {
                int da = (i >= 0) ? static_cast<int>(a[i--]) : 0;
                int db = (j >= 0) ? static_cast<int>(b[j--]) : 0;
                int sum = da + db + carry;
                carry = sum / 10;
                res.push_back(static_cast<std::uint8_t>(sum % 10));
            }
            std::reverse(res.begin(), res.end());
            trim_integer_leading(res);
            return res;
        }

        // 无符号减法 a >= b，a-b 返回高位在前
        std::vector<std::uint8_t> sub_unsigned(const std::vector<std::uint8_t> &a, const std::vector<std::uint8_t> &b) const
        {
            std::vector<std::uint8_t> res;
            int borrow = 0;
            int i = static_cast<int>(a.size()) - 1;
            int j = static_cast<int>(b.size()) - 1;
            while (i >= 0 || j >= 0)
            {
                int da = (i >= 0) ? static_cast<int>(a[i--]) : 0;
                int db = (j >= 0) ? static_cast<int>(b[j--]) : 0;
                da -= borrow;
                borrow = 0;
                if (da < db)
                {
                    da += 10;
                    borrow = 1;
                }
                res.push_back(static_cast<std::uint8_t>(da - db));
            }
            std::reverse(res.begin(), res.end());
            trim_integer_leading(res);
            return res;
        }

        // 无符号比较 a>b 返回true，高位在前向量
        bool cmp_unsigned_gt(const std::vector<std::uint8_t> &a, const std::vector<std::uint8_t> &b) const
        {
            if (a.size() != b.size())
                return a.size() > b.size();
            return a > b;
        }
        bool cmp_unsigned_ge(const std::vector<std::uint8_t> &a, const std::vector<std::uint8_t> &b) const
        {
            if (a.size() != b.size())
                return a.size() > b.size();
            return a >= b;
        }

        // 无符号乘法 a*b，高位在前
        std::vector<std::uint8_t> mul_unsigned(const std::vector<std::uint8_t> &a, const std::vector<std::uint8_t> &b) const
        {
            if ((a.size() == 1 && a[0] == 0) || (b.size() == 1 && b[0] == 0))
            {
                return {0};
            }
            std::vector<std::uint8_t> res(a.size() + b.size(), 0U);
            for (int i = static_cast<int>(a.size()) - 1; i >= 0; --i)
            {
                int digitA = static_cast<int>(a[i]);
                int carry = 0;
                for (int j = static_cast<int>(b.size()) - 1; j >= 0; --j)
                {
                    int digitB = static_cast<int>(b[j]);
                    int product = digitA * digitB + static_cast<int>(res[i + j + 1]) + carry;
                    carry = product / 10;
                    res[i + j + 1] = static_cast<std::uint8_t>(product % 10);
                }
                res[i] = static_cast<std::uint8_t>(static_cast<int>(res[i]) + carry);
            }
            trim_integer_leading(res);
            return res;
        }

        // 无符号长除法：被除数 / 除数；返回 {商整数部分，商小数部分}；divisor不能为0
        // max_decimal：最多生成多少位小数
        std::pair<std::vector<std::uint8_t>, std::vector<std::uint8_t>>
        div_unsigned(const std::vector<std::uint8_t> &dividend,
                     const std::vector<std::uint8_t> &divisor,
                     size_t max_decimal = 50) const
        {
            if (divisor.size() == 1 && divisor[0] == 0)
            {
                throw std::domain_error("division by zero");
            }
            std::vector<std::uint8_t> int_quot;
            std::vector<std::uint8_t> remainder;

            // 整数部分长除
            for (auto d : dividend)
            {
                remainder.push_back(d);
                trim_integer_leading(remainder);
                uint8_t q_digit = 0;
                while (cmp_unsigned_ge(remainder, divisor))
                {
                    remainder = sub_unsigned(remainder, divisor);
                    q_digit++;
                }
                int_quot.push_back(q_digit);
            }
            trim_integer_leading(int_quot);

            // 小数部分计算
            std::vector<std::uint8_t> dec_quot;
            for (size_t i = 0; i < max_decimal; i++)
            {
                if (remainder.size() == 1 && remainder[0] == 0)
                    break;
                remainder.push_back(0);
                trim_integer_leading(remainder);
                uint8_t q_digit = 0;
                while (cmp_unsigned_ge(remainder, divisor))
                {
                    remainder = sub_unsigned(remainder, divisor);
                    q_digit++;
                }
                dec_quot.push_back(q_digit);
            }
            trim_decimal_trailing(dec_quot);
            return {int_quot, dec_quot};
        }

        // 把 Realnum 的整数+小数拼接成完整无符号数字向量，用于乘法
        std::vector<std::uint8_t> combine_all_digits() const
        {
            std::vector<std::uint8_t> out = integer_digits;
            out.insert(out.end(), decimal_digits.begin(), decimal_digits.end());
            return out;
        }

        // 字符串解析辅助：把纯数字字符串转为数字向量（高位在前）
        std::vector<std::uint8_t> str_to_digits(const std::string &s) const
        {
            std::vector<std::uint8_t> v;
            for (char ch : s)
            {
                v.push_back(static_cast<std::uint8_t>(ch - '0'));
            }
            trim_integer_leading(v);
            return v;
        }

        // 数字向量转字符串
        std::string digits_to_str(const std::vector<std::uint8_t> &vec) const
        {
            std::string s;
            for (auto d : vec)
            {
                s += static_cast<char>('0' + d);
            }
            return s;
        }

    public:
        // ========== 构造函数 ==========
        Realnum() : is_negative(false), integer_digits{0}, decimal_digits{}, max_decimal{50} {}

        /**
         * @brief 字节数组构造函数
         * @param int_digits 整数数字数组，高位在前，元素0~9
         * @param int_len 整数数组长度
         * @param dec_digits 小数数字数组，第0个是小数点后第一位，可为nullptr
         * @param dec_len 小数数组长度
         * @param negative 是否负数
         * @param max_decimal 最大乘除精度
         */
        Realnum(const std::uint8_t int_digits[], size_t int_len,
                const std::uint8_t dec_digits[] = nullptr, size_t dec_len = 0,
                bool negative = false, size_t max_decima = 50)
        {
            is_negative = negative;
            integer_digits.assign(int_digits, int_digits + int_len);
            if (dec_digits != nullptr && dec_len > 0)
            {
                decimal_digits.assign(dec_digits, dec_digits + dec_len);
            }
            else
            {
                decimal_digits.clear();
            }
            max_decimal = max_decima;
            normalize();
        }

        // int 构造
        Realnum(int num, size_t max_decima = 50)
        {
            is_negative = (num < 0);
            long long val = static_cast<long long>(num < 0 ? -num : num);
            integer_digits = str_to_digits(std::to_string(val));
            decimal_digits.clear();
            max_decimal = max_decima;
            normalize();
        }

        // long long 构造
        Realnum(long long num, size_t max_decima = 50)
        {
            is_negative = (num < 0);
            unsigned long long val = (num < 0) ? static_cast<unsigned long long>(-num) : static_cast<unsigned long long>(num);
            integer_digits = str_to_digits(std::to_string(val));
            decimal_digits.clear();
            max_decimal = max_decima;

            normalize();
        }

        // float 构造
        Realnum(float num, size_t max_decima = 50)
        {
            max_decimal = max_decima;
            *this = Realnum(static_cast<long double>(num));
        }

        // 字符串构造 "-123.456" / "789" / "0.001"
        Realnum(const std::string &s, size_t max_decima = 50)
        {
            std::string str = s;
            is_negative = false;
            size_t pos = 0;
            if (!str.empty() && str[0] == '-')
            {
                is_negative = true;
                pos = 1;
            }
            else if (!str.empty() && str[0] == '+')
            {
                pos = 1;
            }
            size_t dot = str.find('.', pos);
            std::string intStr, decStr;
            if (dot == std::string::npos)
            {
                intStr = str.substr(pos);
                decStr = "";
            }
            else
            {
                intStr = str.substr(pos, dot - pos);
                decStr = str.substr(dot + 1);
            }
            if (intStr.empty())
                intStr = "0";
            integer_digits = str_to_digits(intStr);
            decimal_digits.clear();
            for (char ch : decStr)
            {
                decimal_digits.push_back(static_cast<std::uint8_t>(ch - '0'));
            }
            max_decimal = max_decima;
            normalize();
        }

        // long double 构造
        Realnum(long double num, size_t max_decima = 50)
        {
            is_negative = (num < 0);
            num = std::fabsl(num);
            long long intPart = static_cast<long long>(num);
            long double decPart = num - static_cast<long double>(intPart);
            integer_digits = str_to_digits(std::to_string(intPart));
            decimal_digits.clear();
            const int MAX_DEC = 15;
            for (int i = 0; i < MAX_DEC; ++i)
            {
                decPart *= 10.0L;
                int d = static_cast<int>(decPart);
                decimal_digits.push_back(static_cast<std::uint8_t>(d));
                decPart -= static_cast<long double>(d);
                if (decPart < 1e-15L)
                    break;
            }
            max_decimal = max_decima;
            normalize();
        }

        // ========= 转换为字符串 =========
        std::string to_string() const
        {
            std::string res;
            if (is_negative)
                res += '-';
            res += digits_to_str(integer_digits);
            if (!decimal_digits.empty())
            {
                res += '.';
                res += digits_to_str(decimal_digits);
            }
            return res;
        }

        Realnum abs() const
        {
            Realnum ret = *this;
            ret.is_negative = false;
            return ret;
        }

        // 非负整数次幂
        Realnum pow(int exponent) const
        {
            if (exponent < 0)
                throw std::invalid_argument("Exponent must be non‑negative");
            if (exponent == 0)
                return Realnum("1");
            Realnum result = *this;
            for (int i = 1; i < exponent; ++i)
            {
                result = result * *this;
            }
            return result;
        }

        // ========= 二元算术运算符 =========
        Realnum operator+(const Realnum &other) const
        {
            Realnum res;
            if (is_negative == other.is_negative)
            {
                res.is_negative = is_negative;
                // 对齐小数位数
                auto decA = decimal_digits;
                auto decB = other.decimal_digits;
                size_t maxDec = std::max(decA.size(), decB.size());
                decA.resize(maxDec, 0U);
                decB.resize(maxDec, 0U);

                // 小数相加
                auto decSum = add_unsigned(decA, decB);
                std::vector<std::uint8_t> carry{0};
                if (decSum.size() > maxDec)
                {
                    size_t carryLen = decSum.size() - maxDec;
                    carry.assign(decSum.begin(), decSum.begin() + static_cast<ptrdiff_t>(carryLen));
                    decSum.assign(decSum.begin() + static_cast<ptrdiff_t>(carryLen), decSum.end());
                }
                auto intSum = add_unsigned(add_unsigned(integer_digits, other.integer_digits), carry);
                res.integer_digits = intSum;
                res.decimal_digits = decSum;
            }
            else
            {
                Realnum a = *this;
                a.is_negative = false;
                Realnum b = other;
                b.is_negative = false;
                if (a >= b)
                {
                    res = a - b;
                    res.is_negative = is_negative;
                }
                else
                {
                    res = b - a;
                    res.is_negative = other.is_negative;
                }
            }
            res.normalize();
            return res;
        }

        Realnum operator-(const Realnum &other) const
        {
            Realnum negOther = other;
            negOther.is_negative = !negOther.is_negative;
            Realnum res = *this + negOther;
            res.normalize();
            return res;
        }

        Realnum operator*(const Realnum &other) const
        {
            Realnum res;
            res.is_negative = (is_negative != other.is_negative);
            auto aFull = combine_all_digits();
            auto bFull = other.combine_all_digits();
            auto product = mul_unsigned(aFull, bFull);
            int totalDec = static_cast<int>(decimal_digits.size() + other.decimal_digits.size());

            if (totalDec == 0)
            {
                res.integer_digits = product;
                res.decimal_digits.clear();
            }
            else if (totalDec >= static_cast<int>(product.size()))
            {
                res.integer_digits = {0};
                int padZero = totalDec - static_cast<int>(product.size());
                res.decimal_digits.assign(static_cast<size_t>(padZero), 0U);
                res.decimal_digits.insert(res.decimal_digits.end(), product.begin(), product.end());
            }
            else
            {
                size_t splitPos = product.size() - static_cast<size_t>(totalDec);
                res.integer_digits.assign(product.begin(), product.begin() + static_cast<ptrdiff_t>(splitPos));
                res.decimal_digits.assign(product.begin() + static_cast<ptrdiff_t>(splitPos), product.end());
            }
            res.normalize();
            return res;
        }

        /**
         * @brief 大数除法，默认输出50位小数
         * @param other 除数
         */
        Realnum operator/(const Realnum &other) const
        {
            Realnum a = *this;
            Realnum b = other;
            a.is_negative = false;
            b.is_negative = false;

            // 拼接全部数字
            auto a_all = a.combine_all_digits();
            auto b_all = b.combine_all_digits();
            int shift_a = static_cast<int>(a.decimal_digits.size());
            int shift_b = static_cast<int>(b.decimal_digits.size());

            // 对齐小数点，等价 a_all / b_all * 10^(shift_b - shift_a)
            int exp_diff = shift_b - shift_a;
            while (exp_diff > 0)
            {
                a_all.push_back(0);
                exp_diff--;
            }
            while (exp_diff < 0)
            {
                b_all.push_back(0);
                exp_diff++;
            }

            auto [q_int, q_dec] = div_unsigned(a_all, b_all, max_decimal);
            Realnum res;
            res.is_negative = (this->is_negative != other.is_negative);
            res.integer_digits = q_int;
            res.decimal_digits = q_dec;
            res.normalize();
            return res;
        }

        // ========= 复合赋值运算符 += -= *= /= =========
        Realnum &operator+=(const Realnum &other)
        {
            *this = *this + other;
            return *this;
        }
        Realnum &operator-=(const Realnum &other)
        {
            *this = *this - other;
            return *this;
        }
        Realnum &operator*=(const Realnum &other)
        {
            *this = *this * other;
            return *this;
        }
        Realnum &operator/=(const Realnum &other)
        {
            *this = *this / other;
            return *this;
        }

        // ========= 一元负号 =========
        Realnum operator-() const
        {
            Realnum ret = *this;
            if (!(ret.integer_digits.size() == 1 && ret.integer_digits[0] == 0 && ret.decimal_digits.empty()))
            {
                ret.is_negative = !ret.is_negative;
            }
            return ret;
        }

        // ========= 自增自减 前置/后置 =========
        // 前置 ++a
        Realnum &operator++()
        {
            *this = *this + Realnum("1");
            return *this;
        }
        // 后置 a++
        Realnum operator++(int)
        {
            Realnum tmp = *this;
            ++(*this);
            return tmp;
        }
        // 前置 --a
        Realnum &operator--()
        {
            *this = *this - Realnum("1");
            return *this;
        }
        // 后置 a--
        Realnum operator--(int)
        {
            Realnum tmp = *this;
            --(*this);
            return tmp;
        }

        // ========= 全部比较运算符 =========
        bool operator>=(const Realnum &other) const
        {
            if (is_negative != other.is_negative)
            {
                return !is_negative;
            }
            if (!is_negative)
            {
                if (cmp_unsigned_gt(integer_digits, other.integer_digits))
                    return true;
                if (integer_digits != other.integer_digits)
                    return false;
                auto da = decimal_digits;
                auto db = other.decimal_digits;
                size_t maxD = std::max(da.size(), db.size());
                da.resize(maxD, 0U);
                db.resize(maxD, 0U);
                return da >= db;
            }
            else
            {
                if (cmp_unsigned_gt(integer_digits, other.integer_digits))
                    return false;
                if (integer_digits != other.integer_digits)
                    return true;
                auto da = decimal_digits;
                auto db = other.decimal_digits;
                size_t maxD = std::max(da.size(), db.size());
                da.resize(maxD, 0U);
                db.resize(maxD, 0U);
                return da <= db;
            }
        }
        bool operator==(const Realnum &other) const
        {
            if (is_negative != other.is_negative)
                return false;
            if (integer_digits != other.integer_digits)
                return false;
            auto da = decimal_digits;
            auto db = other.decimal_digits;
            size_t maxD = std::max(da.size(), db.size());
            da.resize(maxD, 0U);
            db.resize(maxD, 0U);
            return da == db;
        }
        bool operator!=(const Realnum &other) const { return !(*this == other); }
        bool operator>(const Realnum &other) const { return (*this >= other) && !(*this == other); }
        bool operator<=(const Realnum &other) const { return !(*this > other); }
        bool operator<(const Realnum &other) const { return !(*this >= other); }

        // ========= 流输出、流输入 =========
        friend std::ostream &operator<<(std::ostream &os, const Realnum &num)
        {
            os << num.to_string();
            return os;
        }
        friend std::istream &operator>>(std::istream &is, Realnum &num)
        {
            std::string s;
            is >> s;
            num = Realnum(s);
            return is;
        }
    };

} // namespace JuLib

#endif // REALNUM_H
