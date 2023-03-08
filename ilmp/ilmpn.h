/*
	The implementation of this library is partially inspired by or adapted from

		GNU-MP (https://gmplib.org/), 
	
	especially some MACROS and masm codes.

	Note:
	UCASE_: dangerous macro used only in this .h, undef after use
	UCASE: safe macro
	_lcase_: temp var in macro
	ilmp_lcase_: dangerous function name/macro function name
	ilmp_lcase: safe function name/macro function name
*/

#ifndef ILMP_ILMPN_H
#define ILMP_ILMPN_H

#include<stdlib.h>
#include<string.h>
#include<malloc.h>
#include"ilmp.h"

#define DIV_DIVIDE_THRESHOLD            50
#define DIV_MULINV_L_THRESHOLD         477
#define DIV_MULINV_N_THRESHOLD        1736

#define INV_NEWTON_THRESHOLD            21
#define INV_MODM_THRESHOLD             734

#define DIV_MULINV_MODM_THRESHOLD      477

#define SQRT_NEWTON_THRESHOLD           50
#define SQRT_NEWTON_MODM_THRESHOLD     734

#define MUL_TOOM22_THRESHOLD            20
#define MUL_TOOMX2_THRESHOLD            30
#define MUL_TOOM33_THRESHOLD            65
#define MUL_FFT_THRESHOLD             1736

#define MUL_FFT_MODF_THRESHOLD         477

#define TO_STR_DIVIDE_THRESHOLD         20
#define TO_STR_BASEPOW_THRESHOLD        30
#define FROM_STR_DIVIDE_THRESHOLD       45
#define FROM_STR_BASEPOW_THRESHOLD     100



#define INLINE __inline
#define L1_CACHE_SIZE 8192

#define LIMB_BITS        64
#define LIMB_BYTES        8
#define LOG2_LIMB_BITS    6
#define LIMB_MAX   (~(mp_limb_t)0)
#define PART_SIZE (L1_CACHE_SIZE/LIMB_BYTES/4)

//minimum natural number k s.t. 2^k>x
int ilmp_limb_bits_(mp_limb_t x);
//number of leading zeros in a limb
int ilmp_leading_zeros_(mp_limb_t x);
//number of tailing zeros in a limb
int ilmp_tailing_zeros_(mp_limb_t x);
//return a*b/B
mp_limb_t ilmp_mulh_(mp_limb_t a,mp_limb_t b);

//used for toom33 and toom42
void ilmp_toom_interp5_(mp_ptr dst,mp_ptr v2,mp_ptr vm1,mp_size_t n,mp_size_t spt,int vm1_neg,mp_limb_t vinf0);

//[dst,2*na]=[numa,na]^2, need(na>0, sep(dst,numa))
void ilmp_sqr_basecase_(mp_ptr dst,mp_srcptr numa,mp_size_t na);
//[dst,2*na]=[numa,na]^2, need(na>0, sep(dst,numa), na>???)
void ilmp_sqr_toom2_(mp_ptr dst,mp_srcptr numa,mp_size_t na);
//[dst,2*na]=[numa,na]^2, need(na>0, sep(dst,numa), na>???)
void ilmp_sqr_toom3_(mp_ptr dst,mp_srcptr numa,mp_size_t na);
//[dst,na+nb]=[numa,na]*[numb,nb], need(0<nb<=na, sep(dst,[numa|numb]))
void ilmp_mul_basecase_(mp_ptr dst,mp_srcptr numa,mp_size_t na,mp_srcptr numb,mp_size_t nb);
//[dst,na+nb]=[numa,na]*[numb,nb], need(4/5<=nb/na<=1, nb>=5, sep(dst,[numa|numb]))
void ilmp_mul_toom22_(mp_ptr dst,mp_srcptr numa,mp_size_t na,mp_srcptr numb,mp_size_t nb);
//[dst,na+nb]=[numa,na]*[numb,nb], need(5/9<=nb/na<=4/5, nb>=12, sep(dst,[numa|numb]))
void ilmp_mul_toom32_(mp_ptr dst,mp_srcptr numa,mp_size_t na,mp_srcptr numb,mp_size_t nb);
//[dst,na+nb]=[numa,na]*[numb,nb], need(4/5<=nb/na<=1, nb>=26, sep(dst,[numa|numb]))
void ilmp_mul_toom33_(mp_ptr dst,mp_srcptr numa,mp_size_t na,mp_srcptr numb,mp_size_t nb);
//[dst,na+nb]=[numa,na]*[numb,nb], need(1/3<=nb/na<=5/9, nb>=20, sep(dst,[numa|numb]))
void ilmp_mul_toom42_(mp_ptr dst,mp_srcptr numa,mp_size_t na,mp_srcptr numb,mp_size_t nb);
//minimum feasible size for fermat/mersenne mul that >= n 
mp_size_t ilmp_fft_next_size_(mp_size_t n);
//[dst,rn+1]=[numa,na]*[numb,nb] mod B^rn+1, need(0<=[numa,na]<2*B^rn, 0<=[numb,nb]<2*B^rn)
void ilmp_mul_fermat_(mp_ptr dst,mp_size_t rn,mp_srcptr numa,mp_size_t na,mp_srcptr numb,mp_size_t nb);
//[dst,rn]=[numa,na]*[numb,nb] mod B^rn-1, need(0<=[numa,na]<B^rn, 0<=[numb,nb]<B^rn)
void ilmp_mul_mersenne_(mp_ptr dst,mp_size_t rn,mp_srcptr numa,mp_size_t na,mp_srcptr numb,mp_size_t nb);
//[dst,na+nb]=[numa,na]*[numb,nb], need(???<=nb<=na, sep(dst,[numa|numb]))
void ilmp_mul_fft_(mp_ptr dst,mp_srcptr numa,mp_size_t na,mp_srcptr numb,mp_size_t nb);

