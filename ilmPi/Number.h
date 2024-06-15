#pragma once
#include<stdint.h>
#define ILMP_NO_TYPES
#include"../ilmp/ilmp.h"
#include<utility>
#pragma comment(lib,"../x64/Release/ilmp.lib")

namespace ilmp{
    class Integer;
    class Number;

    typedef uint8_t mp_byte_t;
    typedef uint64_t mp_limb_t;
    typedef int64_t mp_slimb_t;
    typedef mp_limb_t *mp_ptr;
    typedef const mp_limb_t *mp_srcptr;
    typedef int64_t mp_int;
    typedef uint64_t mp_uint;
    typedef double mp_prec_t;

    //constants

    const mp_uint MP_MAX_UINT=-1;
    const mp_int MP_MAX_INT=MP_MAX_UINT>>1;
    const mp_int MP_MIN_INT=-1-MP_MAX_INT;
    const mp_int MP_LOG2_LIMB_BITS=6;
    const mp_int MP_LIMB_BITS=64;
    const mp_limb_t INT_PINF=0x7FF0000000000000;
    const mp_prec_t INT_PREC=*(mp_prec_t*)&INT_PINF;
    //B=2^64
    const mp_prec_t MP_BASE=18446744073709551616.;
    //max representable int in mp_prec_t
    const mp_int MAX_PREC_INT=(mp_int)1<<53;
    //max supported number of limbs
    const mp_int MAX_LIMBS=MAX_PREC_INT>>6;

    //precision configurations
    
    //minimum precision bits,
    // used when input a float value without prec specifier,
    // in [32, MAX_PREC_BITS]
    extern mp_prec_t MIN_PREC_BITS;
    //maximum precision bits,
    // in [MIN_PREC_BITS, MAX_LIMBS*MP_LIMB_BITS]
    extern mp_prec_t MAX_PREC_BITS;
    extern mp_int MAX_EXP_LIMBS;
    extern mp_prec_t MAX_EXP_BITS;
    //when exp>=this value, use scientific format to print
    extern mp_int MIN_EXP_SCI_OUT;

    //allocate configurations
    namespace utils{
        const mp_int EXTRA_ALLOC_LIMB=4;
        const mp_int EXTRA_ALLOC_RATIO=16;

        mp_int precision_limbs(mp_int iprec);
        mp_int reserve_limbs_lower(mp_int size);
        mp_int reserve_limbs_upper(mp_int size);
        // log2(2^x+2^y)
        mp_prec_t log2add(mp_prec_t x,mp_prec_t y);
        // bit count leading zeros
        int count_left_zeros(mp_limb_t x);
    };
    /*

       Zero:
               0 = {data,0}

       NonZero values:
         sign*[data,size]
                 = {data,sign*size}
       //  sign=[-1|1], data[size-1]!=0

    Note:
      * if data==value, value[2] stores the value
      * else alloc is # of limbs allocated and pointed to by data
      * Integer is faster than Number for small integers
      * Integer do not support nan/inf or overflow detection
    */
    class Integer{
    public:
        mp_ptr data;
        mp_int ssize;
        union{
            mp_limb_t value[2];
            mp_int alloc_size;
        };
    public:
        Integer();
        Integer(bool);
        Integer(char);
        Integer(signed char);
        Integer(unsigned char);
        Integer(signed short);
        Integer(unsigned short);
        Integer(signed int);
        Integer(unsigned int);
        Integer(signed long);
        Integer(unsigned long);
        Integer(signed long long);
        Integer(unsigned long long);
        explicit Integer(float);
        explicit Integer(double);

        Integer(Integer &&);
        Integer(const Integer&);
        explicit Integer(Number &&x);
        explicit Integer(const Number&);
        Integer(const char*,int base=10);
        ~Integer();

