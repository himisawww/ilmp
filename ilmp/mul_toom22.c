#include"ilmpn.h"

/*

Evaluate in: -1, 0, +inf

   <-s--><--n-->
   |-a1-|--a0--|
    |b1-|--b0--|
    <-t-><--n-->
	
v0  =  a0    * b0      #   A(0)*B(0)
vm1 = (a0-a1)*(b0-b1)  #  A(-1)*B(-1)
vinf=     a1 *    b1   # A(inf)*B(inf)

*/


void ilmp_mul_toom22_(mp_ptr dst,mp_srcptr numa,mp_size_t na,mp_srcptr numb,mp_size_t nb){
	mp_size_t s=na>>1,n=na-s,t=nb-n;
	mp_limb_t *vm1=SALLOC_TYPE(2*n,mp_limb_t);
	int vm1_neg=0;
	mp_slimb_t cy,cy2;

#define a0 numa
#define a1 (numa+n)
#define b0 numb
#define b1 (numb+n)
#define asm1 dst
#define bsm1 (dst+n)

	if(s==n){
		if(ilmp_cmp_(a0,a1,n)<0){
			ilmp_sub_n_(asm1,a1,a0,n);
			vm1_neg=1;
		}
		else ilmp_sub_n_(asm1,a0,a1,n);
    }
	else{//s==n-1
		if(a0[s]==0&&ilmp_cmp_(a0,a1,s)<0){
			ilmp_sub_n_(asm1,a1,a0,s);
			asm1[s]=0;
			vm1_neg=1;
		}
		else asm1[s]=a0[s]-ilmp_sub_n_(asm1,a0,a1,s);
	}
	
	if(t==n){
		if(ilmp_cmp_(b0,b1,n)<0){
			ilmp_sub_n_(bsm1,b1,b0,n);
			vm1_neg^=1;
		}
		else ilmp_sub_n_(bsm1,b0,b1,n);
	}
	else{
		if(ilmp_zero_q_(b0+t,n-t)&&ilmp_cmp_(b0,b1,t)<0){
			ilmp_sub_n_(bsm1,b1,b0,t);
			ilmp_zero(bsm1+t,n-t);
			vm1_neg^=1;
		}
		else ilmp_sub_(bsm1,b0,n,b1,t);
	}

	ilmp_mul_n_(vm1,asm1,bsm1,n);

#undef asm1
#undef bsm1
#define v0 dst
#define vinf (dst+2*n)

	ilmp_mul_n_(v0,a0,b0,n);

	ilmp_mul_(vinf,a1,s,b1,t);

	cy=ilmp_add_n_(dst+2*n,v0+n,vinf,n);
	cy2=cy+ilmp_add_n_(dst+n,dst+2*n,v0,n);
	cy+=ilmp_add_(dst+2*n,dst+2*n,n,vinf+n,s+t-n);

	if(vm1_neg)cy+=ilmp_add_n_(dst+n,dst+n,vm1,2*n);
	else cy-=ilmp_sub_n_(dst+n,dst+n,vm1,2*n);

	//no overflow, if s+t>n. proved.
	ilmp_inc_1(dst+2*n,cy2);

	if(cy<0)ilmp_dec(dst+3*n);
	else ilmp_inc_1(dst+3*n,cy);

}
