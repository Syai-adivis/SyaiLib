#ifndef JUST_FUNC_H
#define JUST_FUNC_H
#include<cmath>
namespace SyLib{

class Func{
	long double a=1,b=0,c=0,d=0;
	public: 
		Func(long double na=1,long double nb=0,long double nc=0,long double nd=0){a=na;b=nb;c=nc;d=nd;}
		long double f_z(long double x){return x/a;}
		long double f_f(long double x){return x*a;}
		long double f_2(long double x){return a*pow(x,2)+b*x+c;}
		long double f_3(long double x){return a*pow(x,3)+b*pow(x,2)+c*x+d;}
    	void get_const(long double arr[4]) const {arr[0] = a;arr[1] = b;arr[2] = c;arr[3] = d;}
		void set_const(long double na=0,long double nb=0,long double nc=0,long double nd=0){a=na;b=nb;c=nc;d=nd;}
		long double get_a() const { return a; }
    	long double get_b() const { return b; }
    	long double get_c() const { return c; }
    	long double get_d() const { return d; }
};
}
#endif
