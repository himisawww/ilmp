#include"ilmpn.h"


//1:[dst,na]=(2^(2*na*ILMP_BITS)-1)/[numa,na]
//need(na>0, MSB(numa), sep(dst, numa))
void ilmp_inv_basecase_(mp_ptr dst,mp_srcptr numa,mp_size_t na){
	if(na==1)
		*dst=ilmp_inv_1_(*numa);
	else{
		TEMP_DECL;
		mp_ptr xp=TALLOC_TYPE(2*na,mp_limb_t);
		mp_size_t i=na;
		do xp[--i]=LIMB_MAX;
		while(i);
		ilmp_not_(xp+na,numa,na);
		//[xp,2*na]=2^(64*2*na)-1-[numa,na]*2^(64*na)
		
		if(na==2){
			ilmp_div_2_s_(dst,xp,4,numa);
		}
		else{
			mp_limb_t inv21=ilmp_inv_2_1_(numa[na-1],numa[na-2]);
			if(na<DIV_DIVIDE_THRESHOLD)
				ilmp_div_basecase_(dst,xp,2*na,numa,na,inv21);
			else
				ilmp_div_divide_(dst,xp,2*na,numa,na,inv21);
		}
		TEMP_FREE;
	}
}

//1:[dst,na]=(2^(2*na*ILMP_BITS)-1)/[numa,na]+[0|-1]
//need(na>4, MSB(numa), sep(dst, numa))
void ilmp_invappr_newton_(mp_ptr dst,mp_srcptr numa,mp_size_t na){
	mp_limb_t cy;
	mp_size_t nr=na,mn;
	mp_size_t sizes[LIMB_BITS],*sizp=sizes;
	
	do{
		*sizp=nr;
		nr=(nr>>1)+1;
		++sizp;
	}while(nr>=INV_NEWTON_THRESHOLD);
	
	numa+=na;
	dst+=na;

	ilmp_inv_basecase_(dst-nr,numa-nr,nr);
	
	TEMP_DECL;
	mp_ptr xp=TALLOC_TYPE(3*(na>>1)+3,mp_limb_t);
	do{
		na=*--sizp;
		
		//ar = 0:[numa-nr,nr]
		//an = 0:[numa-na,na]
		//ir = 1:[dst-nr,nr] = (B^(2*nr)-1)/ar - [0|1]
		//rem = ir*an-B^(na+nr)
		//-2*B^na < rem < 2*B^na

		//[xp] = rem
		if(na<INV_MODM_THRESHOLD||(mn=ilmp_fft_next_size_(na+1))>=na+nr){
			ilmp_mul_(xp,numa-na,na,dst-nr,nr);
			ilmp_add_n_(xp+nr,xp+nr,numa-na,na+1-nr);
			cy=1;//for mod B^(na+1)
		}
		else{// nr < na < mn < na+nr

			//[xp,mn] = [dst,nr] * [numa,na] mod (B^mn-1)
			ilmp_mul_mersenne_(xp,mn,numa-na,na,dst-nr,nr);

			//[xp,mn] += [numa,na]*B^nr mod (B^mn-1)
			cy=ilmp_add_n_(xp+nr,xp+nr,numa-na,mn-nr);
			cy=ilmp_add_nc_(xp,xp,numa-(na-(mn-nr)),na-(mn-nr),cy);

			//[xp,mn] -= B^(na+nr) mod (B^mn-1)
			xp[mn]=1;
			ilmp_dec_1(xp+na+nr-mn,1-cy);
			ilmp_dec_1(xp,1-xp[mn]);

			cy=0;//for mod (B^mn-1)
		}
		
		//adjust ir,rem s.t.
		// -B^na < rem = ir*an - B^(na+nr) < 0
		// use this we can prove B^nr <= ir < 2*B^nr
		// so inc/dec ir won't overflow
		if(xp[na]<2){//rem>=0

			//rem-=cy*an s.t. rem[na]=0
			if(cy=xp[na]){
				if(!ilmp_sub_n_(xp,xp,numa-na,na)){
					++cy;
					ilmp_sub_n_(xp,xp,numa-na,na);
				}
			}

			//rem-=cy*an s.t. 0<=rem<an
			if(ilmp_cmp_(xp,numa-na,na)>=0){
				ilmp_sub_n_(xp,xp,numa-na,na);
				++cy;
			}

			// 0 < an-rem <= an < B^na , trunc to nr limbs
			ilmp_sub_nc_(xp+2*nr,numa-nr,xp+na-nr,nr,ilmp_cmp_(xp,numa-na,na-nr)>0);
			++cy;

			ilmp_dec_1(dst-nr,cy);
		}
		else{//rem<0
			if(cy)ilmp_dec(xp);//for neg to not
			//else (neg to not) compensate (mod transfer)

			if(xp[na]!=LIMB_MAX){
				ilmp_assert(xp[na]+ilmp_add_n_(xp,xp,numa-na,na)==LIMB_MAX);
				ilmp_inc(dst-nr);
			}

			//-rem
			ilmp_not_(xp+2*nr,xp+na-nr,nr);
		}
		
		//in = 1:[dst-na,na]
		//in = ir*B^(na-nr) + ir*(-rem/B^(na-nr))/B^(3*nr-na)
		//use inequality an*ir!=B^(na+nr),
		//(otherwise obviously contradictory),
		//we can prove
		// an*in <= an*ir * ( 2*B^(na+nr) - an*ir ) * B^(-2*nr) < B^(2*na)
		//so in < B^(2*na)/an <= 2*B^(na),
		//inc below won't overflow

		//and via inequality -B^na < an*ir - B^(na+nr) < 0
		//we can prove in = (B^(2*na)-1)/an - [0|1]
		ilmp_mul_n_(xp,xp+2*nr,dst-nr,nr);
		cy=ilmp_add_n_(xp+nr,xp+nr,xp+2*nr,2*nr-na);
		if(ilmp_add_nc_(dst-na,xp+3*nr-na,xp+4*nr-na,na-nr,cy))ilmp_inc(dst-nr);

		nr=na;
	}while(sizp!=sizes);
	TEMP_FREE;
}

void ilmp_invappr_(mp_ptr dst,mp_srcptr numa,mp_size_t na){
	if(na<INV_NEWTON_THRESHOLD)
		ilmp_inv_basecase_(dst,numa,na);
	else
		ilmp_invappr_newton_(dst,numa,na);
}

void ilmp_inv_(mp_ptr dst,mp_srcptr numa,mp_size_t na,mp_size_t nf){
	mp_limb_t high=numa[na-1];
	int nsh=ilmp_leading_zeros_(high);
	TEMP_DECL;
	if(dst==numa||nsh||nf){
		nf+=nsh!=0;
		mp_ptr numa2=TALLOC_TYPE(na+nf,mp_limb_t);
		ilmp_zero(numa2,nf);
		if(nsh)ilmp_shl_(numa2+nf,numa,na,nsh);
		else ilmp_copy(numa2+nf,numa,na);
		numa=numa2;
	}
	ilmp_invappr_(dst,numa,na+nf);
	if(nsh)ilmp_shr_c_(dst,dst,na+nf,LIMB_BITS-nsh,(mp_limb_t)1<<nsh);
	else dst[na+nf]=1;
	TEMP_FREE;
}
