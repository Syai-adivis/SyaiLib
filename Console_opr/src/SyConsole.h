/*
概述
Justdio 是一个基于 C++ 标准库的输入输出扩展库，
提供了更便捷的输入处理、输出格式化及错误处理功
能，简化了 C++ 程序中的 IO 操作
*/
#ifndef CCNSL_IO
#define CCNSL_IO
#include <iostream>
#include <string>
#include <cmath>
#include <limits>
#include <iomanip>
#include <cstdlib>
// Windows控制台编码适配
#ifdef _WIN32
#include <windows.h>
#endif
namespace SyLib
{

	class inputStr
	{
		bool isSayError;
		bool inputerror;

	public:
		inputStr()
		{
			isSayError = false;
			inputerror = false;
		}
		// 是否有错误
		bool cnfail()
		{
			return inputerror;
		}
		// 是否报告错误
		void iserror(bool arg)
		{
			isSayError = arg;
		}

		/*输入
		const std::string& prompt:提示语
		*/
		template <typename T>
		T read(const std::string &prompt)
		{
			T value;
			std::cout << prompt;
			std::cin >> value;
			if (inputerror && isSayError)
				std::cerr << "Please clean input error!\n";
			if (std::cin.fail())
			{
				std::cin.clear();
				std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
				std::cerr << "Input error!\n\a";
				inputerror = true;
				return T();
			}
			else
			{
				std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
				return value;
			}
		}
		/*输入
		 */
		template <typename T>
		T read()
		{
			T value;
			std::cin >> value;
			if (inputerror && isSayError)
				std::cerr << "Please clean input error!\n";
			if (std::cin.fail())
			{
				std::cin.clear();
				std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
				std::cerr << "Input error!\n\a";
				inputerror = true;
				return T();
			}
			else
			{
				std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
				return value;
			}
		}
		/*输入一行
		 */
		std::string readln()
		{
			std::string value;
			std::getline(std::cin, value);
			if (inputerror && isSayError)
				std::cerr << "Please clean input error!\n";
			if (std::cin.fail())
			{
				std::cin.clear();
				std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
				std::cerr << "Input error!\n\a";
				inputerror = true;
				return "";
			}
			else
			{
				return value;
			}
		}
		/*输入一行
		const std::string& prompt:提示语
	   */
		std::string readln(const std::string &prompt)
		{
			std::string value;
			std::cout << prompt;
			std::getline(std::cin, value);
			if (inputerror && isSayError)
				std::cerr << "Please clean input error!\n";
			if (std::cin.fail())
			{
				std::cin.clear();
				std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
				std::cerr << "Input error!\n\a";
				inputerror = true;
				return "";
			}
			else
			{
				return value;
			}
		}

		// 清除错误
		void clear()
		{
			inputerror = false;
		}

		void ScanHelper() {}

		// 递归处理每个变量
		template <typename T, typename... Args>
		void ScanHelper(T &first, Args &...rest)
		{
			std::cin >> first;
			if (std::cin.fail())
			{
				std::cin.clear();
				std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
				std::cerr << "Input error!\n\a";
				inputerror = true;
			}
			ScanHelper(rest...);
		}

		/*输入
		const std::string& prompt:提示语
		*/
		template <typename T, typename... Args>
		void Scan(const std::string &prompt, T &first, Args &...args)
		{
			if (inputerror && isSayError)
				std::cerr << "Please clean input error!\n";
			std::cout << prompt;
			std::cin >> first;
			if (std::cin.fail())
			{
				std::cin.clear();
				std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
				std::cerr << "Input error!\n\a";
				inputerror = true;
			}
			ScanHelper(args...);
		}

		/*输入
		 */
		template <typename... Args>
		void Scan(Args &...args)
		{
			if (inputerror && isSayError)
				std::cerr << "Please clean input error!\n";
			ScanHelper(args...);
		}
	};

	class outputStr
	{
	public:
		/*基础输出*/
		void output() {}
		/*基础输出*/
		template <typename U, typename... Rest>
		void output(U first, Rest... rest)
		{
			std::cout << first;
			output(rest...); // 递归调用
		}
		/*带换行的输出*/
		void outputln()
		{
			std::cout << std::endl;
		}
		/*带换行的输出*/
		template <typename U, typename... Rest>
		void outputln(U first, Rest... rest)
		{
			std::cout << first << " ";
			outputln(rest...); // 递归调用
		}
		/*错误输出*/
		void errput() { std::cerr << "\a"; }
		/*错误输出*/
		template <typename U, typename... Rest>
		void errput(U first, Rest... rest)
		{
			std::cerr << first;
			errput(rest...); // 递归调用
		}
	};
	class format
	{
	public:
		void formbase(const int base)
		{
			std::cout << std::fixed << std::setbase(base);
		}

		void formprec(const int format)
		{
			std::cout << std::fixed << std::setprecision(format);
		}

		void formstew(const int format)
		{
			std::cout << std::fixed << std::setw(format);
		}
	};
	class Console_opr
	{
	public:
		outputStr out;
		format form;
		inputStr in;
		Console_opr()
		{
			in = inputStr();
		}
		// 初始化控制台
		void consoleSetup(long Encode = 65001, bool isop = false)
		{
#ifdef _WIN32
			SetConsoleOutputCP(Encode);
			SetConsoleCP(Encode);
			// 清除chcp输出的多余信息
			if (!isop)
				system("chcp >nul");
#endif
		}
		// 跨平台清屏函数
		void clearScreen()
		{
#ifdef _WIN32
			system("cls");
#else
			system("clear");
#endif
		}
	};
	inline Console_opr Console;
} // namespace JuLib
#endif
