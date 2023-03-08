#include"ilmpn.h"


#define _FFT_TABLE_ENTRY(n)   {((mp_size_t)3<<2*(n)-5)+1,(n)}
#define _FFT_TABLE_ENTRY4(n)  \
	_FFT_TABLE_ENTRY(n),      \
	_FFT_TABLE_ENTRY((n)+1),  \
	_FFT_TABLE_ENTRY((n)+2),  \
	_FFT_TABLE_ENTRY((n)+3)
//to make sure best_k_(next_size_(n))=best_k_(n)
//table[i+1][0]-1 must be a multiple of 2^(table[i][1]-LOG2_LIMB_BITS)
static const mp_size_t ilmp_fft_table_[][2]={
	{0,6},{1597,7},{1655,6},
	{1917,7},{3447,8},{3565,7},
	{3831,8},{7661,9},{8145,8},
	{8685,9},{14289,10},{16289,9},{20433,10},{24481,9},
	{26577,10},{28593,11},{32545,10},{57249,11},{65313,10},
	{73633,11},{98081,12},{130625,11},{196385,12},{261697,11},{294689,12},
	{392769,13},{523265,12},{654913,11},{917281,13},
	{1047553,11},{1600001,12},{1834561,14},{2095105,12},
	_FFT_TABLE_ENTRY4(13),
	_FFT_TABLE_ENTRY4(17),
	_FFT_TABLE_ENTRY4(21),
	_FFT_TABLE_ENTRY4(25),
	{(mp_size_t)-1,127}
};

typedef struct{
	mp_ptr temp_coef;//temp coef for xchg
	mp_size_t lenw;//word length of coef
	mp_ssize_t maxdepth;
	mp_ssize_t tempdepth;
	void *mem[16];
	mp_size_t memsize[16];
} fft_memstack;

