#include"ilmpn.h"

mp_size_t ilmp_div_inv_size_(mp_size_t nq,mp_size_t nb){
	mp_size_t ni,b;
	if(nq>nb){
		b=(nq-1)/nb+1;		//ceil(nq/nb), number of blocks
		ni=(nq-1)/b+1;		//ceil(nq/b)
	}
	else if(3*nq>nb){
		ni=(nq-1)/2+1;		//b=2
	}
	else{
		ni=(nq-1)/1+1;		//b=1
	}
	return ni;
}

void ilmp_inv_prediv_(mp_ptr dst,mp_srcptr numa,mp_size_t na,mp_size_t ni){
	TEMP_DECL;
	mp_limb_t cy;
	mp_ptr tp=TALLOC_TYPE(ni+1,mp_limb_t);
	
	if(na==ni){
		ilmp_copy(tp+1,numa,ni);
		tp[0]=1;
		cy=0;
    }
	else cy=ilmp_add_1_(tp,numa+na-(ni+1),ni+1,1);

	if(cy)ilmp_zero(dst,ni);
	else{
		mp_ptr invappr=TALLOC_TYPE(ni+1,mp_limb_t);
		ilmp_invappr_(invappr,tp,ni+1);
		ilmp_copy(dst,invappr+1,ni);
	}
	TEMP_FREE;
}

mp_limb_t ilmp_div_mulinv_(mp_ptr dstq,mp_ptr numa,mp_size_t na,mp_srcptr numb,mp_size_t nb,mp_srcptr invappr,mp_size_t ni){
	mp_size_t nq=na-nb,ntp=MIN(ni,nq)+nb;
	mp_limb_t qh;
	TEMP_DECL;
	mp_ptr tp=TALLOC_TYPE(ntp,mp_limb_t);
	
	numa+=nq;
	dstq+=nq;
	
	qh=ilmp_cmp_(numa,numb,nb)>=0;
	if(qh)ilmp_sub_n_(numa,numa,numb,nb);
	
	while(nq){
		if(nq<ni){
			invappr+=ni-nq;
			ni=nq;
		}
		numa-=ni;
		dstq-=ni;
		nq-=ni;
		
		ilmp_mul_n_(tp,numa+nb,invappr,ni);
		ilmp_assert(ilmp_add_n_(dstq,tp+ni,numa+nb,ni)==0);
		
		mp_size_t mn,wn;
		mp_limb_t cy;

      	if(nb<DIV_MULINV_MODM_THRESHOLD||(mn=ilmp_fft_next_size_(nb+1))>=nb+ni)
			ilmp_mul_(tp,numb,nb,dstq,ni);		//nb+ni limbs, high 'ni' cancels
		else{
			//0<wn<ni<=nb<mn<nb+ni
			wn=nb+ni-mn;

			//x=b*q
			//tp=x mod 2^mn-1
			ilmp_mul_mersenne_(tp,mn,numb,nb,dstq,ni);

			//tp-=ah:0 mod B^mn-1, if result=0, represent it as B^mn-1
			cy=ilmp_sub_nc_(tp,tp,numa+mn,wn,1);
			if(cy)cy=ilmp_sub_1_(tp+wn,tp+wn,mn-wn,1);
			if(!cy)ilmp_inc(tp);

			//if al<<tp,
			if(ilmp_cmp_(numa+nb,tp+nb,mn-nb)<0){
				//maybe ah=xh+1 and al<<xl,
				// so we subtracted 1 too much when tp-=ah,
				// now tp=xl-1 mod B^mn-1, and 0<=al<<xl-1<B^mn-1, so tp=xl-1
				//or ah=xh and al>=xl,
				// tp=xl mod B^mn-1, the only possibility is we represented xl=0 as tp=B^mn-1
				//whatever, just inc and then tp=xl
				tp[mn]=0;//set a limit
				ilmp_inc(tp);
			}
		}
		
		mp_limb_t r=numa[nb]-tp[nb];
		cy=ilmp_sub_n_(numa,numa,tp,nb);
		
		while((r-=cy)||ilmp_cmp_(numa,numb,nb)>=0){
			ilmp_inc(dstq);
			cy=ilmp_sub_n_(numa,numa,numb,nb);
		}	
	}
	TEMP_FREE;
	return qh;
}