//inv1(x)=(B^2-1)/x-B, need(MSB(x)=1)
mp_limb_t ilmp_inv_1_(mp_limb_t x);
//inv21(xh:xl)=(B^3-1)/(xh*B+xl)-B, need(MSB(xh)=1)
mp_limb_t ilmp_inv_2_1_(mp_limb_t xh,mp_limb_t xl);
//[dst,na]=invappr([numa,na])=(B^(2*na)-1)/[numa,na]-B^na+[0|-1]
//need(na>0, MSB(numa)=1, sep(dst, numa))
void ilmp_invappr_(mp_ptr dst,mp_srcptr numa,mp_size_t na);
//[numa,2]=[numa,3] mod [numb,2], return quotient
//need([numa,3]<[numb,2]*B, MSB(numb)=1, inv21=inv21([numb,2]))
mp_limb_t ilmp_div_3_2_(mp_ptr numa,mp_srcptr numb,mp_limb_t inv21);
//if(dstq)[dstq,na]=[numa,na] div x, need(na>0, x!=0, eqsep(dstq,numa)), return remainder
//dstq>=numa-1 is acceptable
mp_limb_t ilmp_div_1_(mp_ptr dstq,mp_srcptr numa,mp_size_t na,mp_limb_t x);
//if(dstq)[dstq,na-1]=[numa,na] div [numb,2], 
//[numb,2]=[numa,na] mod [numb,2]
//need(na>=2, numb[1]!=0, eqsep(dstq,numa))
//dstq>=numa is acceptable
void ilmp_div_2_(mp_ptr dstq,mp_srcptr numa,mp_size_t na,mp_ptr numb);
//qh:[dstq,na-nb]=[numa,na] div [numb,nb], [numa,nb]=[numa,na] mod [numb,nb], return qh
//need(na>=nb>=3, MSB(numb)=1, inv21=inv21([numb+nb-2,2]), sep(dstq,numa,numb))
mp_limb_t ilmp_div_basecase_(mp_ptr dstq,mp_ptr numa,mp_size_t na,mp_srcptr numb,mp_size_t nb,mp_limb_t inv21);
//qh:[dstq,na-nb]=[numa,na] div [numb,nb], [numa,nb]=[numa,na] mod [numb,nb], return qh
//need(na>=nb*2, nb>=6, MSB(numb)=1, inv21=inv21([numb+nb-2,2]), sep(dstq,numa,numb))
mp_limb_t ilmp_div_divide_(mp_ptr dstq,mp_ptr numa,mp_size_t na,mp_srcptr numb,mp_size_t nb,mp_limb_t inv21);
//choose the size of precomputed inverse ni<=nb for a normalised [nq+nb]/[nb]=[nq] division
//need(nq>0, nb>0)
mp_size_t ilmp_div_inv_size_(mp_size_t nq,mp_size_t nb);
//1:[dst,ni]=invappr((ni+1 MSLs of numa)+1)/B
//need(na>=ni>0, MSB(numa)=1)
void ilmp_inv_prediv_(mp_ptr dst,mp_srcptr numa,mp_size_t na,mp_size_t ni);
//qh:[dstq,na-nb]=[numa,na] div [numb,nb], [numa,nb]=[numa,na] mod [numb,nb], return qh
//need(na>=nb>=ni>0, MSB(numb)=1, [invappr,ni]=invprediv([numb,nb]), sep(dstq,numa,numb,invappr))
mp_limb_t ilmp_div_mulinv_(mp_ptr dstq,mp_ptr numa,mp_size_t na,mp_srcptr numb,mp_size_t nb,mp_srcptr invappr,mp_size_t ni);
//qh:[dstq,na-1]=[numa,na] div x, [numa,1]=[numa,na] mod x, return qh
//need(na>1, MSB(x)=1, sep(dstq,numa))
mp_limb_t ilmp_div_1_s_(mp_ptr dstq,mp_ptr numa,mp_size_t na,mp_limb_t x);
//qh:[dstq,na-2]=[numa,na] div [numb,2], [numa,2]=[numa,na] mod [numb,2], return qh
//need(na>2, MSB(numb)=1, sep(dstq,numa,numb))
mp_limb_t ilmp_div_2_s_(mp_ptr dstq,mp_ptr numa,mp_size_t na,mp_srcptr numb);
//qh:[dstq,na-nb]=[numa,na] div [numb,nb], [numa,nb]=[numa,na] mod [numb,nb], return qh
//need(na>=nb>0, MSB(numb)=1, sep(dstq,numa,numb))
mp_limb_t ilmp_div_s_(mp_ptr dstq,mp_ptr numa,mp_size_t na,mp_srcptr numb,mp_size_t nb);

