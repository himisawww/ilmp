#include"ilmpn.h"

//round(sqrt(2^25/(i+128+1/2)))-256
static const mp_byte_t ilmp_invsqrt_table_[]={
	0xff,0xfd,0xfb,0xf9,0xf7,0xf5,0xf3,0xf2,0xf0,0xee,0xec,0xea,0xe9,0xe7,0xe5,0xe4,
	0xe2,0xe0,0xdf,0xdd,0xdb,0xda,0xd8,0xd7,0xd5,0xd4,0xd2,0xd1,0xcf,0xce,0xcc,0xcb,
	0xc9,0xc8,0xc6,0xc5,0xc4,0xc2,0xc1,0xc0,0xbe,0xbd,0xbc,0xba,0xb9,0xb8,0xb7,0xb5,
	0xb4,0xb3,0xb2,0xb0,0xaf,0xae,0xad,0xac,0xaa,0xa9,0xa8,0xa7,0xa6,0xa5,0xa4,0xa3,
	0xa2,0xa0,0x9f,0x9e,0x9d,0x9c,0x9b,0x9a,0x99,0x98,0x97,0x96,0x95,0x94,0x93,0x92,
	0x91,0x90,0x8f,0x8e,0x8d,0x8c,0x8c,0x8b,0x8a,0x89,0x88,0x87,0x86,0x85,0x84,0x83,
	0x83,0x82,0x81,0x80,0x7f,0x7e,0x7e,0x7d,0x7c,0x7b,0x7a,0x79,0x79,0x78,0x77,0x76,
	0x76,0x75,0x74,0x73,0x72,0x72,0x71,0x70,0x6f,0x6f,0x6e,0x6d,0x6d,0x6c,0x6b,0x6a,
	0x6a,0x69,0x68,0x68,0x67,0x66,0x66,0x65,0x64,0x64,0x63,0x62,0x62,0x61,0x60,0x60,
	0x5f,0x5e,0x5e,0x5d,0x5c,0x5c,0x5b,0x5a,0x5a,0x59,0x59,0x58,0x57,0x57,0x56,0x56,
	0x55,0x54,0x54,0x53,0x53,0x52,0x52,0x51,0x50,0x50,0x4f,0x4f,0x4e,0x4e,0x4d,0x4d,
	0x4c,0x4b,0x4b,0x4a,0x4a,0x49,0x49,0x48,0x48,0x47,0x47,0x46,0x46,0x45,0x45,0x44,
	0x44,0x43,0x43,0x42,0x42,0x41,0x41,0x40,0x40,0x3f,0x3f,0x3e,0x3e,0x3d,0x3d,0x3c,
	0x3c,0x3b,0x3b,0x3a,0x3a,0x39,0x39,0x39,0x38,0x38,0x37,0x37,0x36,0x36,0x35,0x35,
	0x35,0x34,0x34,0x33,0x33,0x32,0x32,0x32,0x31,0x31,0x30,0x30,0x2f,0x2f,0x2f,0x2e,
	0x2e,0x2d,0x2d,0x2d,0x2c,0x2c,0x2b,0x2b,0x2b,0x2a,0x2a,0x29,0x29,0x29,0x28,0x28,
	0x27,0x27,0x27,0x26,0x26,0x26,0x25,0x25,0x24,0x24,0x24,0x23,0x23,0x23,0x22,0x22,
	0x21,0x21,0x21,0x20,0x20,0x20,0x1f,0x1f,0x1f,0x1e,0x1e,0x1e,0x1d,0x1d,0x1d,0x1c,
	0x1c,0x1b,0x1b,0x1b,0x1a,0x1a,0x1a,0x19,0x19,0x19,0x18,0x18,0x18,0x18,0x17,0x17,
	0x17,0x16,0x16,0x16,0x15,0x15,0x15,0x14,0x14,0x14,0x13,0x13,0x13,0x12,0x12,0x12,
	0x12,0x11,0x11,0x11,0x10,0x10,0x10,0x0f,0x0f,0x0f,0x0f,0x0e,0x0e,0x0e,0x0d,0x0d,
	0x0d,0x0c,0x0c,0x0c,0x0c,0x0b,0x0b,0x0b,0x0a,0x0a,0x0a,0x0a,0x09,0x09,0x09,0x09,
	0x08,0x08,0x08,0x07,0x07,0x07,0x07,0x06,0x06,0x06,0x06,0x05,0x05,0x05,0x04,0x04,
	0x04,0x04,0x03,0x03,0x03,0x03,0x02,0x02,0x02,0x02,0x01,0x01,0x01,0x01,0x00,0x00
};