//best k to use for fft mod B^m+1 for m>=n.
mp_size_t ilmp_fft_best_k_(mp_size_t n){
	mp_size_t k=0;
	while(n>=ilmp_fft_table_[k+1][0])++k;
	return ilmp_fft_table_[k][1];
}
mp_size_t ilmp_fft_next_size_(mp_size_t n){
	mp_size_t k=ilmp_fft_best_k_(n);
	ilmp_assert(k>=LOG2_LIMB_BITS);
	k-=LOG2_LIMB_BITS;
	n=(n-1>>k)+1<<k;
	return n;
}
void *ilmp_fft_memstack_(fft_memstack *ms,mp_size_t size){
	if(size){//malloc
		if(++ms->tempdepth>ms->maxdepth){
			ms->mem[++ms->maxdepth]=ilmp_alloc(size);
			ms->memsize[ms->maxdepth]=size;
		}
		ilmp_assert(ms->memsize[ms->tempdepth]==size);
		return ms->mem[ms->tempdepth];
	}
	else{//free
		if(--ms->tempdepth<0){
			for(mp_size_t i=0;i<=ms->maxdepth;++i)
				ilmp_free(ms->mem[i]);
			ms->maxdepth=-1;
		}
		return 0;
	}
}
//[dst,lenw+1]=[(bit*)numa+bitoffset,bits], need(bitoffset>=0, 0<bits<=LIMB_BITS*lenw, sep(dst,numa))
void ilmp_fft_extract_coef_(mp_ptr dst,mp_srcptr numa,mp_size_t bitoffset,mp_size_t bits,mp_size_t lenw){
	mp_size_t shr=bitoffset&LIMB_BITS-1,offset=bitoffset/LIMB_BITS;
	mp_size_t lena=(bitoffset+bits-1)/LIMB_BITS-offset+1,endp=(bits-1)/LIMB_BITS;
	if(shr)ilmp_shr_(dst,numa+offset,lena,shr);
	else ilmp_copy(dst,numa+offset,lena);
	dst[endp]&=LIMB_MAX>>(-bits&LIMB_BITS-1);
	ilmp_zero(dst+endp+1,lenw-endp);
}
//n=ms->lenw*LIMB_BITS
//[*coef,ms->lenw+1]<<=shl, result pseudo-normalized mod 2^n+1.
//need(0<shr<2*n, *coef pseudo-normalized mod 2^n+1, [ms->temp_coef,ms->lenw+1])
void ilmp_fft_shl_coef_(fft_memstack *ms,mp_ptr *coef,mp_size_t shl){
	mp_size_t l=ms->lenw,w=shl/LIMB_BITS;
	shl&=LIMB_BITS-1;
	mp_ptr src=*coef,dst=ms->temp_coef;
	mp_limb_t cc,rd;
	if(w>=l){
		w-=l;
		if(shl){
			ilmp_shl_(dst,src+l-w,w+1,shl);
			rd=dst[w];
			cc=ilmp_shlnot_(dst+w,src,l-w,shl);
		}
		else{
			if(w)ilmp_copy(dst,src+l-w,w);
			rd=src[l];
			ilmp_not_(dst+w,src,l-w);
			cc=0;
		}
		dst[l]=0;
		++cc;
		ilmp_inc_1(dst,cc);
		if(++rd==0)ilmp_inc(dst+w+1);
		else ilmp_inc_1(dst+w,rd);
	}
	else{
		if(shl){
			ilmp_shlnot_(dst,src+l-w,w+1,shl);
			rd=~dst[w];
			cc=ilmp_shl_(dst+w,src,l-w,shl);
		}
		else{
			if(w)ilmp_not_(dst,src+l-w,w);
			rd=src[l];
			ilmp_copy(dst+w,src,l-w);
			cc=0;
		}
		dst[l]=2;
		ilmp_inc_1(dst,3);
		ilmp_dec_1(dst,cc);
		if(++rd==0)ilmp_dec(dst+w+1);
		else ilmp_dec_1(dst+w,rd);
		cc=dst[l];
		dst[l]=dst[0]<cc;
		ilmp_dec_1(dst,cc-dst[l]);
	}
	ms->temp_coef=src;
	*coef=dst;
}
//n=ms->lenw*LIMB_BITS
//[*coef,ms->lenw+1]>>=shr, result pseudo-normalized mod 2^n+1.
//need(0<shr<2*n, *coef pseudo-normalized mod 2^n+1, [ms->temp_coef,ms->lenw+1])
void ilmp_fft_shr_coef_(fft_memstack *ms,mp_ptr *coef,mp_size_t shr){
	ilmp_fft_shl_coef_(ms,coef,2*ms->lenw*LIMB_BITS-shr);
}
//a=[coef[0],ms->lenw+1], b=[coef[wing],ms->lenw+1], n=ms->lenw*LIMB_BITS
//(a,b)=(a+b,a-b<<w), result pseudo-normalized mod 2^n+1.
//need(0<=w<n, [a|b] pseudo-normalized mod 2^n+1, [ms->temp_coef,ms->lenw+1])
void ilmp_fft_bfy_(fft_memstack *ms,mp_ptr *coef,mp_size_t wing,mp_size_t w){
	mp_ptr numa=coef[0],numb=coef[wing],numc=ms->temp_coef;
	mp_size_t shl=w&LIMB_BITS-1;
	w/=LIMB_BITS;
	mp_size_t l=ms->lenw;
	mp_slimb_t acyo=0,scyo=0,ch;
	mp_limb_t shlcyo=0,chp=0,chn=0;
	for(mp_size_t off=0;off<l-w;off+=PART_SIZE){
		mp_size_t cursize=MIN(l-w-off,PART_SIZE);
		scyo=ilmp_sub_nc_(numc+w+off,numa+off,numb+off,cursize,scyo);
		acyo=ilmp_add_nc_(numa+off,numa+off,numb+off,cursize,acyo);
		if(shl)shlcyo=ilmp_shl_c_(numc+w+off,numc+w+off,cursize,shl,shlcyo);
	}
	ch=shlcyo+(-scyo<<shl);//(-2^63<ch<2^63-1)
	if(ch>0)chp=ch;
	else chn=-ch;
	scyo=0;
	shlcyo=0;
	for(mp_size_t off=l-w;off<l;off+=PART_SIZE){
		mp_size_t cursize=MIN(l-off,PART_SIZE);
		scyo=ilmp_sub_nc_(numc+off-(l-w),numb+off,numa+off,cursize,scyo);
		acyo=ilmp_add_nc_(numa+off,numa+off,numb+off,cursize,acyo);
		if(shl)shlcyo=ilmp_shl_c_(numc+off-(l-w),numc+off-(l-w),cursize,shl,shlcyo);
	}
	numc[w]+=shlcyo;
	scyo=-scyo+numb[l]-numa[l];
	acyo+=numa[l]+numb[l];
	numa[l]=numa[0]<acyo;
	ilmp_dec_1(numa,acyo-numa[l]);

	numc[l]=1;
	++chn;
	if(scyo>0)ilmp_inc_1(numc+w,scyo<<shl);
	else if(scyo<0){
		if(scyo==-2&&shl==LIMB_BITS-1)ilmp_dec(numc+w+1);
		else ilmp_dec_1(numc+w,-scyo<<shl);
	}
	chp+=numc[l];

	if(chn>=chp){
		numc[l]=0;
		ilmp_inc_1(numc,chn-chp);
	}
	else{
		chp-=chn;
		numc[l]=numc[0]<chp;
		ilmp_dec_1(numc,chp-numc[l]);
	}

	coef[wing]=numc;
	ms->temp_coef=numb;
}
//a=[coef[0],ms->lenw+1], b=[coef[wing],ms->lenw+1], n=ms->lenw*LIMB_BITS
//(a,b)=(a+(b>>w),a-(b>>w)), result pseudo-normalized mod 2^n+1.
//need(0<=w<n, [a|b] pseudo-normalized mod 2^n+1, [ms->temp_coef,ms->lenw+1])
void ilmp_ifft_bfy_(fft_memstack *ms,mp_ptr *coef,mp_size_t wing,mp_size_t w){
	mp_ptr numa=coef[0],numb=coef[wing],numc=ms->temp_coef;
	mp_size_t shr=w&LIMB_BITS-1;
	w/=LIMB_BITS;
	mp_size_t l=ms->lenw;
	mp_slimb_t bcyo=0,acyo=0,ah;
	mp_limb_t shrcyo=shr?numb[0]<<LIMB_BITS-shr:0;
	for(mp_size_t off=l-w;off<l;off+=PART_SIZE){
		mp_size_t cursize=MIN(l-off,PART_SIZE);
		if(shr)ilmp_shr_c_(numb+off-(l-w),numb+off-(l-w),cursize,shr,numb[off-(l-w)+cursize]<<LIMB_BITS-shr);
		bcyo=ilmp_add_nc_(numc+off,numa+off,numb+off-(l-w),cursize,bcyo);
		acyo=ilmp_sub_nc_(numa+off,numa+off,numb+off-(l-w),cursize,acyo);
	}
	for(mp_size_t off=0;off<l-w;off+=PART_SIZE){
		mp_size_t cursize=MIN(l-w-off,PART_SIZE);
		if(shr)ilmp_shr_c_(numb+w+off,numb+w+off,cursize,shr,numb[off+w+cursize]<<LIMB_BITS-shr);
		bcyo=ilmp_sub_nc_(numc+off,numa+off,numb+w+off,cursize,bcyo);
		acyo=ilmp_add_nc_(numa+off,numa+off,numb+w+off,cursize,acyo);
	}
	acyo+=numb[l]>>shr;
	bcyo=-bcyo-(numb[l]>>shr);

	acyo-=numa[l-w-1]<shrcyo;
	numa[l-w-1]-=shrcyo;
	numc[l-w-1]+=shrcyo;
	bcyo+=numc[l-w-1]<shrcyo;

	ah=numa[l];

	numa[l]+=1;
	if(w==0)numa[l]+=acyo;
	else{
		if(acyo<0)ilmp_dec(numa+l-w);
		else ilmp_inc_1(numa+l-w,acyo);
	}
	acyo=numa[l]-1;
	if(acyo<0){
		numa[l]=0;
		ilmp_inc(numa);
	}
	else{
		numa[l]=numa[0]<acyo;
		ilmp_dec_1(numa,acyo-numa[l]);
	}

	numc[l]=ah+2;
	if(w==0)numc[l]+=bcyo;
	else{
		if(bcyo>0)ilmp_inc(numc+l-w);
		else ilmp_dec_1(numc+l-w,-bcyo);
	}
	bcyo=numc[l]-2;
	if(bcyo<=0){
		numc[l]=0;
		ilmp_inc_1(numc,-bcyo);
	}
	else{
		numc[l]=numc[0]<bcyo;
		ilmp_dec_1(numc,bcyo-numc[l]);
	}

	coef[wing]=numc;
	ms->temp_coef=numb;
}
//fast fourier transform
void ilmp_fft_b1_(fft_memstack *ms,mp_ptr *coef,mp_size_t dis,mp_size_t k,mp_size_t w,mp_size_t w0){
	if(k==1)
		ilmp_fft_bfy_(ms,coef,dis,w0);
	else{
		k-=2;
		mp_size_t Kq=dis<<k;
		for(mp_size_t i=0;i<Kq;i+=dis){
			ilmp_fft_bfy_(ms,coef+i,2*Kq,i*w+w0);
			ilmp_fft_bfy_(ms,coef+i+Kq,2*Kq,(i+Kq)*w+w0);
			ilmp_fft_bfy_(ms,coef+i,Kq,2*(i*w+w0));
			ilmp_fft_bfy_(ms,coef+i+Kq*2,Kq,2*(i*w+w0));
		}
		if(k>0){
			ilmp_fft_b1_(ms,coef,dis,k,4*w,4*w0);
			ilmp_fft_b1_(ms,coef+Kq,dis,k,4*w,4*w0);
			ilmp_fft_b1_(ms,coef+Kq*2,dis,k,4*w,4*w0);
			ilmp_fft_b1_(ms,coef+Kq*3,dis,k,4*w,4*w0);
		}
	}
}
void ilmp_fft_4_(fft_memstack *ms,mp_ptr *coef,mp_size_t k,mp_size_t w){
	if(k==1)
		ilmp_fft_bfy_(ms,coef,1,0);
	else{
		k-=2;
		mp_size_t Kq=((mp_size_t)1)<<k;
		for(mp_size_t i=0;i<Kq;++i){
			ilmp_fft_bfy_(ms,coef+i,Kq*2,i*w);
			ilmp_fft_bfy_(ms,coef+i+Kq,Kq*2,(i+Kq)*w);
			ilmp_fft_bfy_(ms,coef+i,Kq,2*i*w);
			ilmp_fft_bfy_(ms,coef+i+2*Kq,Kq,2*i*w);
		}
		if(k>0){
			ilmp_fft_4_(ms,coef,k,w*4);
			ilmp_fft_4_(ms,coef+Kq,k,w*4);
			ilmp_fft_4_(ms,coef+2*Kq,k,w*4);
			ilmp_fft_4_(ms,coef+3*Kq,k,w*4);
		}
	}
}
void ilmp_fft_(fft_memstack *ms,mp_ptr *coef,mp_size_t k,mp_size_t w){
	mp_size_t k1=k>>1;
	k-=k1;
	mp_size_t Kp=((mp_size_t)1)<<k,Kq=((mp_size_t)1)<<k1;
	for(mp_size_t i=0;i<Kp;++i)
		ilmp_fft_b1_(ms,coef+i,Kp,k1,w,i*w);
	for(mp_size_t i=0;i<Kq;++i)
		ilmp_fft_4_(ms,coef+Kp*i,k,w*Kq);
}
//inverse fast fourier transform
void ilmp_ifft_b1_(fft_memstack *ms,mp_ptr *coef,mp_size_t dis,mp_size_t k,mp_size_t w,mp_size_t w0){
	if(k==1)
		ilmp_ifft_bfy_(ms,coef,dis,w0);
	else{
		k-=2;
		mp_size_t Kq=dis<<k;
		if(k>0){
			ilmp_ifft_b1_(ms,coef,dis,k,4*w,4*w0);
			ilmp_ifft_b1_(ms,coef+Kq,dis,k,4*w,4*w0);
			ilmp_ifft_b1_(ms,coef+Kq*2,dis,k,4*w,4*w0);
			ilmp_ifft_b1_(ms,coef+Kq*3,dis,k,4*w,4*w0);
		}
		for(mp_size_t i=0;i<Kq;i+=dis){
			ilmp_ifft_bfy_(ms,coef+i,Kq,2*(i*w+w0));
			ilmp_ifft_bfy_(ms,coef+i+Kq*2,Kq,2*(i*w+w0));
			ilmp_ifft_bfy_(ms,coef+i,2*Kq,i*w+w0);
			ilmp_ifft_bfy_(ms,coef+i+Kq,2*Kq,(i+Kq)*w+w0);
		}
	}
}
void ilmp_ifft_4_(fft_memstack *ms,mp_ptr *coef,mp_size_t k,mp_size_t w){
	if(k==1)
		ilmp_ifft_bfy_(ms,coef,1,0);
	else{
		k-=2;
		mp_size_t Kq=((mp_size_t)1)<<k;
		if(k>0){
			ilmp_ifft_4_(ms,coef,k,w*4);
			ilmp_ifft_4_(ms,coef+Kq,k,w*4);
			ilmp_ifft_4_(ms,coef+2*Kq,k,w*4);
			ilmp_ifft_4_(ms,coef+3*Kq,k,w*4);
		}
		for(mp_size_t i=0;i<Kq;++i){
			ilmp_ifft_bfy_(ms,coef+i,Kq,2*i*w);
			ilmp_ifft_bfy_(ms,coef+i+2*Kq,Kq,2*i*w);
			ilmp_ifft_bfy_(ms,coef+i,Kq*2,i*w);
			ilmp_ifft_bfy_(ms,coef+i+Kq,Kq*2,(i+Kq)*w);
		}
	}
}
void ilmp_ifft_(fft_memstack *ms,mp_ptr *coef,mp_size_t k,mp_size_t w){
	mp_size_t k1=k>>1;
	k-=k1;
	mp_size_t Kp=((mp_size_t)1)<<k,Kq=((mp_size_t)1)<<k1;
	for(mp_size_t i=0;i<Kq;++i)
		ilmp_ifft_4_(ms,coef+Kp*i,k,w*Kq);
	for(mp_size_t i=0;i<Kp;++i)
		ilmp_ifft_b1_(ms,coef+i,Kp,k1,w,i*w);
}
//recombine of fermat multiplication
void ilmp_mul_fermat_recombine_(
	fft_memstack *ms,mp_ptr dst,mp_ptr *pfca,
	mp_size_t K,mp_size_t k,mp_size_t n,mp_size_t M,mp_size_t rn)
{
	mp_size_t rhead=0,nlen=ms->lenw+1;
	mp_slimb_t borrow=0,maxc=0;
	for(mp_size_t i=0;i<K;++i){
		ilmp_fft_shr_coef_(ms,pfca+i,(i*n>>k)+k);
		mp_ptr nums=pfca[i];
		if(nums[nlen-1]){
			ilmp_dec(nums);
			--nums[nlen-1];
		}
		if(nums[nlen-1]==0&&nums[nlen-2]>>LIMB_BITS-1){
			ilmp_dec(nums);
			--nums[nlen-1];
		}
		if(borrow){
			mp_size_t brshift=borrow-1+n-M,bshl=brshift&LIMB_BITS-1;
			brshift/=LIMB_BITS;
			--nums[nlen-1];
			ilmp_dec_1(nums+brshift,(mp_limb_t)1<<bshl);
			++nums[nlen-1];
		}
		borrow=-nums[nlen-1];
		nums[nlen-1]=0;
		
		mp_size_t roffset=i*M,shl=roffset&LIMB_BITS-1;
		roffset/=LIMB_BITS;
		if(shl)ilmp_shl_(nums,nums,nlen,shl);
		if(i==0){
			ilmp_copy(dst,nums,nlen);
			rhead=nlen;
		}
		else if(roffset+nlen<=rn){
			//roffset<rhead && rhead<=roffset+nlen && no carry
			ilmp_add_(dst+roffset,nums,nlen,dst+roffset,rhead-roffset);
			rhead=roffset+nlen;
		}
		else{
			maxc+=ilmp_add_(dst+roffset,nums,rn-roffset,dst+roffset,rhead-roffset);
			maxc-=ilmp_sub_(dst,dst,rn,nums+rn-roffset,nlen+roffset-rn);
			rhead=rn;
		}
	}

	if(borrow){
		mp_size_t cyshift=borrow-1+n-M,cshl=cyshift&LIMB_BITS-1;
		cyshift/=LIMB_BITS;
		maxc+=ilmp_add_1_(dst+cyshift,dst+cyshift,rn-cyshift,(mp_limb_t)1<<cshl);
	}
	if(maxc>0){
		dst[rn]=dst[0]<maxc;
		ilmp_dec_1(dst,maxc-dst[rn]);
	}
	else{
		dst[rn]=0;
		ilmp_inc_1(dst,-maxc);
	}
}
//for(0<=i<K0)[pc1[i],ms->lenw+1]*=[pc2[i],ms->lenw+1] mod B^ms->lenw+1, result pseudo-normalized,
//need(K0>0, for(0<=i<K0)[[pc1|pc2][i],ms->lenw+1] pseudo-normalized)
void ilmp_mul_fermat_recurse_(fft_memstack *ms,mp_ptr *pc1,mp_ptr *pc2,mp_size_t K0){
	int nsqr=pc1!=pc2;
	//push
	mp_ptr push_temp_coef=ms->temp_coef;
	mp_size_t rn=ms->lenw;
	if(rn<MUL_FFT_MODF_THRESHOLD){//direct mul mod
		mp_ptr temp_mul=(mp_ptr)ilmp_fft_memstack_(ms,(rn+1)*2*LIMB_BYTES);
		for(mp_size_t i=0;i<K0;++i){
			if(nsqr)ilmp_mul_n_(temp_mul,pc1[i],pc2[i],rn+1);
			else ilmp_sqr_(temp_mul,pc1[i],rn+1);
			mp_limb_t maxc=ilmp_sub_n_(pc1[i],temp_mul,temp_mul+rn,rn)+temp_mul[rn*2];
			pc1[i][rn]=0;
			ilmp_inc_1(pc1[i],maxc);
		}
		ilmp_fft_memstack_(ms,0);
	}
	else{
		mp_size_t N=rn*LIMB_BITS,k=ilmp_fft_best_k_(rn),K=((mp_size_t)1)<<k;
		ilmp_assert(!(N&K-1));//N must be a multiple of K
		mp_size_t M=N>>k,n=2*M+k+2;
		//n>=2*M+k+2 and should be a multiple of both LIMB_BITS and K.
		n=n+LIMB_BITS-1&-LIMB_BITS;
		n=(n-1>>k)+1<<k;

		ms->lenw=n/LIMB_BITS;
		mp_size_t nlen=ms->lenw+1;
		//alloc:
		//nlen<<k+nsqr for 2^(k+nsqr) coefs,
		//1<<k+nsqr for 2^(k+nsqr) coefs' pointers,
		//nlen for 1 temp_coef.
		ms->temp_coef=(mp_ptr)ilmp_fft_memstack_(ms,((nlen+1<<k+nsqr)+nlen)*LIMB_BYTES);
		mp_ptr *pfca=(mp_ptr*)(ms->temp_coef+nlen),*pfcb=pfca;
		for(mp_size_t i=0;i<K;++i)
			pfca[i]=(mp_ptr)(pfca+K)+i*nlen;
		if(nsqr){
			pfcb+=nlen+1<<k;
			for(mp_size_t i=0;i<K;++i)
				pfcb[i]=(mp_ptr)(pfcb+K)+i*nlen;
		}
		
		for(mp_size_t j=0;j<K0;++j){
			mp_ptr numa=pc1[j],numb=pc2[j];
			for(mp_size_t i=0;i<K;++i){
				ilmp_fft_extract_coef_(pfca[i],numa,M*i,M+(i==K-1),ms->lenw);
				if(i>0)ilmp_fft_shl_coef_(ms,pfca+i,i*n>>k);
			}
			ilmp_fft_(ms,pfca,k,n>>k-1);
			if(nsqr){
				for(mp_size_t i=0;i<K;++i){
					ilmp_fft_extract_coef_(pfcb[i],numb,M*i,M+(i==K-1),ms->lenw);
					if(i>0)ilmp_fft_shl_coef_(ms,pfcb+i,i*n>>k);
				}
				ilmp_fft_(ms,pfcb,k,n>>k-1);
			}
			ilmp_mul_fermat_recurse_(ms,pfca,pfcb,K);
			ilmp_ifft_(ms,pfca,k,n>>k-1);
			ilmp_mul_fermat_recombine_(ms,numa,pfca,K,k,n,M,rn);
		}
		ilmp_fft_memstack_(ms,0);
	}
	//pop
	ms->temp_coef=push_temp_coef;
	ms->lenw=rn;
}
void ilmp_mul_fermat_(mp_ptr dst,mp_size_t rn,mp_srcptr numa,mp_size_t na,mp_srcptr numb,mp_size_t nb){

	int nsqr=numa!=numb||na!=nb;
	mp_size_t N=rn*LIMB_BITS,k=ilmp_fft_best_k_(rn),K=((mp_size_t)1)<<k;
	ilmp_assert(!(N&K-1));//N must be a multiple of K
	mp_size_t M=N>>k,n=2*M+k+2;
	//n>=2*M+k+2 and should be a multiple of both LIMB_BITS and K.
	n=n+LIMB_BITS-1&-LIMB_BITS;
	n=(n-1>>k)+1<<k;
	
	fft_memstack msr;
	msr.maxdepth=-1;
	msr.tempdepth=-1;
	msr.lenw=n/LIMB_BITS;
	mp_size_t nlen=msr.lenw+1;
	//alloc:
	//nlen<<k+nsqr for 2^(k+nsqr) coefs,
	//1<<k+nsqr for 2^(k+nsqr) coefs' pointers,
	//nlen for 1 temp_coef.
	msr.temp_coef=(mp_ptr)ilmp_fft_memstack_(&msr,((nlen+1<<k+nsqr)+nlen)*LIMB_BYTES);
	
	mp_ptr *pfca=(mp_ptr*)(msr.temp_coef+nlen),*pfcb=pfca;
	mp_size_t narest=na*LIMB_BITS,nbrest=nb*LIMB_BITS;

	for(mp_size_t i=0;i<K;++i){
		mp_size_t coeflen;
		pfca[i]=(mp_ptr)(pfca+K)+i*nlen;
		if(narest>0){
			coeflen=M+(i==K-1);
			coeflen=MIN(narest,coeflen);
			narest-=coeflen;
			ilmp_fft_extract_coef_(pfca[i],numa,M*i,coeflen,msr.lenw);
			if(i>0)ilmp_fft_shl_coef_(&msr,pfca+i,i*n>>k);
		}
		else ilmp_zero(pfca[i],nlen);
	}
	ilmp_fft_(&msr,pfca,k,n>>k-1);
	if(nsqr){
		pfcb+=nlen+1<<k;
		for(mp_size_t i=0;i<K;++i){
			mp_size_t coeflen;
			pfcb[i]=(mp_ptr)(pfcb+K)+i*nlen;
			if(nbrest>0){
				coeflen=M+(i==K-1);
				coeflen=MIN(nbrest,coeflen);
				nbrest-=coeflen;
				ilmp_fft_extract_coef_(pfcb[i],numb,M*i,coeflen,msr.lenw);
				if(i>0)ilmp_fft_shl_coef_(&msr,pfcb+i,i*n>>k);
			}
			else ilmp_zero(pfcb[i],nlen);
		}
		ilmp_fft_(&msr,pfcb,k,n>>k-1);
	}
	ilmp_mul_fermat_recurse_(&msr,pfca,pfcb,K);
	ilmp_ifft_(&msr,pfca,k,n>>k-1);
	ilmp_mul_fermat_recombine_(&msr,dst,pfca,K,k,n,M,rn);
	if(dst[rn]&&!ilmp_zero_q_(dst,rn)){
		dst[rn]=0;
		ilmp_dec(dst);
	}
	ilmp_fft_memstack_(&msr,0);
}
void ilmp_mul_mersenne_(mp_ptr dst,mp_size_t rn,mp_srcptr numa,mp_size_t na,mp_srcptr numb,mp_size_t nb){
	int nsqr=numa!=numb||na!=nb;
	mp_size_t N=rn*LIMB_BITS,k=ilmp_fft_best_k_(rn),K=((mp_size_t)1)<<k;
	ilmp_assert(!(N&K-1));//N must be a multiple of K
	mp_size_t M=N>>k,n=2*M+k;
	//n>=2*M+k and should be a multiple of both LIMB_BITS and K/2.
	n=n+LIMB_BITS-1&-LIMB_BITS;
	n=(n-1>>k-1)+1<<k-1;

	fft_memstack msr;
	msr.maxdepth=-1;
	msr.tempdepth=-1;
	msr.lenw=n/LIMB_BITS;
	mp_size_t nlen=msr.lenw+1;
	//alloc:
	//nlen<<k+nsqr for 2^(k+nsqr) coefs,
	//1<<k+nsqr for 2^(k+nsqr) coefs' pointers,
	//nlen for 1 temp_coef.
	msr.temp_coef=(mp_ptr)ilmp_fft_memstack_(&msr,((nlen+1<<k+nsqr)+nlen)*LIMB_BYTES);

	mp_ptr *pfca=(mp_ptr*)(msr.temp_coef+nlen),*pfcb=pfca;
	mp_size_t narest=na*LIMB_BITS,nbrest=nb*LIMB_BITS;

	for(mp_size_t i=0;i<K;++i){
		mp_size_t coeflen;
		pfca[i]=(mp_ptr)(pfca+K)+i*nlen;
		if(narest>0){
			coeflen=MIN(narest,M);
			narest-=coeflen;
			ilmp_fft_extract_coef_(pfca[i],numa,M*i,coeflen,msr.lenw);

		}
		else ilmp_zero(pfca[i],nlen);
	}
	ilmp_fft_(&msr,pfca,k,n>>k-1);
	if(nsqr){
		pfcb+=nlen+1<<k;
		for(mp_size_t i=0;i<K;++i){
			mp_size_t coeflen;
			pfcb[i]=(mp_ptr)(pfcb+K)+i*nlen;
			if(nbrest>0){
				coeflen=MIN(nbrest,M);
				nbrest-=coeflen;
				ilmp_fft_extract_coef_(pfcb[i],numb,M*i,coeflen,msr.lenw);

			}
			else ilmp_zero(pfcb[i],nlen);
		}
		ilmp_fft_(&msr,pfcb,k,n>>k-1);
	}
	ilmp_mul_fermat_recurse_(&msr,pfca,pfcb,K);
	ilmp_ifft_(&msr,pfca,k,n>>k-1);

	mp_size_t rhead=0,maxc=0;
	for(mp_size_t i=0;i<K;++i){
		ilmp_fft_shr_coef_(&msr,pfca+i,k);
		mp_ptr nums=pfca[i];
		if(nums[nlen-1]){
			ilmp_dec(nums);
			ilmp_assert(nums[nlen-1]==1);
			nums[nlen-1]=0;
		}

		mp_size_t roffset=i*M,shl=roffset&LIMB_BITS-1;
		roffset/=LIMB_BITS;
		if(shl)ilmp_shl_(nums,nums,nlen,shl);
		if(i==0){
			ilmp_copy(dst,nums,nlen);
			rhead=nlen;
		}
		else if(roffset+nlen<=rn){
			//roffset<rhead && rhead<=roffset+nlen && no carry
			ilmp_add_(dst+roffset,nums,nlen,dst+roffset,rhead-roffset);
			rhead=roffset+nlen;
		}
		else{
			maxc+=ilmp_add_(dst+roffset,nums,rn-roffset,dst+roffset,rhead-roffset);
			maxc+=ilmp_add_(dst,dst,rn,nums+rn-roffset,nlen+roffset-rn);
			rhead=rn;
		}
	}
	if(!ilmp_add_1_(dst,dst,rn,1+maxc))ilmp_dec(dst);
	ilmp_fft_memstack_(&msr,0);
}