        explicit operator bool              () const;
        explicit operator char              () const;
        explicit operator signed char       () const;
        explicit operator unsigned char     () const;
        explicit operator signed short      () const;
        explicit operator unsigned short    () const;
        explicit operator signed int        () const;
        explicit operator unsigned int      () const;
        explicit operator signed long       () const;
        explicit operator unsigned long     () const;
        explicit operator signed long long  () const;
        explicit operator unsigned long long() const;
        explicit operator float             () const;
        explicit operator double            () const;

        //this=0, release allocated memory
        void clear();
        //# of limbs pointed by data
        mp_int capacity() const;

        void from_int(mp_int);
        void from_uint(mp_uint);
        //from_float(pointer to float, size of float format);
        //require little endian
        //Note: type_bytes should be actuall format size,
        //      some platform have 80-bits long double padded to 12/16 bytes,
        //      in this case, type_bytes should be 10 but not sizeof(long double)
        void from_float(    const void *fptr,    mp_int type_bytes,
                            mp_int exp_bits=-1,  mp_int hidden_bit=-1);
        //[+-][N]
        void from_str(const char*,int base=10);

        //result may mod by 2^64
        mp_int to_int() const;
        //result may mod by 2^64
        mp_uint to_uint() const;
        //to_float(pointer to float, size of float format);
        //result will be little endian
        //Note: type_bytes should be actuall format size,
        //      some platform have 80-bits long double padded to 12/16 bytes,
        //      in this case, type_bytes should be 10 but not sizeof(long double)
        void to_float(      void *fptr,          mp_int type_bytes,
                            mp_int exp_bits=-1,  mp_int hidden_bit=-1) const;
        //buffer length needed to store this as a string in a given base
        mp_int strlen(int base=10) const;
        //convert this into a string in a given base
        //return strlen
        mp_int to_str(char*,int base=10) const;

        //this=-this
        void neg();
        //-1 for negative,0 for 0,+1 for positive
        int sign() const;
        //# of limbs in use to store this
        mp_int size() const;

        //1+floor(logB(this))
        mp_int loglimb() const;
        //log2(this*2^shift)
        mp_prec_t logbit(mp_int shift=0) const;

        void swap(Integer&);

        //if alloc too long compared to size
        void normalize();

        Integer &operator =(Integer&&);
        Integer &operator =(const Integer&);
        Integer &operator+=(const Integer&);
        Integer &operator-=(const Integer&);
        Integer &operator*=(const Integer&);
        Integer &operator/=(const Integer&);

        Integer &operator<<=(mp_int);
        Integer &operator>>=(mp_int);
    };

    //1:>, 0:=, -1:<, *Note: -2:nan
    int compare(const Integer&,const Integer&,bool abscomp=false);
    bool operator==(const Integer&,const Integer&);
    bool operator!=(const Integer&,const Integer&);
    bool operator>(const Integer&,const Integer&);
    bool operator>=(const Integer&,const Integer&);
    bool operator<(const Integer&,const Integer&);
    bool operator<=(const Integer&,const Integer&);

    Integer operator-(const Integer&);
    Integer operator+(const Integer&);
    Integer operator+(const Integer&,const Integer&);
    Integer operator-(const Integer&,const Integer&);
    Integer operator*(const Integer&,const Integer&);
    Integer operator/(const Integer&,const Integer&);

    void add(Integer&,const Integer&,const Integer&);
    void sub(Integer&,const Integer&,const Integer&);
    void mul(Integer&,const Integer&,const Integer&);
    void div(Integer&,const Integer&,const Integer&);

