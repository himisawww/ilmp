#include"Number.h"
#include<cstdlib>
#include<cstring>
#include<cmath>
#include<algorithm>

namespace ilmp{
    using namespace ilmp::utils;

    Complex::Complex(){}
    Complex::Complex(Number &&x):re(std::move(x)),im(0){}
    Complex::Complex(const Number &x):re(x),im(0){}
    Complex::Complex(Complex &&z):re(std::move(z.re)),im(std::move(z.im)){}
    Complex::Complex(const Complex &z):re(z.re),im(z.im){}

    void Complex::clear(){ re.clear(); im.clear(); }
    void Complex::neg(){ re.neg(); im.neg(); }
    void Complex::conjugate(){ im.neg(); }
    bool Complex::is_num() const{ return re.is_num()&&im.is_num(); }
    void Complex::swap(Complex &z){ re.swap(z.re); im.swap(z.im); }

    Complex &Complex::operator =(Number &&x){ re=std::move(x); im=0; return *this; }
    Complex &Complex::operator =(const Number &x){ re=x; im=0; return *this; }
    Complex &Complex::operator+=(const Number &x){ re+=x; return *this; }
    Complex &Complex::operator-=(const Number &x){ re-=x; return *this; }
    Complex &Complex::operator*=(const Number &x){ *this=*this*x; return *this; }
    Complex &Complex::operator/=(const Number &x){ *this=*this/x; return *this; }

    Complex &Complex::operator =(Complex &&z){ re=std::move(z.re); im=std::move(z.im); return *this; }
    Complex &Complex::operator =(const Complex &z){ re=z.re; im=z.im; return *this; }
    Complex &Complex::operator+=(const Complex &z){ re+=z.re; im+=z.im; return *this; }
    Complex &Complex::operator-=(const Complex &z){ re-=z.re; im-=z.im; return *this; }
    Complex &Complex::operator*=(const Complex &z){ *this=*this*z; return *this; }
    Complex &Complex::operator/=(const Complex &z){ *this=*this/z; return *this; }

    Complex operator-(const Complex &z){
        Complex result(z);
        result.neg();
        return result;
    }
    Complex operator+(const Complex &z){
        Complex result(z);
        return result;
    }
    Complex operator+(const Complex &z1,const Complex &z2){
        return Complex(z1.re+z2.re,z1.im+z2.im);
    }
    Complex operator-(const Complex &z1,const Complex &z2){
        return Complex(z1.re-z2.re,z1.im-z2.im);
    }

    //standard complex multiplication
    Complex mul_complex_4(const Complex &z1,const Complex &z2){
        return Complex(
            z1.re*z2.re-z1.im*z2.im,
            z1.im*z2.re+z2.im*z1.re);
    }
    //karatsuba complex multiplication
    //use 3 instead of 4 real multiplications, but results into lower accurary
    //maybe significantly slower when input re/im magnitude are unbalanced
    Complex mul_complex_3(const Complex &z1,const Complex &z2){
        Number
            p1(z1.re*z2.re),
            p2(z1.im*z2.im),
            p3((z1.re+z1.im)*(z2.re+z2.im));
        return Complex(p1-p2,p3-(p1+p2));
    }
    //standard complex multiplication z1*conj(z2)
    Complex mul_complex_c4(const Complex &z1,const Complex &z2){
        return Complex(
            z1.re*z2.re+z1.im*z2.im,
            z1.im*z2.re-z2.im*z1.re);
    }
    //karatsuba complex multiplication z1*conj(z2)
    //use 3 instead of 4 real multiplications, but results into lower accurary
    //maybe significantly slower when input re/im magnitude are unbalanced
    Complex mul_complex_c3(const Complex &z1,const Complex &z2){
        Number
            p1(z1.re*z2.re),
            p2(z1.im*z2.im),
            p3((z1.re+z1.im)*(z2.re-z2.im));
        return Complex(p1+p2,p3-(p1-p2));
    }
    //standard complex multiplication z*conj(z)
    Number sqr_complex_c(const Complex &z){
        return z.re*z.re+z.im*z.im;
    }