typedef struct ilmp_mp_base_t_{
	//the largest power of base fits in a limb
	//except for base=2,4,8,16,...,256, large_base=log2(base)
	mp_limb_t large_base;
	//ceiling(2^64*log2(base)/log2(2^64))
	//an n-digit number will have at most n*lg_base/2^64+1 limbs
	mp_limb_t lg_base;
	//ceiling(2^64/log2(base))
	//except for base=2, lg_base=2^64-1
	//an n-bit number will have at most n*inv_lg_base/2^64+1 digits
	mp_limb_t inv_lg_base;
	//floor(64/log2(base))
	//the largest number of digits fits in a limb
	int digits_in_limb;
	//2<=base<=256
	int base;
} mp_base_t;

extern const mp_base_t ilmp_bases_[257];

typedef struct ilmp_mp_basepow_t_{
	//base^digits, zeros trimmed, maybe normalized if invp is given
	mp_ptr p;
	mp_size_t np;
	//inv of normalized p if needed
	mp_ptr invp;
	mp_size_t ni;
	//number of trimmed zero limbs
	mp_size_t zeros;
	//logb(p)
	mp_size_t digits;
	//number of bits shifted, if p is normalized
	int norm_cnt;
	//base
	int base;
} mp_basepow_t;




#define ABS(x)   ((x)>=0?(x):-(x))
#define MIN(l,o) ((l)<(o)?(l):(o))
#define MAX(h,i) ((h)>(i)?(h):(i))
#define SWAP(x,y,type)       \
	do{                      \
		type _swap_=(x);     \
		(x)=(y);             \
		(y)=_swap_;          \
	}while(0)
#define POW2_Q(n) (((n)&(n)-1)==0)
#define ROUND_UP_MULTIPLE(a,m) ((a)+(m)-1-((a)+(m)-1)%(m))

void *ilmp_temp_alloc_(void **,size_t);
void  ilmp_temp_free_(void *);

#define TEMP_DECL void *ilmp_temp_alloc_marker_=0
#define TEMP_SALLOC(n) alloca(n)
#define TEMP_BALLOC(n) ilmp_temp_alloc_(&ilmp_temp_alloc_marker_,(n))
#define TEMP_TALLOC(n) ((n)<=0x7f00?TEMP_SALLOC(n):TEMP_BALLOC(n))
#define SALLOC_TYPE(n,type) ((type *)TEMP_SALLOC((n)*sizeof(type)))
#define BALLOC_TYPE(n,type) ((type *)TEMP_BALLOC((n)*sizeof(type)))
#define TALLOC_TYPE(n,type) ((type *)TEMP_TALLOC((n)*sizeof(type)))
#define TEMP_FREE                                      \
	do{                                                \
		if(ilmp_temp_alloc_marker_)                    \
			ilmp_temp_free_(ilmp_temp_alloc_marker_);  \
	}while(0)

#define ALLOC_TYPE(n,type) ((type *)ilmp_alloc((n)*sizeof(type)))
#define REALLOC_TYPE(p,new_size,type) ((type *)ilmp_realloc((p),(new_size)*sizeof(type)))
#define FREE(ptr) ilmp_free(ptr)

#define ilmp_copy(dst,src,n) memmove(dst,src,(n)<<3)
#define ilmp_zero(dst,n) memset(dst,0,(n)<<3)
#define ilmp_assert(x)            \
	do{                           \
		if(!(x))*(mp_limb_t*)0=0; \
	}while(0)

//[p]+=1, expecting no carry
#define ilmp_inc(p)                   \
	do{                               \
		mp_ptr _p_=(p);               \
		while(++(*(_p_++))==0);       \
	}while(0)
