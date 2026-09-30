// includes
#include <string>  //stdc++ string
#include <cstdint> //stdc inttypes
// add-syntax
// Helper
#if !(defined(CONCAT) && defined(CONCAT_INPL))
#define CONCAT_IMPL(a, b) a##b
#define CONCAT(a, b) CONCAT_IMPL(a, b)
#endif
// repeat(int_lit){stmt*}
#if !defined(repeat)
#define REPEAT_LOOP_BODY(varname, num) for (int varname = 0; varname < (num); varname++)
#define repeat(num) REPEAT_LOOP_BODY(CONCAT(_rl, __COUNTER__), num)
#endif
// repeatLong(long_lit){stmt*}
#if !defined(repeatLong)
#define REPEAT_LOOP_BODY_LONG(varname, num) for (long varname = 0; varname < (num); varname++)
#define repeatLong(num) REPEAT_LOOP_BODY_LONG(CONCAT(_rl, __COUNTER__), num)
#endif
// fRepeat{stmt*}
#if !defined(fRepeat)
#define fRepeat while (true)
#endif
// repeatColl(var,collectionName){stmt*}
#if !defined(repeatColl)
#define repeatColl(i, collection) for (i : collection)
#endif
// Enter(param){stmt*}
#if !defined(Enter)
#define Enter(params) int main(params)
#endif
// UnParamEnter{stmt*}
#if !defined(UnParamEnter)
#define UnParamEnter int main()
#endif
// UnPrefix identifer
#if !defined(UnPrefix)
#define UnPrefix using namespace
#endif
// Exit obj
#if !defined(Exit)
#define Exit return
#endif
// var name
#if !defined(var)
#define var auto
#endif
// types
typedef int Idef;
typedef double lFloat;
typedef long Ilong;
typedef long long Ilongl;
typedef long double llFloat;
typedef float Float;
typedef std::string String;
typedef char Char;
typedef int8_t Byte;