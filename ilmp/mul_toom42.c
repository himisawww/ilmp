#include"ilmpn.h"

/*

Evaluate in: -1, 0, +1, +2, +inf

  <-s-><--n--><--n--><--n-->
  |a3-|---a2-|---a1-|---a0-|
               |-b1-|---b0-|
               <-t--><--n-->

v0  =  a0             * b0      #   A(0)*B(0)
v1  = (a0+ a1+ a2+ a3)*(b0+ b1) #   A(1)*B(1)      ah  <= 3  bh <= 1
vm1 = (a0- a1+ a2- a3)*(b0- b1) #  A(-1)*B(-1)    |ah| <= 1  bh  = 0
v2  = (a0+2a1+4a2+8a3)*(b0+2b1) #   A(2)*B(2)      ah  <= 14 bh <= 2
vinf=              a3 *     b1  # A(inf)*B(inf)

*/

void ilmp_mul_toom42_(mp_ptr dst,mp_srcptr numa, mp_size_t na,mp_srcptr numb, mp_size_t nb){
	mp_size_t n=na>=2*nb?na+3>>2:nb+1>>1,s=na-3*n,t=nb-n;
	int vm1_neg;
	mp_limb_t cy,vinf0,am1h;
	mp_limb_t *tp=SALLOC_TYPE(4*n+4,mp_limb_t);
	
#define a0    numa
#define a1    (numa+n)
#define a2    (numa+2*n)
#define a3    (numa+3*n)
#define b0    numb
#define b1    (numb+n)

#define v0    dst          //[dst,2*n]
#define v1    (dst+2*n)    //[dst+2*n,2*n+1]
#define vinf  (dst+4*n)    //[dst+4*n,s+t]
#define vm1   tp           //[tp,2*n+1]
#define v2    (tp+2*n+2)   //[tp+2*n+2,2*n+1]

#define bm1   dst          //[dst,n]
#define am1   (dst+n)      //[dst+n,n+1]
#define ap1   tp           //[tp,n+1]
#define bp1   (tp+n+1)     //[tp+n+1,n+1]
#define ap2   ap1          //same space
#define bp2   bp1          //same space
#define a13   bp1          //temporary use

	//ap1,am1
	ap1[n]=ilmp_add_n_(ap1,a0,a2,n);
	a13[n]=ilmp_add_(a13,a1,n,a3,s);
	vm1_neg=ilmp_cmp_(ap1,a13,n+1)<0;
	if(vm1_neg)ilmp_add_n_sub_n_(ap1,am1,a13,ap1,n+1);
	else ilmp_add_n_sub_n_(ap1,am1,ap1,a13,n+1);
	am1h=am1[n];//overlap with v1

	//bp1,bm1
	if(t==n){
		if(ilmp_cmp_(b0,b1,n)<0){
			bp1[n]=ilmp_add_n_sub_n_(bp1,bm1,b1,b0,n)>>1;
			vm1_neg^=1;
		}
		else{
			bp1[n]=ilmp_add_n_sub_n_(bp1,bm1,b0,b1,n)>>1;
		}
	}
	else{
		if(ilmp_zero_q_(b0+t,n-t)&&ilmp_cmp_(b0,b1,t)<0){
			cy=ilmp_add_n_sub_n_(bp1,bm1,b1,b0,t);
			ilmp_zero(bm1+t,n-t);
			vm1_neg^=1;
		}
		else{
			cy=ilmp_add_n_sub_n_(bp1,bm1,b0,b1,t);
			ilmp_sub_1_(bm1+t,b0+t,n-t,cy&1);
		}
		bp1[n]=ilmp_add_1_(bp1+t,b0+t,n-t,cy>>1);
    }
	
	//vinf=a3*b1
	if(s>t)ilmp_mul_(vinf,a3,s,b1,t);
	else ilmp_mul_(vinf,b1,t,a3,s);
	vinf0=vinf[0];//overlap with v1
	cy=vinf[1];//overlap with v1

	//v1=ap1*bp1
	ilmp_mul_n_(v1,ap1,bp1,n+1);
	vinf[1]=cy;//restore, since v1[2*n+1]==0.
	
	//ap2
	cy=ilmp_addshl1_n_(ap2,a2,a3,s);
	if(s!=n)cy=ilmp_add_1_(ap2+s,a2+s,n-s,cy);
	cy=2*cy+ilmp_addshl1_n_(ap2,a1,ap2,n);
	cy=2*cy+ilmp_addshl1_n_(ap2,a0,ap2,n);
	ap2[n]=cy;

	//bp2=bp1+b1
	ilmp_add_(bp2,bp1,n+1,b1,t);

	//v2=ap2*bp2
	ilmp_mul_n_(v2,ap2,bp2,n+1);

	//vm1=am1*bm1
	ilmp_mul_n_(vm1,am1,bm1,n);
	if(am1h)vm1[2*n]=ilmp_add_n_(vm1+n,vm1+n,bm1,n);
	else vm1[2*n]=0;

	//v0=a0*b0
	ilmp_mul_n_(v0,a0,b0,n);

	ilmp_toom_interp5_(dst,v2,vm1,n,s+t,vm1_neg,vinf0);
}