    Complex operator*(const Complex &z1,const Complex &z2){
        return mul_complex_4(z1,z2);
    }
    Complex operator/(const Complex &z1,const Complex &z2){
        return mul_complex_c4(z1,z2)/sqr_complex_c(z2);
    }
    Complex operator+(const Number &x,const Complex &z){ return Complex(x+z.re,+z.im); }
    Complex operator-(const Number &x,const Complex &z){ return Complex(x-z.re,-z.im); }
    Complex operator*(const Number &x,const Complex &z){ return Complex(x*z.re,x*z.im); }
    Complex operator/(const Number &x,const Complex &z){
        Complex result(x*z.re,x*z.im);
        result.conjugate();
        return result/sqr_complex_c(z);
    }
    Complex operator+(const Complex &z,const Number &x){ return Complex(z.re+x,z.im); }
    Complex operator-(const Complex &z,const Number &x){ return Complex(z.re-x,z.im); }
    Complex operator*(const Complex &z,const Number &x){ return Complex(z.re*x,z.im*x); }
    Complex operator/(const Complex &z,const Number &x){
        bool use_inv=false;
        mp_int nzr,nzi,na,nb,pn;
        mp_prec_t pz,pr,pzr,pzi;
        //if z.re/im,x all are non-zero number
        if(z.re.data&&z.re.ssize&&z.im.data&&z.im.ssize&&x.data&&x.ssize){
            //assume na<n(pa), i.e. z normalized
            na=std::max(nzr=z.re.size(),nzi=z.im.size());
            nb=x.size();
            pz=std::max(pzr=z.re.prec,pzi=z.im.prec);
            pr=std::min(pz,x.prec);
            if(pr==INT_PREC){
                pzr=z.re.is_int()?(nzr-z.re.dotp)*MP_LIMB_BITS+1+MIN_PREC_BITS:pzr;
                pzi=z.im.is_int()?(nzi-z.im.dotp)*MP_LIMB_BITS+1+MIN_PREC_BITS:pzi;
                pz=std::max(pzr,pzi);
                pz=std::max(pz,(nb-x.dotp)*MP_LIMB_BITS+1+MIN_PREC_BITS);
                pn=precision_limbs(pz);
            }
            else{
                pn=precision_limbs(pr);
            }
            const mp_uint COMPLEX_DIVINV_MIN=5;
            const mp_uint COMPLEX_DIVINV_MAX=20000;
            const mp_uint COMPLEX_DIVINV_SUPPRESS=50;
            const mp_prec_t COMPLEX_DIVINV_FACTOR=0.2;
            if(pn>COMPLEX_DIVINV_MIN){
                mp_prec_t kpn=std::min(pn-COMPLEX_DIVINV_MIN,COMPLEX_DIVINV_MAX);
                kpn=COMPLEX_DIVINV_FACTOR*std::sqrt(std::sqrt(kpn));
                //if na(z) short enough and nb(x) long enough, 
                //z*(1/x) (one inv and two mul) will be faster than z/x (two div)
                use_inv=na+COMPLEX_DIVINV_SUPPRESS/pn<mp_uint(kpn*nb);
            }
        }
        if(use_inv){
            if(pr==INT_PREC){
                Number rx=1;
                rx.prec=pz;
                rx/=x;
                Complex result;
                rx.prec=z.re.is_int()?MIN_PREC_BITS+std::max(z.re.logbit(),x.logbit()):pzr+MP_LIMB_BITS;
                result.re=z.re*rx;
                if(z.re.is_int())result.re.try_fix();
                rx.prec=z.im.is_int()?MIN_PREC_BITS+std::max(z.im.logbit(),x.logbit()):pzi+MP_LIMB_BITS;
                result.im=z.im*rx;
                if(z.im.is_int())result.im.try_fix();
                return result;
            }
            else{
                Number rx=1;
                rx.prec=pz+MP_LIMB_BITS;
                rx/=x;
                return Complex(z.re*rx,z.im*rx);
            }
        }
        else{
            return Complex(z.re/x,z.im/x);
        }
    }
};