//[dsts,1]=floor(sqrt(x)), return remainder
//need(x>=B/4)
mp_limb_t ilmp_sqrt_1_(mp_ptr dsts,mp_limb_t x){
	mp_limb_t v,xh=x>>24,s,s2;
	mp_slimb_t t;

	//round(sqrt(2^25/(1/2+floor(x/2^55))))
	v=256+ilmp_invsqrt_table_[(x>>55)-128];
	
	t=(((mp_limb_t)1<<48)-((x>>32)+1)*v*v)*v;
	v=(v<<16)+(t>>33);

	s=v*xh;
	s2=(s>>28)+1;
	t=(xh<<32)-s2*s2;
	s=s+v*(t>>33);

	//we proved that -0.616 < s/2^32 - sqrt(x) < 0
	//so (s>>32) will be either floor(sqrt(x)), or 1 too small
	s>>=32;
	x-=s*s;

	if(x>=2*s+1){
		x-=2*s+1;
		++s;
	}

	*dsts=s;
	return x;
}

//[dsts,1]=floor(sqrt([numa,2])), rh:[dstr,1]=remainder, return rh
//need(numa[1]>=B/4)
mp_limb_t ilmp_sqrt_2_(mp_ptr dsts,mp_ptr dstr,mp_srcptr numa){
	mp_limb_t rl,s,q,al,u;
	mp_slimb_t rh;

	rl=ilmp_sqrt_1_(&s,numa[1]);
	al=numa[0];

	//(r:alh)/2
	rl=rl<<31|al>>33;
	q=rl/s;
	q-=q>>32;

	u=rl-s*q;
	s=s<<32|q;
	rh=u>>31;
	rl=(u<<33)|(al&((mp_limb_t)1<<33)-1);

	q*=q;
	rh-=rl<q;
	rl-=q;
	if(rh<0){
		rl+=s;
		rh+=rl<s;
		--s;
		rl+=s;
		rh+=rl<s;
	}

	dsts[0]=s;
	dstr[0]=rl;
	return rh;
}

//if(!nsh){[dsts,ns],rh:[numa,ns]}=sqrtrem([numa,2*ns]), return rh
//else [dsts,ns]=floor(sqrt([numa,2*ns])), return 1
//need(ns>0, numa[2*ns-1]>=B/4, 0<=nsh<LIMB_BITS)
mp_limb_t ilmp_sqrt_divide_(mp_ptr dsts,mp_ptr numa,mp_size_t ns,int nsh){
	mp_slimb_t rh;
	if(ns==1){
		rh=ilmp_sqrt_2_(dsts,numa,numa);
	}
	else{
		mp_size_t lo=ns/2,hi=ns-lo;
		mp_limb_t qh=ilmp_sqrt_divide_(dsts+lo,numa+2*lo,hi,0);
		if(qh)ilmp_sub_n_(numa+2*lo,numa+2*lo,dsts+lo,hi);
		qh+=ilmp_div_s_(dsts,numa+lo,ns,dsts+lo,hi);
		rh=ilmp_shr_c_(dsts,dsts,lo,1,qh<<LIMB_BITS-1);
		//now dsts is either correct or 1 too big,
		//if nsh-LSBs are non-zero, subtracting 1
		//will not affect anything after de-normalization
		if(dsts[0]&((mp_limb_t)1<<nsh)-1)return 1;
		if(rh)rh=ilmp_add_n_(numa+lo,numa+lo,dsts+lo,hi);
		qh>>=1;
		ilmp_sqr_(numa+ns,dsts,lo);
		mp_limb_t b=qh+ilmp_sub_n_(numa,numa,numa+ns,lo*2);
		if(lo==hi)rh-=b;
		else rh-=ilmp_sub_1_(numa+2*lo,numa+2*lo,1,b);
		if(rh<0){
			qh=ilmp_add_1_(dsts+lo,dsts+lo,hi,qh);
			rh+=2*qh+ilmp_addshl1_n_(numa,numa,dsts,ns);
			rh-=ilmp_sub_1_(numa,numa,ns,1);
			qh-=ilmp_sub_1_(dsts,dsts,ns,1);
		}
	}
	return rh;
}

