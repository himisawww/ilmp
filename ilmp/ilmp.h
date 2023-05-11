#ifndef IL_MP_VERSION
#define IL_MP_VERSION 1

/*************************************

	Notations:

	B			internal base of ilmp, always be 2^64

	[p,n,b]		an n-digit number in base b which is pointed by p
				p[i-1] is its i-th least significant digit (0<i<=n)
				if b is omitted, it is B by default

	sep			separate
				
	eqsep		equal or separate

	[x|y]		x or y

**************************************/

#include<stdint.h>

struct ilmp_compile_time_error_{
	char error_sizeof_pointer_should_be_8[sizeof(void *)==8?1:-1];
};

#if defined ILMP_NO_TYPES
#define mp_byte_t   uint8_t
#define mp_limb_t   uint64_t
#define mp_size_t   uint64_t
#define mp_slimb_t  int64_t
#define mp_ssize_t  int64_t
#define mp_ptr      mp_limb_t *
#define mp_srcptr   const mp_limb_t *
#else
typedef uint8_t mp_byte_t;
typedef uint64_t mp_limb_t;
typedef uint64_t mp_size_t;
typedef int64_t mp_slimb_t;
typedef int64_t mp_ssize_t;
typedef mp_limb_t *mp_ptr;
typedef const mp_limb_t *mp_srcptr;
#endif

