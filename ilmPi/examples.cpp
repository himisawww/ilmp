#include<iostream>
#include<string>
#include"Number.h"
#include"calctime.h"

using ilmp::Number;
using ilmp::Integer;

int main_example(){
    {
        //initialize by int
        Number a(25),b(7);
        std::cout<<"Integer division may result into floating point values.\n";
        std::cout<<"      a = "<<  a<<"\n";
        std::cout<<"      b = "<<  b<<"\n";
        std::cout<<"  a / b = "<<a/b<<"\n";
        std::cout<<
            "Unless the division is exact\n"
            "  where the result will also be an integer.\n";
        
        a=42;

        std::cout<<"      a = "<<  a<<"\n";
        std::cout<<"      b = "<<  b<<"\n";
        std::cout<<"  a / b = "<<a/b<<"\n";

        std::cout<<"This is same for sqrt\n";
        std::cout<<"     b  = "<<  b<<"\n";
        std::cout<<"sqrt(b) = "<<sqrt(b)<<"\n";
        b=81;
        std::cout<<"     b  = "<<  b<<"\n";
        std::cout<<"sqrt(b) = "<<sqrt(b)<<"\n";
    }

    std::cout<<std::endl;

    {
        //initialize by c-style string
        Number a("123456789012345678901234567890");
        Number b("233333333333333333333333333333");
        std::cout<<"Number class can handle arbitrary precision arithmetic.\n";
        std::cout<<"      a = "<<  a<<"\n";
        std::cout<<"      b = "<<  b<<"\n";
        std::cout<<"  a + b = "<<a+b<<"\n";
        std::cout<<"  a - b = "<<a-b<<"\n";
        std::cout<<"  a * b = "<<a*b<<"\n";
        std::cout<<"  a / b = "<<a/b<<"\n";
    }

    std::cout<<std::endl;
    
    {
        //initialize by floating point
        Number a(0.7f),b(0.2f);
        std::cout<<"floating point initialization may not be accurate...\n";
        std::cout<<"      a = "<<  a<<"\n";
        std::cout<<"      b = "<<  b<<"\n";
        std::cout<<"  a + b = "<<a+b<<"\n";
        a="0.7";
        b="0.2";
        std::cout<<"using string initialization can avoid that inaccuracy.\n";
        std::cout<<"      a = "<<  a<<"\n";
        std::cout<<"      b = "<<  b<<"\n";
        std::cout<<"  a + b = "<<a+b<<"\n";
    }

    std::cout<<std::endl;
    
    {
        std::cout<<
            "Number class supports a very large value range.\n"
            "Note:   To avoid confusion when i/o in radices larger than 10,\n"
            "        we use *^ rather than 'e' as the floating point exponent marker,\n"
            "        e.g. speed of light is 3*^8, not 3e8, m/s. \n";
        Number max_approx((2-Number("1*^-35"))*pow(Number(2),ilmp::MAX_EXP_BITS-1));
        Number min_approx(pow(Number("2`60"),-ilmp::MAX_EXP_BITS));
        Number overflow(max_approx*max_approx);
        Number underflow(min_approx*min_approx);
        std::cout<<" Max supported = "<<max_approx<<"\n";
        std::cout<<" Min supported = "<<min_approx<<"\n";
        std::cout<<"   Overflow    = "<<overflow<<"\n";
        std::cout<<"  -Overflow    = "<<-overflow<<"\n";
        std::cout<<"  Underflow    = "<<underflow<<"\n";
        std::cout<<"Indeterminate  = "<<overflow/overflow<<"\n";
    }

    std::cout<<std::endl;

    {
        std::cout<<"Base conversion is supported for bases between 2 ~ 36.\n";
        Number pi=ilmp::Pi(ilmp::convert_precision(ilmp::MIN_PREC_BITS,2));
        std::cout<<"Pi = \n"
            "  base  2: "<<pi.to_string(2)<<"\n"
            "  base  3: "<<pi.to_string(3)<<"\n"
            "  base  5: "<<pi.to_string(5)<<"\n"
            "  base  8: "<<pi.to_string(8)<<"\n"
            "  base 10: "<<pi.to_string(10)<<"\n"
            "  base 16: "<<pi.to_string(16)<<"\n"
            "  base 27: "<<pi.to_string(27)<<"\n"
            "  base 36: "<<pi.to_string(36)<<"\n";

        const char *base10_src="8.25980633651*^1346471";
        std::cout<<"Note: The exponential part (*^) is also represented in target base:\n";
        std::cout<<"    "<<base10_src<<" (base 10) = "
            <<Number(base10_src,10).to_string(36)<<" (base 36)"<<"\n";

        const char *base16_src="0.d000721`d";
        std::cout<<"      ... as well as the precision part (`):\n";
        std::cout<<"    "<<base16_src<<"            (base 16) = "
            <<Number(base16_src,16).to_string(16)<<" (base 16)"<<"\n";
    }

    std::cout<<std::endl;

    {
        std::cout<<"Some other examples:\n";

        std::cout<<" 3 ^ 200 = "<<pow(Integer(3),200)<<"\n";

        //precision can be set in string initialization by marker `
        std::cout<<" sqrt(2) = "<<sqrt(Number("2`100"))<<"\n";

        //a fast algorithm for calculating Pi is implemented
        std::cout<<"      Pi = "<<ilmp::Pi(ilmp::convert_precision(100))<<"\n";

        //a fast algorithm for calculating E is also implemented as ilmp::E(mp_prec_t)
        //however here we use a tiny program to calculate E ( 1 + 1/1! + 1/2! + ... + 1/n! )
        int i=0;
        Number e=1,ei=1;
        ei.set_precision(ilmp::convert_precision(100));
        do{
            ei/=++i;
            e+=ei;
            //x.logbit() will give approximately log2(x), 
            //x.precision() will give the amount of significant bits of x.
            //if ei is smaller than the least significant bit of e,
            // i.e. ei.logbit <= e.logbit - e.precision,
            // adding more terms will have no effect, and algorithm is done.
        } while(ei.logbit()>e.logbit()-e.precision());
        std::cout<<"       E = "<<e<<"\n";
    }

    std::cout<<std::endl;
    {
        Number a(666),b("1`0.1*^-30");
        std::cout<<
            "Number class can automatically handle both integer and floating point values.\n"
            "a string containing either exponent marker *^ or precision marker ` will be considered as floating point.\n"
            "hybrid operations will automatically result in correct precision.\n";
        std::cout<<"      a = "<<  a<<"\n";
        std::cout<<"      b = "<<  b<<"\n";
        std::cout<<"  a + b = "<<a+b<<"\n";
        Number c("99999999999999999999999999999"),d("123.456789");
        std::cout<<"      c = "<<  c<<"\n";
        std::cout<<"      d = "<<  d<<"\n";
        std::cout<<"  c * d = "<<c*d<<"\n";
        std::cout<<
            "What will happen if you calculate 1 - 1*^-10000000000000 ?\n"
            "Please just don't do that.\n";
    }

    std::cout<<std::endl;

    {
        std::cout<<
            "Integer class only supports integers, but has better performance.\n";
        double s=CalcTime();
        Number nsum=0;
        for(Number a=10000000;a;a-=1){
            nsum+=a;
        }
        s=CalcTime()-s;
        std::cout<<"  Number: sum(10000000) = "<<nsum<<" in "<<s<<"s\n";
        s=CalcTime();
        Integer isum=0;
        for(Integer a=10000000;a;a-=1){
            isum+=a;
        }
        s=CalcTime()-s;
        std::cout<<" Integer: sum(10000000) = "<<isum<<" in "<<s<<"s\n";
        std::cout<<"       ...  but they are all slower than built-in types.\n";
        s=CalcTime();
        long long cisum=0;
        for(int a=10000000;a;a-=1){
            cisum+=a;
        }
        s=CalcTime()-s;
        std::cout<<"     int: sum(10000000) = "<<cisum<<" in "<<s<<"s\n";
    }
    std::cout<<std::endl;
    return 0;
}