//[dstis,ns+1]=floor(sqrt(B^(2*ns+na)/[numa,na]))-[0|1], dstis[ns]=1
//need(ns>=3, na>0, numa[na-1]>=B/4)
void ilmp_invsqrt_newton_(mp_ptr dstis,mp_size_t ns,mp_srcptr numa,mp_size_t na){
	mp_size_t nr=ns,namax=na,mn;
	mp_size_t sizes[LIMB_BITS],*sizp=sizes;

	do{
		*sizp=nr;
		nr=(nr>>1)+1;
		++sizp;
	}while(nr>2);

	numa+=na;
	dstis+=ns;

	//nr=2
	//i2=floor((B^5-1)/(1+floor(sqrt(x*B^4))))
	mp_limb_t numa2[6],sval[3];
	ilmp_zero(numa2,4);
	numa2[5]=numa[-1];
	if(na>1)numa2[4]=numa[-2];
	else numa2[4]=0;
	ilmp_sqrt_divide_(sval,numa2,3,0);
	ilmp_inc(sval);
	for(mp_size_t i=0;i<5;++i)numa2[i]=LIMB_MAX;
	dstis[0]=ilmp_div_s_(dstis-2,numa2,5,sval,3);

	TEMP_DECL;
	mp_limb_t alloc_size=na+2*ns+6;
	mp_ptr xp=TALLOC_TYPE(alloc_size,mp_limb_t);
	do{
		na=*--sizp;
		
		//ar = 0:[numa-nr,nr]
		//an = 0:[numa-na,na]
		//ir = 1:[dst-nr,nr] = floor(B^(3*nr/2)/sqrt(ar)) - [0|1]
		// d = B^(na+2*nr)-an*ir*ir
		// -4*B^(na+nr) < d < 4*B^(na+nr)
		
		mp_size_t naz=MIN(na,namax),zeros=na-naz;
		mp_size_t nsqr,nres=naz+nr+1;
		mp_ptr dp=xp+2*nr+1,dip=xp+nr+1;
		int cmod;//1=mod b^mn-1, 0=mod b^(naz+nr+1)
		int sign;//1:d<0, 0:d>=0
		mn=ilmp_fft_next_size_(nres);

		//ir^2
		if(2*SQRT_NEWTON_MODM_THRESHOLD+mn>=nr*2+1){
			cmod=0;
			ilmp_sqr_(xp,dstis-nr,nr+1);
			nsqr=2*nr+1;
		}
		else{
			cmod=1;
			ilmp_mul_mersenne_(xp,mn,dstis-nr,nr+1,dstis-nr,nr+1);
			nsqr=mn;
		}
		
		//ir^2*an
		if(naz<SQRT_NEWTON_MODM_THRESHOLD||naz*8<nsqr||mn>=nsqr+naz){
			if(cmod==0)nsqr=MIN(nsqr,nres);
			ilmp_mul_(dp,xp,nsqr,numa-naz,naz);
			if(cmod==1){
				if(ilmp_add_(dp,dp,mn,dp+mn,naz))ilmp_inc(dp);
			}
		}
		else{
			if(nsqr>mn){//cmod==0
				if(ilmp_add_(xp,xp,mn,xp+mn,nsqr-mn))ilmp_inc(xp);
			}
			ilmp_mul_mersenne_(dp,mn,xp,nsqr,numa-naz,naz);
			cmod=1;
		}

		if(cmod==1){
			// naz+nr < mn <= naz+2*nr
			//[dp,mn] -= B^(naz+2*nr) mod (B^mn-1)
			dp[mn]=1;
			ilmp_dec(dp+naz+2*nr-mn);
			if(dp[mn]==0)ilmp_dec(dp);
		}

		if(dp[nres-1]>3){//-d<0
			if(cmod==0)ilmp_dec(dp);//for neg to not
			//else (neg to not) compensate (mod transfer)
			dp+=naz;
			ilmp_shlnot_(xp,dp+1,nr,LIMB_BITS-1);
			xp[0]^=dp[0]>>1;
			xp[nr]=~dp[nr]>>1;
			sign=0;
		}
		else{//-d>0
			ilmp_shr_(xp,dp+naz,nr+1,1);
			if((dp[naz]&1)||!ilmp_zero_q_(dp,naz))ilmp_inc(xp);
			sign=1;
		}

		ilmp_mul_n_(dip,xp,dstis-nr,nr+1);

		if(sign){
			if(ilmp_zero_q_(dip,3*nr-na)){
				//a limit for dec
				dip[2*nr+1]=1;
				ilmp_dec(dip+3*nr-na);
			}
			ilmp_not_(dstis-na,dip+3*nr-na,na-nr);
			ilmp_dec_1(dstis-nr,dip[2*nr]+1);
		}
		else{
			ilmp_copy(dstis-na,dip+3*nr-na,na-nr);
			ilmp_inc_1(dstis-nr,dip[2*nr]);
		}

		nr=na;
	}while(sizp!=sizes);
	TEMP_FREE;
}