#ifdef __cplusplus
extern "C"{
#endif


// ilmp safe functions

void *ilmp_alloc(size_t);
void *ilmp_realloc(void *,size_t);
void  ilmp_free(void *);


// ilmp_ unsafe functions

//[dst,n]=[numa,n]+[numb,n]+c, need(c=[0|1], n>0, eqsep(dst,[numa|numb])), return c=[0|1]
mp_limb_t ilmp_add_nc_(mp_ptr dst,mp_srcptr numa,mp_srcptr numb,mp_size_t n,mp_limb_t c);
//[dst,n]=[numa,n]+[numb,n], need(n>0, eqsep(dst,[numa|numb])), return c=[0|1]
mp_limb_t ilmp_add_n_(mp_ptr dst,mp_srcptr numa,mp_srcptr numb,mp_size_t n);
//[dst,n]=[numa,n]-[numb,n]-c, need(c=[0|1], n>0, eqsep(dst,[numa|numb])), return c=[0|1]
mp_limb_t ilmp_sub_nc_(mp_ptr dst,mp_srcptr numa,mp_srcptr numb,mp_size_t n,mp_limb_t c);
//[dst,n]=[numa,n]-[numb,n], need(n>0, eqsep(dst,[numa|numb])), return c=[0|1]
mp_limb_t ilmp_sub_n_(mp_ptr dst,mp_srcptr numa,mp_srcptr numb,mp_size_t n);
//([dsta,n],[dstb,n])=([numa,n]+[numb,n],[numa,n]-[numb,n])
//need(n>0, eqsep(dsta,[numa|numb]), eqsep(dstb,[numa|numb]))
//return cb=2*c+b=[0|1|2|3]
mp_limb_t ilmp_add_n_sub_n_(mp_ptr dsta,mp_ptr dstb,mp_srcptr numa,mp_srcptr numb,mp_size_t n);

//[dst,n]=[numa,n]+[numb,n]>>1, need(n>0, eqsep(dst,[numa|numb])), return shift's carry=[0|1]
mp_limb_t ilmp_shr1add_n_(mp_ptr dst,mp_srcptr numa,mp_srcptr numb,mp_size_t n);
//[dst,n]=[numa,n]+[numb,n]+c>>1, return shift's carry=[0|1]
//need(n>0, c=[0|1], eqsep(dst,[numa|numb]))
mp_limb_t ilmp_shr1add_nc_(mp_ptr dst,mp_srcptr numa,mp_srcptr numb,mp_size_t n,mp_limb_t c);
//[dst,n]=[numa,n]-[numb,n]>>1, need(n>0, eqsep(dst,[numa|numb])), return shift's carry=[0|1]
mp_limb_t ilmp_shr1sub_n_(mp_ptr dst,mp_srcptr numa,mp_srcptr numb,mp_size_t n);
//[dst,n]=[numa,n]-[numb,n]-c>>1, return shift's carry=[0|1]
//need(n>0, c=[0|1], eqsep(dst,[numa|numb]))
mp_limb_t ilmp_shr1sub_nc_(mp_ptr dst,mp_srcptr numa,mp_srcptr numb,mp_size_t n,mp_limb_t c);

//[dst,na]=[numa,na]>>shr, shr-MSBs of [dst,na] filled by 0
//need(na>0, 0<=shr<64, eqsep(dst,numa))
//dst<numa is acceptable, i.e. on-site long shr
//shr-MSBs of returned limb filled by [numa,na]'s shr-LSBs, the rest bits are 0
mp_limb_t ilmp_shr_(mp_ptr dst,mp_srcptr numa,mp_size_t na,mp_size_t shr);
//[dst,na]=[numa,na]>>shr, shr-MSBs of [dst,na] filled by shr-MSBs of c
//need(na>0, 0<=shr<64, eqsep(dst,numa), (64-shr)-LSBs of c are 0)
//dst<numa is acceptable, i.e. on-site long shr
//shr-MSBs of returned limb filled by [numa,na]'s shr-LSBs, the rest bits are 0
mp_limb_t ilmp_shr_c_(mp_ptr dst,mp_srcptr numa,mp_size_t na,mp_size_t shr,mp_limb_t c);
//[dst,na]=[numa,na]<<shl, shl-LSBs of [dst,na] filled by 0
//need(na>0, 0<=shl<64, eqsep(dst,numa))
//dst>numa is acceptable, i.e. on-site long shl
//shl-LSBs of returned limb filled by [numa,na]'s shl-MSBs, the rest bits are 0
mp_limb_t ilmp_shl_(mp_ptr dst,mp_srcptr numa,mp_size_t na,mp_size_t shl);
//[dst,na]=[numa,na]<<shl, shl-LSBs of [dst,na] filled by shl-LSBs of c
//need(na>0, 0<=shl<64, eqsep(dst,numa), (64-shl)-MSBs of c are 0)
//dst>numa is acceptable, i.e. on-site long shl
//shl-LSBs of returned limb filled by [numa,na]'s shl-MSBs, the rest bits are 0
mp_limb_t ilmp_shl_c_(mp_ptr dst,mp_srcptr numa,mp_size_t na,mp_size_t shl,mp_limb_t c);
//[dst,na]=~[numa,na], need(na>0, eqsep(dst,numa))
void ilmp_not_(mp_ptr dst,mp_srcptr numa,mp_size_t na);
//[dst,na]=~([numa,na]<<shl), shl-LSBs of [dst,na] filled by 1
//need(na>0, 0<=shl<64, eqsep(dst,numa))
//shl-LSBs of returned limb filled by [numa,na]'s shl-MSBs, the rest bits are 0
mp_limb_t ilmp_shlnot_(mp_ptr dst,mp_srcptr numa,mp_size_t na,mp_size_t shl);

//[dst,n]=[numa,n]+([numb,n]<<1), need(n>0, eqsep(dst,[numa|numb])), return c=[0|1|2]
mp_limb_t ilmp_addshl1_n_(mp_ptr dst,mp_srcptr numa,mp_srcptr numb,mp_size_t n);
//[dst,n]=[numa,n]-([numb,n]<<1), need(n>0, eqsep(dst,[numa|numb])), return c=[0|1|2]
mp_limb_t ilmp_subshl1_n_(mp_ptr dst,mp_srcptr numa,mp_srcptr numb,mp_size_t n);

//[numa,n]+=[numb,n]*B, need(n>0, eqsep(numa,numb)), return carry limb
mp_limb_t ilmp_addmul_1_(mp_ptr numa,mp_srcptr numb,mp_size_t n,mp_limb_t B);
//[numa,n]-=[numb,n]*B, need(n>0, eqsep(numa,numb)), return borrow limb
mp_limb_t ilmp_submul_1_(mp_ptr numa,mp_srcptr numb,mp_size_t n,mp_limb_t B);

//[dst,2*na]=[numa,na]^2, need(na>0, sep(dst,numa))
void ilmp_sqr_(mp_ptr dst,mp_srcptr numa,mp_size_t na);
//[dst,na]=[numa,na]*x, return carry limb, need(na>0, eqsep(dst,numa))
//dst<=numa+1 is acceptable
mp_limb_t ilmp_mul_1_(mp_ptr dst,mp_srcptr numa,mp_size_t na,mp_limb_t x);
//[dst,2*n]=[numa,n]*[numb,n], need(n>0, sep(dst,[numa|numb]))
//if(n==1)dst<=numa+1 is acceptable
//if(n==2)dst<=numa is acceptable
void ilmp_mul_n_(mp_ptr dst,mp_srcptr numa,mp_srcptr numb,mp_size_t n);
//[dst,na+nb]=[numa,na]*[numb,nb], need(0<nb<=na, sep(dst,[numa|numb]))
//if(nb==1)dst<=numa+1 is acceptable
//if(nb==2)dst<=numa is acceptable
void ilmp_mul_(mp_ptr dst,mp_srcptr numa,mp_size_t na,mp_srcptr numb,mp_size_t nb);
//if(dstq)[dstq,na-nb+1]=[numa,na] div [numb,nb]
//if(dstr)[dstr,nb]=[numa,na] mod [numb,nb]
//need(0<nb<=na, numb[nb-1]!=0, sep(dstq,[numa|numb]), eqsep(dstr,[numa|numb]))
//if(nb==1)dstq>=numa-1 is acceptable
//if(nb==2)dstq>=numa is acceptable
void ilmp_div_(mp_ptr dstq,mp_ptr dstr,mp_srcptr numa,mp_size_t na,mp_srcptr numb,mp_size_t nb);
//if(dstr)[dsts,nf+na/2+1],[dstr,nf+na/2+1]=sqrtrem([numa,na]*B^(2*nf))
//else [dsts,nf+na/2+1]=[floor|round](sqrt([numa,na]*B^(2*nf)))
//need(na>0, numa[na-1]!=0, eqsep(dsts,numa), eqsep(dstr,numa))
void ilmp_sqrt_(mp_ptr dsts,mp_ptr dstr,mp_srcptr numa,mp_size_t na,mp_size_t nf);
//[dst,na+nf+1]=(B^(2*(na+nf))-1)/([numa,na]*B^nf)+[0|-1]
//need(na>0, numa[na-1]!=0, eqsep(dst, numa))
void ilmp_inv_(mp_ptr dst,mp_srcptr numa,mp_size_t na,mp_size_t nf);

//convert [src,len,base] to [dst,return value,B]
//need(len>=0, 2<=base<=256)
mp_size_t ilmp_from_str_(mp_ptr dst,const mp_byte_t *src,mp_size_t len,int base);
//convert [numa,na,B] to [dst,return value,base]
//need(na>=0, 2<=base<=256)
mp_size_t ilmp_to_str_(mp_byte_t *dst,mp_srcptr numa,mp_size_t na,int base);

// _ilmp exported inline functions

//return [1|0|-1] in case of [numa,n][>|=|<][numb,n], need(n>0).
int ilmp_cmp_(mp_srcptr numa,mp_srcptr numb,mp_size_t n);
//return [p,n]==0, need(n>0).
int ilmp_zero_q_(mp_srcptr p,mp_size_t n);
//[dst,na]=[numa,na]+[numb,nb], need(na>=nb>0, eqsep(dst,[numa|numb])), return c=[0|1].
mp_limb_t ilmp_add_(mp_ptr dst,mp_srcptr numa,mp_size_t na,mp_srcptr numb,mp_size_t nb);
//[dst,na]=[numa,na]-[numb,nb], need(na>=nb>0, eqsep(dst,[numa|numb])), return c=[0|1].
mp_limb_t ilmp_sub_(mp_ptr dst,mp_srcptr numa,mp_size_t na,mp_srcptr numb,mp_size_t nb);
//[dst,na]=[numa,na]+x, need(na>0, eqsep(dst,numa)), return c=[0|1].
mp_limb_t ilmp_add_1_(mp_ptr dst,mp_srcptr numa,mp_size_t na,mp_limb_t x);
//[dst,na]=[numa,na]-x, need(na>0, eqsep(dst,numa)), return c=[0|1].
mp_limb_t ilmp_sub_1_(mp_ptr dst,mp_srcptr numa,mp_size_t na,mp_limb_t x);

//if(src)return number of limbs of [src,len,base], maybe 1 greater than exact value
//if(!src)return max possible limbs of a len-digit integer
//need(len>=0, 2<=base<=256)
mp_size_t ilmp_limbs_(const mp_byte_t *src,mp_size_t len,int base);
//if(numa)return number of digits of [numa,na], maybe 1 greater than exact value
//if(!numa)return max possible digits of an na-limb integer
//need(na>=0, 2<=base<=256)
mp_size_t ilmp_digits_(mp_srcptr numa,mp_size_t na,int base);

#ifdef __cplusplus
}//extern "C"
#endif

#if defined ILMP_NO_TYPES
#undef mp_byte_t
#undef mp_limb_t
#undef mp_size_t
#undef mp_slimb_t
#undef mp_ssize_t
#undef mp_ptr
#undef mp_srcptr
#endif

#endif//IL_MP_VERSION