    Integer operator<<(const Integer&,mp_int);
    Integer operator>>(const Integer&,mp_int);

/*
   Special values:
         NaN = {null, 0,...}
        +Inf = {null, +,...}
        -Inf = {null, -,...}

   Zeros:
           0 = {data, 0, ., INT_PREC}
   0.*^-prec = {data, 0, ., prec};
   //* -MAX_EXP_BITS <= prec < MAX_EXP_BITS
   //* data=value

   NonZero values:
     sign*[data,size]*2^(-64*dotp)
             = {data,sign*size,dotp,prec}
   //  sign=[-1|1], data[size-1]!=0
   //  prec == INT_PREC for int value
   //  dotp <= 0 for int value
   //* -MAX_EXP_LIMBS < size-dotp <= MAX_EXP_LIMBS
   //* 0 < prec <= MAX_PREC_BITS for float value
   //* size < precision_limbs(prec) for float value

Note:
  * overprec or overflow values can violate //*'s
  * prec is in bits.
  * if data==value, value[2] stores the value
  * else alloc > 2 is # of limbs allocated and pointed to by data
*/
    class Number{
    public:
        mp_ptr data;
        mp_int ssize;
        mp_int dotp;
        mp_prec_t prec;
        union{
            mp_limb_t value[2];
            mp_int alloc_size;
        };
    public:
        Number();
        Number(bool);
        Number(char);
        Number(signed char);
        Number(unsigned char);
        Number(signed short);
        Number(unsigned short);
        Number(signed int);
        Number(unsigned int);
        Number(signed long);
        Number(unsigned long);
        Number(signed long long);
        Number(unsigned long long);
        Number(float);
        Number(double);
        Number(Integer &&);
        Number(const Integer &);
        Number(Number &&);
        Number(const Number&);
        Number(const char*,int base=10);
        ~Number();
        
        explicit operator bool              () const;
        explicit operator char              () const;
        explicit operator signed char       () const;
        explicit operator unsigned char     () const;
        explicit operator signed short      () const;
        explicit operator unsigned short    () const;
        explicit operator signed int        () const;
        explicit operator unsigned int      () const;
        explicit operator signed long       () const;
        explicit operator unsigned long     () const;
        explicit operator signed long long  () const;
        explicit operator unsigned long long() const;
        explicit operator float             () const;
        explicit operator double            () const;

        //this=nan or inf, release allocated memory
        void clear();
        //# of limbs pointed by data
        mp_int capacity() const;

        void from_int(mp_int);
        void from_uint(mp_uint);
        //from_float(pointer to float, size of float format);
        //require little endian
        //Note: type_bytes should be actuall format size,
        //      some platform have 80-bits long double padded to 12/16 bytes,
        //      in this case, type_bytes should be 10 but not sizeof(long double)
        void from_float(    const void *fptr,    mp_int type_bytes,
                            mp_int exp_bits=-1,  mp_int hidden_bit=-1);
        //[+-][N.N][`[+-]prec][*^[+-]exponent]
        void from_str(const char*,int base=10);

        //result may be rounded or mod by 2^64
        mp_int to_int() const;
        //result may be rounded or mod by 2^64
        mp_uint to_uint() const;
        //to_float(pointer to float, size of float format);
        //result will be little endian
        //Note: type_bytes should be actuall format size,
        //      some platform have 80-bits long double padded to 12/16 bytes,
        //      in this case, type_bytes should be 10 but not sizeof(long double)
        void to_float(      void *fptr,          mp_int type_bytes,
                            mp_int exp_bits=-1,  mp_int hidden_bit=-1) const;
        //buffer length needed to store this as a string in a given base
        mp_int strlen(int base=10) const;
        //convert this into a string in a given base
        //return strlen
        mp_int to_str(char*,int base=10) const;

        //this=-this
        void neg();
        //-1 for negative,0 for 0 and nan,+1 for positive
        int sign() const;
        //# of data limbs, valid only when is_num()
        mp_int size() const;

        //1+floor(logB(this))
        mp_int loglimb() const;
        //log2(this*2^shift)
        mp_prec_t logbit(mp_int shift=0) const;
        //precision in bits, if this==0, returns accuracy
        mp_prec_t precision() const;
        //precision are in bits
        //set to INT_PREC or +inf will fix this to integer
        void set_precision(mp_prec_t);
        //if is inf, return sign of inf
        //else return 0
        int is_inf() const;
        bool is_nan() const;
        //check if is integer
        //return value undefined for inf/nan
        bool is_int() const;
        //equivalent to !is_inf() && !is_nan()
        bool is_num() const;
        void swap(Number&);
        //if overprec or overflow
        //if size too long compared to prec
        void normalize();
        //fix this to integer if should be
        void try_fix();