//[dsts,nf+na/2+1]=[floor|round](sqrt([numa,na]*B^(2*nf)))
//need(na>0, nf>=2)
//note: here [floor|ceiling](x) is internally this: round(x-epsilon), where 0<=epsilon<2^-31
//      so if round is equivalent to floor, ceiling will not be used
void ilmp_sqrt_newton_(mp_ptr dsts,mp_srcptr numa,mp_size_t na,mp_size_t nf){
	mp_limb_t high=numa[na-1];
	int nsh=ilmp_leading_zeros_(high)/2;
	mp_size_t ns=na/2+1+nf;

	TEMP_DECL;
	mp_limb_t alloc_size=(nsh?na:0)+ns+1;
	mp_ptr tp=TALLOC_TYPE(alloc_size,mp_limb_t),numa2;
	if(nsh){
		numa2=tp;
		ilmp_shl_(numa2,numa,na,nsh*2);
		tp+=na;
	}
	else numa2=(mp_ptr)numa;

	ilmp_invsqrt_newton_(tp,ns,numa2,na);

	mp_ptr msqr=TALLOC_TYPE(na+ns+1,mp_limb_t);

	if(ns+1>na)ilmp_mul_(msqr,tp,ns+1,numa2,na);
	else ilmp_mul_(msqr,numa2,na,tp,ns+1);

	mp_limb_t cceil;
	if(na&1){
		nsh+=LIMB_BITS/2;
		ilmp_shr_(dsts,msqr+na,ns,nsh);
		cceil=msqr[na]>>nsh-1;
	}
	else{
		if(nsh){
			ilmp_shr_(dsts,msqr+na+1,ns-1,nsh);
			cceil=msqr[na+1]>>nsh-1;
		}
		else {
			ilmp_copy(dsts,msqr+na+1,ns-1);
			cceil=msqr[na]>>LIMB_BITS-1;
		}
		dsts[ns-1]=0;
	}

	if(cceil&1)ilmp_inc(dsts);

	TEMP_FREE;
}

void ilmp_sqrt_(mp_ptr dsts,mp_ptr dstr,mp_srcptr numa,mp_size_t na,mp_size_t nf){
	mp_limb_t high=numa[na-1];
	int nsh=ilmp_leading_zeros_(high)/2;
	mp_size_t nl=na+2*nf;
	if(nl==1){
		mp_limb_t srt;
		ilmp_sqrt_1_(&srt,high<<nsh*2);
		srt>>=nsh;
		dsts[0]=srt;
		if(dstr)dstr[0]=high-srt*srt;
	}
	else if(!dstr&&nf>=10*na+SQRT_NEWTON_THRESHOLD){
		ilmp_sqrt_newton_(dsts,numa,na,nf);
	}
	else{
		TEMP_DECL;
		mp_limb_t ns=(nl+1)/2;
		mp_ptr numa2=TALLOC_TYPE(2*ns,mp_limb_t);
		if(nf)ilmp_zero(numa2,2*nf);
		if(nsh)ilmp_shl_(numa2+2*ns-na,numa,na,nsh*2);
		else ilmp_copy(numa2+2*ns-na,numa,na);
		if(nl&1){
			numa2[2*nf]=0;
			nsh+=LIMB_BITS/2;
		}
		else{
			dsts[ns]=0;
		}
		mp_limb_t rh=ilmp_sqrt_divide_(dsts,numa2,ns,dstr?0:nsh);
		if(nsh){
			if(dstr){
				mp_limb_t ds=dsts[0]&((mp_limb_t)1<<nsh)-1;
				rh+=ilmp_addmul_1_(numa2,dsts,ns,2*ds);
				mp_limb_t b=ilmp_submul_1_(numa2,&ds,1,ds);
				if(ns==1)rh-=b;
				else rh-=ilmp_sub_1_(numa2+1,numa2+1,ns-1,b);
			}
			ilmp_shr_(dsts,dsts,ns,nsh);
		}
		if(dstr){
			numa2[ns]=rh;
			nsh*=2;
			if(nsh>=LIMB_BITS){
				nsh-=LIMB_BITS;
				++numa2;
			}
			else ++ns;
			if(nsh)ilmp_shr_(dstr,numa2,ns,nsh);
			else ilmp_copy(dstr,numa2,ns);
		}

		TEMP_FREE;
	}
}
