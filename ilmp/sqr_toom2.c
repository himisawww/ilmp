#include"ilmpn.h"

/*

Evaluate in: -1, 0, +inf

   <-s--><--n-->
   |-a1-|--a0--|
	
v0  =  a0    ^2  #   A(0)^2
vm1 = (a0-a1)^2  #  A(-1)^2
vinf=     a1 ^2  # A(inf)^2

*/


void ilmp_sqr_toom2_(mp_ptr dst,mp_srcptr numa,mp_size_t na){
	mp_size_t s=na>>1,n=na-s;
	mp_limb_t *vm1=SALLOC_TYPE(2*n,mp_limb_t);
	mp_slimb_t cy,cy2;

#define a0 numa
#define a1 (numa+n)
#define asm1 dst

	if(s==n){
		if(ilmp_cmp_(a0,a1,n)<0)
			ilmp_sub_n_(asm1,a1,a0,n);
		else
			ilmp_sub_n_(asm1,a0,a1,n);
    }
	else{//s==n-1
		if(a0[s]==0&&ilmp_cmp_(a0,a1,s)<0){
			ilmp_sub_n_(asm1,a1,a0,s);
			asm1[s]=0;
		}
		else asm1[s]=a0[s]-ilmp_sub_n_(asm1,a0,a1,s);
	}
	
	ilmp_sqr_(vm1,asm1,n);

#undef asm1
#define v0 dst
#define vinf (dst+2*n)

	ilmp_sqr_(v0,a0,n);

	ilmp_sqr_(vinf,a1,s);

	cy=ilmp_add_n_(dst+2*n,v0+n,vinf,n);
	cy2=cy+ilmp_add_n_(dst+n,dst+2*n,v0,n);
	cy+=ilmp_add_(dst+2*n,dst+2*n,n,vinf+n,s+s-n);

	cy-=ilmp_sub_n_(dst+n,dst+n,vm1,2*n);

	//no overflow.
	ilmp_inc_1(dst+2*n,cy2);

	if(cy<0)ilmp_dec(dst+3*n);
	else ilmp_inc_1(dst+3*n,cy);

}