//[p]+=inc, expecting no carry
#define ilmp_inc_1(p,inc)             \
	do{                               \
		mp_ptr _p_=(p);               \
		mp_limb_t _inc_=(inc),_x_;    \
		_x_=*_p_+_inc_;               \
		*_p_=_x_;                     \
		if(_x_<_inc_)                 \
			while(++(*(++_p_))==0);   \
	}while(0)
//[p]-=1, expecting no borrow
#define ilmp_dec(p)                   \
	do{                               \
		mp_ptr _p_=(p);               \
		while((*(_p_++))--==0);       \
	}while(0)
//[p]-=dec, expecting no borrow
#define ilmp_dec_1(p,dec)             \
	do{                               \
		mp_ptr _p_=(p);               \
		mp_limb_t _dec_=(dec),_x_;    \
		_x_=*_p_;                     \
		*_p_=_x_-_dec_;               \
		if(_x_<_dec_)                 \
			while((*(++_p_))--==0);   \
	}while(0)

INLINE int ilmp_cmp_(mp_srcptr numa,mp_srcptr numb,mp_size_t n){
	mp_ssize_t i=n;
	mp_limb_t x,y;
	while(--i>=0){
		x=numa[i];
		y=numb[i];
		if(x!=y)return (x>y?1:-1);
	}
	return 0;
}
INLINE int ilmp_zero_q_(mp_srcptr p,mp_size_t n){
	do{
		if(p[--n]!=0)return 0;
	}while(n!=0);
	return 1;
}

#define ILMP_AORS_(FUNCTION,TEST)           \
	mp_limb_t _x_;                          \
	if(FUNCTION(dst,numa,numb,nb)){         \
		do{                                 \
			if(nb>=na)return 1;             \
			_x_=numa[nb];                   \
		}                                   \
		while(TEST);                        \
	}                                       \
	if(dst!=numa&&na!=nb)                   \
		ilmp_copy(dst+nb,numa+nb,na-nb);    \
	return 0
INLINE mp_limb_t ilmp_add_(mp_ptr dst,mp_srcptr numa,mp_size_t na,mp_srcptr numb,mp_size_t nb){
	ILMP_AORS_(ilmp_add_n_,((dst[nb++]=_x_+1)==0));
}
INLINE mp_limb_t ilmp_sub_(mp_ptr dst,mp_srcptr numa,mp_size_t na,mp_srcptr numb,mp_size_t nb){
	ILMP_AORS_(ilmp_sub_n_,((dst[nb++]=_x_-1),_x_==0));
}
#undef ILMP_AORS_

#define ILMP_AORS_1_(OP,CB)                 \
	mp_size_t _i_=1;                        \
	mp_limb_t _x_=numa[0],_r_=_x_ OP x;     \
	dst[0]=_r_;                             \
	if(CB(_r_,_x_,x)){                      \
		do{                                 \
			if(_i_>=na)return 1;            \
			_x_=numa[_i_];                  \
			_r_=_x_ OP 1;                   \
			dst[_i_]=_r_;                   \
			++_i_;                          \
		}while(CB(_r_,_x_,1));              \
	}                                       \
	if(numa!=dst&&na!=_i_)                  \
		ilmp_copy(dst+_i_,numa+_i_,na-_i_); \
	return 0
#define ILMP_ADDCB_(r,x,y) ((r)<(y))
#define ILMP_SUBCB_(r,x,y) ((x)<(y))
INLINE mp_limb_t ilmp_add_1_(mp_ptr dst,mp_srcptr numa,mp_size_t na,mp_limb_t x){
	ILMP_AORS_1_(+,ILMP_ADDCB_);
}
INLINE mp_limb_t ilmp_sub_1_(mp_ptr dst,mp_srcptr numa,mp_size_t na,mp_limb_t x){
	ILMP_AORS_1_(-,ILMP_SUBCB_);
}
#undef ILMP_ADDCB_
#undef ILMP_SUBCB_
#undef ILMP_AORS_1_

INLINE mp_size_t ilmp_digits_(mp_srcptr numa,mp_size_t na,int base){
	int mslbits=0;
	if(numa){
		do{
			if(na==0)return 1;
		}while(numa[--na]==0);
		mslbits=ilmp_limb_bits_(numa[na]);
	}
	return ilmp_mulh_(na*64+mslbits,ilmp_bases_[base].inv_lg_base)+1;
}

INLINE mp_size_t ilmp_limbs_(const mp_byte_t *src,mp_size_t len,int base){
	if(src){
		do{
			if(len==0)return 1;
		}while(src[--len]==0);
		++len;
	}
	return ilmp_mulh_(len,ilmp_bases_[base].lg_base)+1;
}

#endif //ILMP_ILMPN_H
