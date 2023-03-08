
#include"ilmpn.h"

mp_limb_t ilmp_div_basecase_(mp_ptr dstq,mp_ptr numa,mp_size_t na,mp_srcptr numb,mp_size_t nb,mp_limb_t inv21){
	mp_size_t nq=na-nb;

	numa+=na;

	mp_limb_t qh=ilmp_cmp_(numa-nb,numb,nb)>=0;
	if(qh)ilmp_sub_n_(numa-nb,numa-nb,numb,nb);

	nb-=2;
	numa-=2;

	mp_limb_t d1=numb[nb+1],d0=numb[nb+0];
	
	while(nq){
		mp_limb_t q;
		--numa;
		if(numa[2]==d1&&numa[1]==d0){
			q=LIMB_MAX;
			ilmp_submul_1_(numa-nb,numb,nb+2,q);
		}
		else{
			mp_limb_t cy,cy1;
			q=ilmp_div_3_2_(numa,numb+nb,inv21);
			cy=ilmp_submul_1_(numa-nb,numb,nb,q);
			cy1=numa[0]<cy;
			numa[0]-=cy;
			cy=numa[1]<cy1;
			numa[1]-=cy1;
			if(cy){
				ilmp_add_n_(numa-nb,numa-nb,numb,nb+2);
				--q;
			}
		}
		dstq[--nq]=q;
	}
	return qh;
}