void ilmp_mul_fft_(mp_ptr dst,mp_srcptr numa,mp_size_t na,mp_srcptr numb,mp_size_t nb){
	mp_size_t hn=ilmp_fft_next_size_(na+nb+1>>1);
	ilmp_assert(na+nb>hn);
	mp_ptr tp=ALLOC_TYPE(hn+1,mp_limb_t);

	mp_srcptr amodm=numa;
	mp_size_t nam=na;
	if(na>hn){
		if(ilmp_add_(dst,numa,hn,numa+hn,na-hn))ilmp_inc(dst);
		amodm=dst;
		nam=hn;
	}
	ilmp_mul_mersenne_(dst,hn,amodm,nam,numb,nb);

	mp_srcptr amodp=numa;
	mp_size_t nap=na;
	if(na>hn){
		tp[hn]=0;
		if(ilmp_sub_(tp,numa,hn,numa+hn,na-hn))ilmp_inc(tp);
		amodp=tp;
		nap=hn+1;
	}
	ilmp_mul_fermat_(tp,hn,amodp,nap,numb,nb);

	mp_limb_t cy=ilmp_shr1add_nc_(dst,dst,tp,hn,tp[hn]);
	cy<<=LIMB_BITS-1;
	dst[hn-1]+=cy;
	if(dst[hn-1]<cy)ilmp_inc(dst);

	if(na+nb==2*hn){
		cy=tp[hn]+ilmp_sub_n_(dst+hn,dst,tp,hn);
		//cy==1 means [tp,hn+1]!=0, then [dst,hn]!=0
		//cy==2 is impossible since [tp,hn+1] is normalized.
		//so the following dec won't overflow.
		ilmp_dec_1(dst,cy);
	}
	else{
		cy=ilmp_sub_n_(dst+hn,dst,tp,na+nb-hn);
		cy=tp[hn]+ilmp_sub_nc_(tp+na+nb-hn,dst+na+nb-hn,tp+na+nb-hn,2*hn-(na+nb),cy);
		cy=ilmp_sub_1_(dst,dst,na+nb,cy);
	}
	FREE(tp);
}
