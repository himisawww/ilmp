#include"ilmpn.h"

/*

Evaluate in: -1, 0, +1, +2, +inf

  <-s--><--n--><--n-->
  |-a2-|--a1--|--a0--|

v0  =  a0         *^2 #   A(0)^2
v1  = (a0+ a1+ a2)*^2 #   A(1)^2    ah  <= 2
vm1 = (a0- a1+ a2)*^2 #  A(-1)^2   |ah| <= 1
v2  = (a0+2a1+4a2)*^2 #   A(2)^2    ah  <= 6
vinf=          a2 *^2 # A(inf)^2

*/

void ilmp_sqr_toom3_(mp_ptr dst,mp_srcptr numa,mp_size_t na){
	mp_size_t n=(na+2)/3,s=na-2*n;
	mp_limb_t cy,cy2,vinf0,am1h;
	mp_limb_t *tp=SALLOC_TYPE(4*n+4,mp_limb_t);
	
#define a0    numa
#define a1    (numa+n)
#define a2    (numa+2*n)

#define v0    dst          //[dst,2*n]
#define v1    (dst+2*n)    //[dst+2*n,2*n+1]
#define vinf  (dst+4*n)    //[dst+4*n,s+t]
#define vm1   tp           //[tp,2*n+1]
#define v2    (tp+2*n+2)   //[tp+2*n+2,2*n+1]

#define am1   (dst)        //[dst,n]
#define ap1   tp           //[tp,n+1]
#define ap2   ap1          //same space

	//ap1, am1
	cy=ilmp_add_(ap1,a0,n,a2,s);
	if(cy==0&&ilmp_cmp_(ap1,a1,n)<0){
		cy=ilmp_add_n_sub_n_(ap1,am1,a1,ap1,n);
		ap1[n]=cy>>1;
		am1h=0;
    }
	else{
		cy2=ilmp_add_n_sub_n_(ap1,am1,ap1,a1,n);
		ap1[n]=cy+(cy2>>1);
		am1h=cy-(cy2&1);
	}

	//vinf
	ilmp_sqr_(vinf,a2,s);
	vinf0=vinf[0];//overlap with v1
	cy=vinf[1];//overlap with v1

	//v1
	ilmp_sqr_(v1,ap1,n+1);
	vinf[1]=cy;//restore, since v1[2*n+1]==0.

	//ap2
	cy=ilmp_addshl1_n_(ap2,a1,a2,s);
	if(s!=n)cy=ilmp_add_1_(ap2+s,a1+s,n-s,cy);
	cy=2*cy+ilmp_addshl1_n_(ap2,a0,ap2,n);
	ap2[n]=cy;

	//v2
	ilmp_sqr_(v2,ap2,n+1);

	//vm1
	ilmp_sqr_(vm1,am1,n);
	if(am1h)am1h+=ilmp_addshl1_n_(vm1+n,vm1+n,am1,n);
	vm1[2*n]=am1h;

	//v0
	ilmp_sqr_(v0,a0,n);

	ilmp_toom_interp5_(dst,v2,vm1,n,s+s,0,vinf0);
}

