# SyaiLib

> Version: 5.0.Release | Namespace: **`SyLib`**
> Platform: Windows (GUI module Windows‑only), Linux / macOS (non‑GUI modules)

SyaiLib (short name **SyLib**) is a self‑written C++ utility library.
It wraps native Win32 GUI, arbitrary‑precision decimal arithmetic, graph algorithms, custom containers, enhanced console I/O, file utilities, syntactic‑sugar macros and nullable object wrappers.
It reduces boilerplate code for everyday C++ development.

## Table of Contents

- [Quick Start](#quick-start)
- [Module Overview](#module-overview)
- [Module API Reference](#module-api-reference)
  - [Sygui — Windows GUI](#sygui--windows-gui)
  - [SyRealnum — Arbitrary‑Precision Real Number](#syrealnum--arbitrary%E2%80%91precision-real-number)
  - [Syarray — Custom Dynamic Array `arraylt<T>`](#syarray--custom-dynamic-array-arrayltt)
  - [SyGraph — Graph Algorithm Library](#sygraph--graph-algorithm-library)
  - [SyConsole — Enhanced Console I/O](#syconsole--enhanced-console-io)
  - [Syfio — File Handler Utilities](#syfio--file-handler-utilities)
  - [Symath — Math Helper Utilities](#symath--math-helper-utilities)
  - [SyObject — Nullable Object Wrapper `Object<T>`](#syobject--nullable-object-wrapper-objectt)
  - [Sysystem — System Helpers](#sysystem--system-helpers)
  - [SySyntax — Syntax‑Sugar Macros & Type Aliases](#sysyntax--syntax%E2%80%91sugar-macros--type-aliases)
- [Build Configuration](#build-configuration)
- [FAQ](#faq)
- [Code Examples](#code-examples)
- [License](#license)

## Quick Start
### Include Headers

```
#include <Sygui.h>
#include <SyRealnum.h>
#include <Syarray.h>
// include other headers as‑needed

using namespace SyLib;
```

> 
> ⚠️ **`Sygui` is Windows‑only**. Including `Sygui.h` on non‑Windows platforms triggers a compile‑time `#error`.

## Module Overview

| Header File | Module | Main Features | Platform |
| --- | --- | --- | --- |
| `Sygui.h` | WinGUI | Win32 wrapper, signal‑slot event model, window, buttons, edit‑controls, listbox, menu, drawing, file dialogs | Windows Only |
| `SyRealnum.h` | Realnum | Arbitrary‑precision decimal arithmetic; no floating‑point precision loss | Cross‑Platform |
| `Syarray.h` | arraylt<T> | Hand‑rolled dynamic array container, similar to `std::vector` | Cross‑Platform |
| `SyGraph.h` | Graph<T> | Adjacency‑list graph; BFS, DFS, Dijkstra shortest‑path, cycle‑detection, directed / undirected weighted graph | Cross‑Platform |
| `SyConsole.h` | Console_opr | Robust console I/O, input error handling, codepage setup, screen clear | Cross‑Platform |
| `Syfio.h` | FileHandler | File read / write, read‑all‑text, read‑lines, file rename / delete | Cross‑Platform |
| `Symath.h` | MathUtils | Factorial, unit conversion, multiplication‑table helper | Cross‑Platform |
| `SyObject.h` | Object<T> | Nullable value wrapper built on `std::optional<T>` | Cross‑Platform |
| `Sysystem.h` | systemform | Thread sleep, console pause, launch notepad | Partial cross‑platform |
| `SySyntax.h` | Syntax‑Sugar | `repeat` loop macro, `var` alias for `auto`, `Enter` main‑function macro | Cross‑Platform |

## Module API Reference

### Sygui — Windows GUI

> 
> Header: `#include "Sygui.h"`
> Main Class: `SyLib::WinGUI`. Wraps raw Win32 API, implements signal‑slot callback system. Provides RAII GDI `GdiPen` / `GdiBrush`.

```
WinGUI(const std::wstring &clsName = L"WinGUI_Class",
       const std::wstring &title = L"WinGUI Window",
       int w = 800, int h = 600);
```

#### Key Member Functions

| Function | Description |
| --- | --- |
| `bool Create(HINSTANCE hInst, int nCmdShow)` | Register window class and create main window; pops error message with `GetLastError()` on failure |
| `int RunMessageLoop()` | Run Windows message loop; returns application exit code |
| `HWND CreateButtonCtrl(...)` | Create push‑button child control |
| `HWND CreateEditBoxCtrl(...)` | Edit‑box control; multi‑line / password / read‑only supported |
| `HWND CreateListBoxCtrl(...)` | List‑box control |
| `HWND CreateComboBoxCtrl(...)` | Combo‑box / drop‑list control |
| `CreateCheckBoxCtrl / CreateRadioButtonCtrl` | Check‑box and radio‑button controls |
| `CreateMainMenuCtrl(...)` | Create top‑level main menu bar |
| `OpenFileDialogCtrl / SaveFileDialogCtrl` | Common open / save file dialog |
| `DrawTextToWnd / DrawRectToWnd` | Direct GDI drawing on main window surface |
| `CenterWnd()` | Center main window on screen |
| `EnableFileDrop(bool enable)` | Enable file drag‑and‑drop; receive file list via `signal_dropFiles` |

#### Signal‑Slot Event System

```
signal_wndProc;        // Raw window‑procedure message, returns LRESULT
signal_wndClose;        // Window close event
signal_wndDestroy;      // Window destroy event
signal_wndPaint;        // WM_PAINT paint event
signal_command;         // Control command notifications (button‑click etc.)
signal_mouseMove, signal_lButtonDown... // Mouse input events
signal_keyDown;         // Keyboard key‑down
signal_timer;           // Timer tick event
signal_dropFiles;       // Drag‑drop file receive
```

Use `connectScoped()` to bind callbacks. `ScopedConnection` automatically disconnects to prevent dangling callbacks.

> 
> Destructor automatically calls `DestroyWindow` and `UnregisterClassW`. Do not manually release window class twice.

#### Minimal GUI Example

```
#include <Sygui.h>

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int nCmdShow)
{
    SyLib::WinGUI wnd(L"MyWindowClass", L"Demo Window", 640, 480);
    if (!wnd.Create(hInst, nCmdShow))
        return -1;
    return wnd.RunMessageLoop();
}
```

### SyRealnum — Arbitrary‑Precision Real Number

> 
> Header: `#include "SyRealnum.h"`, Class: `SyLib::Realnum`

Internally stores integer digits and decimal digits separately inside `std::vector<uint8_t>`. Eliminates floating‑point precision loss for decimal arithmetic.

Supports operators: `+ - * / ++ -- += -= *= /=`; all comparison operators `> >= < <= == !=`.

Construct from string, integer, `long long`, `long double`.

```
Realnum a("0.1");
Realnum b("0.2");
auto c = a + b;
Console.out.outputln(c.to_string()); // outputs "0.3" without precision error
```

| API | Description |
| --- | --- |
| `to_string()` | Convert number to human‑readable string |
| `abs()` | Return absolute value copy |
| `pow(int exponent)` | Non‑negative‑integer exponentiation |

> 
> Division defaults to maximum 50 decimal digits. Pass custom `max_decimal` in constructor to adjust precision.

### Syarray — Custom Dynamic Array `arraylt<T>`

> 
> Header: `#include "Syarray.h"`, Template class `SyLib::arraylt<T>`

Hand‑written dynamic‑array container, conceptually similar to `std::vector`. Implements copy‑semantics and move‑semantics.

```
size_t size();
size_t capacity();
bool empty();
void reserve(size_t new_cap);
void resize(size_t new_size, const T& val = T());
void shrink_to_fit();

void push_back(const T&);
template<typename...Args> void emplace_back(Args&&... args);
void pop_back();

iterator insert(iterator pos, const T& val);
iterator erase(iterator pos);
iterator erase(iterator first, iterator last);
void clear();

T& operator[](size_t idx);
T& at(size_t idx); // throws std::out_of_range when index out‑of‑bounds
```

Sample usage:

```
arraylt<Idef> arr;
arr.push_back(10);
arr.emplace_back(20);
for (var v : arr)
    Console.out.outputln(v);
```

### SyGraph — Graph Algorithm Library

> 
> Header: `#include "SyGraph.h"`, Template class `SyLib::Graph<T>`

Adjacency‑list implementation for directed / undirected weighted graphs.

| Method | Function |
| --- | --- |
| `Graph(bool is_directed = false)` | Constructor; `false` = undirected graph |
| `add_vertex(const T& v)` | Insert vertex |
| `add_edge(v1, v2, int weight = 1)` | Add weighted edge; throws exception for duplicate edges |
| `remove_edge(v1, v2)` | Delete an existing edge |
| `remove_vertex(v)` | Delete vertex and all edges connected to it |
| `has_edge(v1, v2)` | Test whether edge exists |
| `get_neighbors(v)` | Return neighbor list `vector<pair<T, int>>` |
| `dfs(start)` | Depth‑first traversal, returns vertex sequence `vector<T>` |
| `bfs(start)` | Breadth‑first traversal |
| `dijkstra(start)` | Dijkstra algorithm, distance map `unordered_map<T, int>` |
| `get_shortest_path(start, end)` | Return shortest‑path vertex sequence |
| `has_cycle()` | Cycle detection; auto‑adapt directed / undirected graph |
| `print_graph()` | Print adjacency‑list to console |

```
Graph<String> g(false);
g.add_edge("A", "B", 2);
auto path = g.get_shortest_path("A", "B");
```

### SyConsole — Enhanced Console I/O

> 
> Header: `#include "SyConsole.h"`, global instance `SyLib::Console`

- `Console.in` for input operations
- `Console.out` for formatted output
- `Console.form` for numeric formatting

```
// Set console codepage to UTF‑8 (65001)
Console.consoleSetup(65001);

Idef num = Console.in.read<Idef>("Please enter number: ");
String s = Console.in.readln("Input one line: ");

Console.out.outputln("Result ", num);
```

`inputStr` core methods:

- `read<T>(prompt)` read value of type `T`. Automatically clear fail‑bit on malformed input, returns default value.
- `readln()` read full input line as string.
- `Scan(prompt, var1, var2 ...)` read multiple variables in one call.
- `cnfail()` check whether input error has occurred.

### Syfio — File Handler Utilities

> 
> Header: `#include "Syfio.h"`, global instance `SyLib::fio`

```
fio.open("test.txt", std::ios::in);
std::string allText = fio.readAll();
auto lines = fio.readLines();
fio.write<std::string>("hello world", false /* overwrite instead append */);

bool ok = fio.exists();
size_t sz = fio.size();
```

### Symath — Math Helper Utilities

> 
> Header: `Symath.h`, global instance `SyLib::mt`

```
mt.fac(5);            // factorial 5!
mt.kmToMile(100);     // kilometer to mile
mt.cToF(25);          // Celsius to Fahrenheit
mt.mult(9);           // print multiplication table up to 9
```

### SyObject — Nullable Object Wrapper `Object<T>`

> 
> Header: `SyObject.h`. Wrapper built on `std::optional<T>`.

```
Object<Idef> obj(100);
if (!obj.isNull()) begin
    Idef val = obj.getOrthrow(); // throws if no value present
end
obj.del(); // mark object as null
```

### Sysystem — System Helpers

> 
> Header: `Sysystem.h`, global instance `SyLib::sys`

```
sys.pause(500);      // sleep 500 milliseconds
sys.pause();         // console press‑any‑key pause
sys.op_notepad();    // launch notepad (Windows‑preferred)
```

### SySyntax — Syntax‑Sugar Macros & Type Aliases

> 
> Header: `SySyntax.h`

```
repeat(10) begin
    // loop body runs 10 times
end
var x = 10;          // alias for auto x =10;

Enter(int argc, char* argv[]) begin
    // expands to int main(int argc, char* argv[])
end
UnParamEnter begin
    // expands to int main()
end
Exit 0;              // alias for return 0;
```

Type aliases:
`Idef(int)`, `Imidlong(long)`, `Ilong(long long)`, `sFloat(float)`, `Float(double)`, `lFloat(long double)`, `String(std::string)`, `Char(char)`, `Byte(int8_t)`

## Build Configuration

### Windows (Sygui GUI enabled)

- Link Windows system libraries: `user32.lib gdi32.lib comdlg32.lib shell32.lib`
- Define `UNICODE` and `_UNICODE`. Project character‑set: **Unicode**.
- Supports MSVC and MinGW‑w64.

### Linux / macOS (non‑GUI modules only)

- **Do NOT include `Sygui.h`**.
- Compile with C++17 or newer standard.

> 
> Recommend C++17 or higher; library uses `std::optional`, `std::forward`, `if constexpr`.

## FAQ

### Q: `RegisterClassExW` returns failure (`Failed to register window class`)

1. Duplicate registration with identical window‑class name.
2. `m_hInstance` is passed as `nullptr`.
3. Do not instantiate multiple `WinGUI` objects with same class‑name; destructor calls `UnregisterClassW`.

### Q: `CreateWindowExW` window creation fails

1. Verify `RegisterWindowClass()` returns true before creating window.
2. Pass correct current‑process `HINSTANCE` into `Create()`.
3. Check popup error dialog, inspect `GetLastError()` error‑code for Win32 root‑cause.

### Q: Realnum division runs slow

Reduce `max_decimal` parameter in constructor; default is 50 decimal digits.

### Q: Notes for `arraylt::emplace_back`

Performs in‑place object construction. Do not pass already‑invalidated objects.

## Code Examples

### Console Input + Arbitrary‑Precision Calculation

```
#include <iostream>
#include <SyConsole.h>
#include <SyRealnum.h>

UnPrefix SyLib;

UnParamEnter begin
    Console.consoleSetup(65001);
    var a = Console.in.read<Realnum>("Input first decimal number: ");
    var b = Console.in.read<Realnum>("Input second decimal number: ");
    var res = a + b;
    Console.out.outputln("a + b = ", res.to_string());
    Exit 0;
end
```

## License
Apache 2.0 License.