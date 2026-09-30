#ifndef CCNSL_SESTEM
#define CCNSL_SESTEM
#include <iostream>
#include <string>
#include <cmath>
#include <limits>
#include <limits>
#include <thread>
#include <chrono>
namespace SyLib
{

    class systemform
    {
    public:
        void pause(long millisecond)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(millisecond));
        }
        void pause(int second)
        {
            std::this_thread::sleep_for(std::chrono::seconds(second));
        }
        void deline()
        {
            std::cout << "\r" << "                                                    \r";
        }
        void pause()
        {
#ifdef _WIN32
            system("pause");
#else
            std::cout << "Press Enter to continue...";
            cin.get(); // 等待用户按下回车键
#endif
        }

        void op_notepad()
        {
#ifdef _WIN32
            std::cout << "Opening in default text editor\n";
            system("notepad");
#elif __linux__
            system("xdg-open /tmp/calculator_note.txt");
            std::cout << "Opening in default text editor/tmp/calculator_note.txt" << endl;
#elif __APPLE__
            system("open /tmp/calculator_note.txt");
            std::cout << "Opening in default text editor/tmp/calculator_note.txt" << endl;
#else
            std::cout << "This platform does not support Notepad functionality" << endl;
#endif
        }
    } sys;
}
#endif
