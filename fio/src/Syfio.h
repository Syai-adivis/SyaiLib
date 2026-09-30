#ifndef CCNSL_FIRE_OP
#define CCNSL_FIRE_OP
#include <iostream>
#include <string>
#include <cmath>
#include <limits>
#include <iomanip>
#include <fstream>
#include <vector>
#include <stdexcept>
#include <sstream>
#include <memory>
#include <algorithm>
#include <limits>
#include <thread>
#include <chrono>
namespace SyLib
{

    class FileHandler
    {
    private:
        std::string m_filename;
        bool m_isOpen;

    public:
        // 构造函数
        FileHandler() : m_filename(""), m_isOpen(false) {}

        explicit FileHandler(const std::string &filename)
            : m_filename(filename), m_isOpen(false) {}

        // 析构函数
        ~FileHandler()
        {
            close();
        }

        // 打开文件（根据模式决定是读取、写入还是追加）
        bool open(std::ios::openmode mode = std::ios::in)
        {
            close(); // 先关闭已打开的文件
            std::fstream file(m_filename, mode);
            m_isOpen = file.is_open();
            return m_isOpen;
        }

        // 关闭文件
        void close()
        {
            m_isOpen = false;
        }

        // 写入单个数据
        template <typename T>
        bool write(const T &data, bool append = false)
        {
            std::ofstream file(m_filename, append ? std::ios::app : std::ios::trunc);
            if (!file.is_open())
            {
                std::cerr << "[Error] Cannot open file: " << m_filename << std::endl;
                return false;
            }
            file << data;
            return !file.fail();
        }

        // 写入容器数据
        template <typename Container>
        bool writeContainer(const Container &data, const std::string &delimiter = "\n", bool append = false)
        {
            std::ofstream file(m_filename, append ? std::ios::app : std::ios::trunc);
            if (!file.is_open())
            {
                std::cerr << "[Error] Cannot open file: " << m_filename << std::endl;
                return false;
            }
            for (const auto &item : data)
            {
                file << item << delimiter;
            }
            return !file.fail();
        }

        // 读取整个文件内容
        std::string readAll() const
        {
            std::ifstream file(m_filename);
            if (!file.is_open())
            {
                std::cerr << "[Error] Cannot open file: " << m_filename << std::endl;
                return "";
            }
            return std::string((std::istreambuf_iterator<char>(file)),
                               std::istreambuf_iterator<char>());
        }

        // 读取文件到字符串向量（每行一个元素）
        std::vector<std::string> readLines() const
        {
            std::vector<std::string> lines;
            std::ifstream file(m_filename);
            if (!file.is_open())
            {
                std::cerr << "[Error] Cannot open file: " << m_filename << std::endl;
                return lines;
            }
            std::string line;
            while (std::getline(file, line))
            {
                lines.push_back(line);
            }
            return lines;
        }

        // 读取文件到数值向量
        template <typename T>
        std::vector<T> readValues() const
        {
            std::vector<T> data;
            std::ifstream file(m_filename);
            if (!file.is_open())
            {
                std::cerr << "[Error] Cannot open file: " << m_filename << std::endl;
                return data;
            }
            T value;
            while (file >> value)
            {
                data.push_back(value);
            }
            return data;
        }

        // 检查文件是否存在
        bool exists() const
        {
            std::ifstream file(m_filename);
            return file.good();
        }

        // 获取文件大小
        size_t size() const
        {
            std::ifstream file(m_filename, std::ios::binary | std::ios::ate);
            return file.tellg();
        }

        // 清空文件内容
        bool clear()
        {
            std::ofstream file(m_filename, std::ios::trunc);
            return file.is_open();
        }

        // 静态方法：重命名文件
        static bool rename(const std::string &oldName, const std::string &newName)
        {
            return (std::rename(oldName.c_str(), newName.c_str()) == 0);
        }

        // 静态方法：删除文件
        static bool remove(const std::string &filename)
        {
            return (std::remove(filename.c_str()) == 0);
        }
    } fio;
}
#endif
