#include"ilmpn.h"

#define MODLIMB_INVERSE_3 ((mp_limb_t)0xAAAAAAAAAAAAAAAB)

void ilmp_divexact_by3_(mp_ptr dst,mp_srcptr numa,mp_size_t na){
	mp_limb_t c=0;
	mp_limb_t l,q,s;
	mp_size_t i=0;
	do{
		s=numa[i];
		l=s-c;
		c=l>s;
		q=l*MODLIMB_INVERSE_3;
		dst[i]=q;
		l=q+q;
		c+=l<q;
		l+=q;
		c+=l<q;
	}
	while(++i<na);
}


void ilmp_toom_interp5_(mp_ptr dst,mp_ptr v2,mp_ptr vm1,mp_size_t n,mp_size_t spt,int vm1_neg,mp_limb_t vinf0){
	mp_limb_t cy,saved;
	mp_size_t dnp=2*n+1;

#define r0    (dst)
#define r1    (dst+n)
#define r2    (dst+2*n)
#define r3    (dst+3*n)
#define r4    (dst+4*n)
#define v0    r0
#define v1    r2
#define vinf  r4

	//v2=(v2-vm1)/3
	if(vm1_neg)ilmp_add_n_(v2,v2,vm1,dnp);
	else ilmp_sub_n_(v2,v2,vm1,dnp);
	ilmp_divexact_by3_(v2,v2,dnp);
	
	//vm1=(v1-vm1)/2
	if (vm1_neg)ilmp_shr1add_n_(vm1,v1,vm1,dnp);
	else ilmp_shr1sub_n_(vm1,v1,vm1,dnp);
    
	//v1=v1-v0
	v1[2*n]-=ilmp_sub_n_(v1,v1,v0,2*n);
	
	//v2=(v2-v1)/2
	ilmp_shr1sub_n_(v2,v2,v1,dnp);

	//v1=v1-vm1
	ilmp_sub_n_(v1,v1,vm1,dnp);
	
	//add vm1 at correct place.
	cy=ilmp_add_n_(r1,r1,vm1,dnp);
	ilmp_inc_1(r3+1,cy);//at most propagate to v1[2*n]

	saved=v1[2*n];//it is vinf[0]
	vinf[0]=vinf0;//correct value of vinf

	//v2=v2-vinf*2
	cy=ilmp_shl_(vm1,vinf,spt,1);
	cy+=ilmp_sub_n_(v2,v2,vm1,spt);
	ilmp_dec_1(v2+spt,cy);

	//vinf+=v2h, no overflow
	cy=ilmp_add_n_(vinf,vinf,v2+n,n+1);
	ilmp_inc_1(r3+dnp,cy);

	//v1-=vinf, (same time vmh-=v2h)
	cy=ilmp_sub_n_(v1,v1,vinf,spt);
	vinf0=vinf[0];
	v1[2*n]=saved;//correct value of v1
	ilmp_dec_1(v1+spt,cy);

	//vml-=v2l
	cy=ilmp_sub_n_(r1,r1,v2,n);
	ilmp_dec_1(v1,cy);
	
	//last v2l
	cy=ilmp_add_n_(r3,r3,v2,n);
	v1[2*n]+=cy;//no carry
	ilmp_inc_1(vinf,vinf0);
}
