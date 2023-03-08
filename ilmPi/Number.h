#pragma once
#include<stdint.h>
#define ILMP_NO_TYPES
#include"../ilmp/ilmp.h"
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

	//when input a float value without prec specifier,
	const mp_int MIN_PREC_BITS=32;
	//when exp>=this value, use scientific format to print
	const mp_int MIN_EXP_SCI_OUT=6;
	//max representable int in mp_prec_t
	const mp_int MAX_PREC_INT=(mp_int)1<<53;
	//what is overflow, at least >>3
	const mp_int MAX_EXP_BITS=MAX_PREC_INT>>20;
	const mp_int MAX_EXP_LIMBS=MAX_EXP_BITS>>6;

	const mp_uint MP_MAX_UINT=-1;
	const mp_int MP_MAX_INT=MP_MAX_UINT>>1;
	const mp_int MP_MIN_INT=-1-MP_MAX_INT;

	const mp_int MP_LIMB_BITS=64;

	const mp_limb_t INT_PINF=0x7FF0000000000000;
	const mp_limb_t INT_MINF=0xFFF0000000000000;
	const mp_prec_t INT_PREC=*(mp_prec_t*)&INT_PINF;
	const mp_prec_t NAN_PREC=*(mp_prec_t*)&INT_MINF;
	const mp_prec_t MAX_PREC_BITS=MAX_EXP_BITS;
	const mp_prec_t MAX_PREC_LIMBS=MAX_EXP_LIMBS;
	const mp_prec_t MP_BASE=18446744073709551616.;

/*
   Special value:
         NaN = {null,0}
		   0 = {data,0}

   Normal values:
     sign*[data,size]
	         = {data,sign*size}
   //  sign=[-1|1], data[size-1]!=0

Note:
  * if data==value, value[2] stores the value
  * else alloc is # of limbs allocated and pointed to by data
  * Integer is faster than Number for small integers
*/
	class Integer{
	public:
		mp_ptr data;
		mp_int ssize;
		union{
			mp_limb_t value[2];
			mp_int alloc;
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
		Integer(Integer &&);
		Integer(const Integer&);
		Integer(const Number&);
		Integer(const char*,int base=10);
		~Integer();

		//this=NaN
		void clear();
		void from_int(mp_int);
		void from_uint(mp_uint);
		//[+-][N]
		void from_str(const char*,int base=10);

		//result may mod by 2^64
		mp_int to_int() const;
		//result may mod by 2^64
		mp_uint to_uint() const;
		//buffer length needed to store this as a string in a given base
		mp_int strlen(int base=10) const;
		//convert this into a string in a given base
		void to_str(char*,int base=10) const;

		//this=-this
		void neg();
		//-1 for negative,0 for 0 and nan,+1 for positive
		int sign() const;
		//# of limbs in use to store this
		mp_int size() const;
		//# of limbs pointed by data
		mp_int capacity() const;
		//log2(this*2^shift)
		mp_prec_t log2(mp_int shift=0) const;
		bool is_nan() const;
		void swap(Integer&);

		Integer &operator =(Integer&&);
		Integer &operator =(const Integer&);
		Integer &operator+=(const Integer&);
		Integer &operator-=(const Integer&);
		Integer &operator*=(const Integer&);
		Integer &operator/=(const Integer&);
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
/*
   Special values:
         NaN = {null, 0,0,NAN_PREC}
		+Inf = {null, 1,0,NAN_PREC}
		-Inf = {null,-1,0,NAN_PREC}

   Zeros:
           0 = {null, 0,0,INT_PREC}
   0.*^-prec = {null, 0,0,prec};
   //* -MAX_EXP_BITS <= prec < MAX_EXP_BITS

   Normal values:
     sign*[data,size]*2^(-64*dotp)
	         = {data,sign*size,dotp,prec}
   //  sign=[-1|1], data[size-1]!=0
   //  prec == INT_PREC and dotp <= 0 for int value
   //* 0 < prec <= MAX_PREC_BITS for float value
   //* -MAX_EXP_LIMBS < size-dotp <= MAX_EXP_LIMBS

Note:
  * overprec or overflow values can violate //*'s
  * prec is in bits.
*/
	class Number{
	public:
		mp_ptr data;
		mp_int ssize;
		mp_int dotp;
		mp_prec_t prec;
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
		Number(long double);
		Number(Integer &&);
		Number(const Integer &);
		Number(Number &&);
		Number(const Number&);
		Number(const char*,int base=10);
		~Number();

		//this=NaN
		void clear();
		void from_int(mp_int);
		void from_uint(mp_uint);
		//from_float(&fvalue,sizeof(fvalue));
		void from_float(const void*,mp_int tsize);
		//[+-][N.N][`[+-]prec][*^[+-]exponent]
		void from_str(const char*,int base=10);

		//result may be rounded or mod by 2^64
		mp_int to_int() const;
		//result may be rounded or mod by 2^64
		mp_uint to_uint() const;
		//to_float(&fvalue,sizeof(fvalue));
		void to_float(void*,mp_int tsize) const;
		//buffer length needed to store this as a string in a given base
		mp_int strlen(int base=10) const;
		//convert this into a string in a given base
		void to_str(char*,int base=10) const;

		//this=-this
		void neg();
		//-1 for negative,0 for 0 and nan,+1 for positive
		int sign() const;
		//# of limbs in use to store this
		mp_int size() const;
		//precision in # of limbs
		mp_int precision_size() const;
		//log2(this*2^shift)
		mp_prec_t log2(mp_int shift=0) const;
		//precision in bits, if this=0, returns accuracy
		mp_prec_t precision() const;
		//set to INT_PREC or +inf will fix this to integer
		void set_precision(mp_prec_t);
		//if is inf, return sign of inf
		//else return 0
		int is_inf() const;
		bool is_nan() const;
		bool is_int() const;
		void swap(Number&);
		//if overprec or overflow
		//if size too long compared to prec
		void normalize();

		Number &operator =(Number&&);
		Number &operator =(const Number&);
		Number &operator+=(const Number&);
		Number &operator-=(const Number&);
		Number &operator*=(const Number&);
		Number &operator/=(const Number&);
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

	Number sqrt(const Number&);
	Number pow(const Number&,mp_int);

	Number Pi(mp_uint digits,int base);
}