        Number &operator =(Number&&);
        Number &operator =(const Number&);
        Number &operator+=(const Number&);
        Number &operator-=(const Number&);
        Number &operator*=(const Number&);
        Number &operator/=(const Number&);

        Number &operator<<=(mp_int);
        Number &operator>>=(mp_int);
    };

    //1:>, 0:=, -1:<, *Note: -2:nan
    int compare(const Number&,const Number&,bool abscomp=false);
    bool operator==(const Number&,const Number&);
    bool operator!=(const Number&,const Number&);
    bool operator>(const Number&,const Number&);
    bool operator>=(const Number&,const Number&);
    bool operator<(const Number&,const Number&);
    bool operator<=(const Number&,const Number&);

    Number operator-(const Number&);
    Number operator+(const Number&);
    Number operator+(const Number&,const Number&);
    Number operator-(const Number&,const Number&);
    Number operator*(const Number&,const Number&);
    Number operator/(const Number&,const Number&);
    
    Number operator<<(const Number &,mp_int);
    Number operator>>(const Number &,mp_int);

    //fix this to an integer
    //fix type: -2: to zero, -1: to -inf, 0: to int, 1: to +inf, 2: to +/- inf
    Number fix(const Number &x,int fix_type=-2);
    Number floor(const Number &x);
    Number ceil(const Number &x);
    Number round(const Number &x);

    Number sqrt(const Number&);
    Number pow(const Number&,mp_int);

    //convert precision from digits in a given base to mp_prec_t in bits
    mp_prec_t convert_precision(mp_uint digits,int base=10);
    //force precision into [MIN_PREC_BITS, MAX_PREC_BITS]
    mp_prec_t check_precision(mp_prec_t precision);
    Number Pi(mp_prec_t precision);
    Number E(mp_prec_t precision);
    Number Prime_BuenosAires(mp_prec_t precision);
    //return coef * arctan[h if hyperbolic] (1/x) for x>=2
    //precision in bits.
    Number acoti(mp_int _coef,mp_uint _x,mp_prec_t precision,int _hyperbolic=0);
    Number const_ln2(mp_prec_t precision);

    //basically two numbers...
    class Complex{
    public:
        Number re,im;
        Complex();
        Complex(Number &&);
        Complex(const Number &);
        Complex(Complex &&);
        Complex(const Complex &);
        template<typename T1,typename T2>
        Complex(T1 &&_re,T2 &&_im):
            re(std::forward<T1>(_re)),
            im(std::forward<T2>(_im)){}

        //this=nan or inf, release allocated memory
        void clear();
        //this=-this
        void neg();
        //this.im=-this.im
        void conjugate();
        //non of Re/Im nan or inf
        bool is_num() const;
        void swap(Complex&);

        Complex &operator =(Number&&);
        Complex &operator =(const Number&);
        Complex &operator+=(const Number&);
        Complex &operator-=(const Number&);
        Complex &operator*=(const Number&);
        Complex &operator/=(const Number&);

        Complex &operator =(Complex&&);
        Complex &operator =(const Complex&);
        Complex &operator+=(const Complex&);
        Complex &operator-=(const Complex&);
        Complex &operator*=(const Complex&);
        Complex &operator/=(const Complex&);
    };

    Complex operator-(const Complex&);
    Complex operator+(const Complex&);
    Complex operator+(const Number&,const Complex&);
    Complex operator-(const Number&,const Complex&);
    Complex operator*(const Number&,const Complex&);
    Complex operator/(const Number&,const Complex&);
    Complex operator+(const Complex&,const Number&);
    Complex operator-(const Complex&,const Number&);
    Complex operator*(const Complex&,const Number&);
    Complex operator/(const Complex&,const Number&);
    Complex operator+(const Complex&,const Complex&);
    Complex operator-(const Complex&,const Complex&);
    Complex operator*(const Complex&,const Complex&);
    Complex operator/(const Complex&,const Complex&);

}
