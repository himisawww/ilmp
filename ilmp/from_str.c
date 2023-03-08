#include"ilmpn.h"

//assume src[len-1]!=0
mp_size_t ilmp_from_str_basecase_(mp_ptr dst,const mp_byte_t *src,mp_size_t len,int base){
	int digitspl=ilmp_bases_[base].digits_in_limb;
	mp_limb_t lbase=ilmp_bases_[base].large_base;
	mp_size_t limbs=0,i=len;

	while(i){
		mp_limb_t curlimb;
		if(i>=digitspl){
			curlimb=src[--i];
			for(mp_size_t j=1;j<digitspl;++j){
				curlimb=curlimb*base+src[--i];
			}
		}
		else{
			curlimb=src[--i];
			lbase=base;
			while(i){
				curlimb=curlimb*base+src[--i];
				lbase*=base;
			}
		}

		if(limbs==0){
			dst[0]=curlimb;
			limbs=1;
		}
		else{
			mp_limb_t cy=ilmp_mul_1_(dst,dst,limbs,lbase);
			cy+=ilmp_add_1_(dst,dst,limbs,curlimb);
			if(cy){
				dst[limbs]=cy;
				++limbs;
			}
		}
	}

	return limbs;
}

//assume src[len-1]!=0
//N=pow->np+pow->zeros
//limbs=return value
//1st level need(nh>=2, [dst,2*N], [tp,limbs])
//recursive need(N>=2, [dst,limbs+1], [tp,2*N-1])
mp_size_t ilmp_from_str_divide_(mp_ptr dst,const mp_byte_t *src,mp_size_t len,mp_basepow_t *pow,mp_ptr tp){
	mp_size_t limbs;
	int base=pow->base;

	if(ilmp_limbs_(0,len,base)<FROM_STR_DIVIDE_THRESHOLD){
		limbs=ilmp_from_str_basecase_(dst,src,len,base);
	}
	else{
		mp_size_t pdigits=pow->digits;
		if(len<=pdigits){
			limbs=ilmp_from_str_divide_(dst,src,len,pow-1,tp);
		}
		else{
			mp_ptr p=pow->p;
			mp_size_t np=pow->np;
			mp_size_t zeros=pow->zeros;
			mp_ptr lp=tp,hp=tp+np+zeros-2;//overwrite 2 limbs
			mp_size_t nl=0,nh;

			mp_size_t llen=pdigits;
			while(llen&&src[llen-1]==0)--llen;
			if(llen)nl=ilmp_from_str_divide_(lp,src,llen,pow-1,dst);
			
			mp_limb_t save0=hp[0],save1=hp[1];//save 2 limbs
			nh=ilmp_from_str_divide_(hp,src+pdigits,len-pdigits,pow-1,dst);
			if(nh>=np)ilmp_mul_(dst+zeros,hp,nh,p,np);
			else ilmp_mul_(dst+zeros,p,np,hp,nh);
			//restore 2 limbs
			hp[0]=save0;
			hp[1]=save1;

			if(zeros<nl){
				ilmp_copy(dst,lp,zeros);
				mp_limb_t cy=ilmp_add_n_(dst+zeros,dst+zeros,lp+zeros,nl-zeros);
				//h*p+l<=(B^nh-1)*p+(p-1)<B^nh*p, limited by nh+np limbs,
				//so inc won't overflow
				if(cy)ilmp_inc(dst+nl);
			}
			else{
				ilmp_copy(dst,lp,nl);
				if(zeros>nl)ilmp_zero(dst+nl,zeros-nl);
			}

			limbs=nh+np+zeros;
			limbs-=dst[limbs-1]==0;
		}
	}

	return limbs;
}

mp_size_t ilmp_from_str_(mp_ptr dst,const mp_byte_t *src,mp_size_t len,int base){
	do{
		if(len==0)return 0;
	}while(src[--len]==0);
	++len;

	mp_size_t limbs;

	if(POW2_Q(base)){
		mp_limb_t curlimb=0;
		const mp_byte_t *srcend=src+len;
		int bitspd=ilmp_bases_[base].large_base;
		int bitpos=0;
		limbs=0;

		do{
			mp_limb_t curdigit=*src;
			curlimb|=curdigit<<bitpos;
			bitpos+=bitspd;
			if(bitpos>=LIMB_BITS){
				dst[limbs]=curlimb;
				++limbs;
				bitpos-=LIMB_BITS;
				curlimb=curdigit>>bitspd-bitpos;
			}
		}while(++src!=srcend);
		if(curlimb){
			dst[limbs]=curlimb;
			++limbs;
		}
	}
	else if(ilmp_limbs_(0,len,base)<FROM_STR_BASEPOW_THRESHOLD){
		limbs=ilmp_from_str_basecase_(dst,src,len,base);
	}
	else{
		TEMP_DECL;
		mp_basepow_t powers[LIMB_BITS];
		mp_limb_t lbase=ilmp_bases_[base].large_base;
		mp_size_t digitspl=ilmp_bases_[base].digits_in_limb;
		mp_size_t bexp,lexp=(len-1)/digitspl+1;
		mp_size_t tzbit=ilmp_tailing_zeros_(lbase);
		//need 1 extra limb to store result
		mp_size_t alloc_size=ilmp_limbs_(0,len,base)+1;
		mp_limb_t cy;
		mp_ptr tp;

		int cpow=ilmp_limb_bits_(lexp-1);

		for(int i=cpow;i>0;--i){
			//we will calculate lbase^bexp
			bexp=(lexp-1>>i)+1;
			//we will calculate lbase^(bexp-1) first, and trim it s. t.
			//it contains at most 2 tailing 0 limb, then multiply it by lbase,
			//so we need npow limbs to store lbase^bexp
			mp_size_t npow=ilmp_limbs_(0,(bexp-1)*digitspl+1,base)+1;

			if(tzbit){
				mp_size_t tzlimb=tzbit*(bexp-1)/LIMB_BITS;
				if(tzlimb>=2)npow-=tzlimb-2;
			}

			//space needed for a trimmed npow-limb lbase^bexp
			alloc_size+=npow;
		}
		
		tp=BALLOC_TYPE(alloc_size,mp_limb_t);
		
		for(int i=0;i<2;++i){
			tp[0]=lbase;
			powers[i].p=tp;
			powers[i].np=1;
			tp+=i+1;
			powers[i].zeros=0;
			powers[i].digits=digitspl*(i+1);
			powers[i].base=base;
		}

		mp_ptr p=powers[1].p;
		mp_size_t zeros=0,np=1;
		for(int i=2;i<cpow;++i){
			ilmp_sqr_(tp,p,np);
			np*=2;
			np-=tp[np-1]==0;
			bexp=lexp-1>>cpow-i;
			if(bexp&1){
				cy=ilmp_mul_1_(tp,tp,np,lbase);
				tp[np]=cy;
				np+=cy!=0;
			}
			zeros*=2;
			while(tp[0]==0){
				//at most 2 tailing 0 limb here
				++zeros;
				++tp;
				--np;
			}
			p=tp;
			powers[i].p=p;
			powers[i].np=np;
			powers[i].zeros=zeros;
			powers[i].digits=digitspl*(bexp+1);
			powers[i].base=base;
			tp+=np+1;
		}

		for(int i=1;i<cpow;++i){
			p=powers[i].p;
			np=powers[i].np;
			cy=ilmp_mul_1_(p,p,np,lbase);
			p[np]=cy;
			np+=cy!=0;
			if(p[0]==0){
				++powers[i].zeros;
				++p;
				--np;
			}

			powers[i].p=p;
			powers[i].np=np;
		}

		limbs=ilmp_from_str_divide_(tp,src,len,powers+cpow-1,dst);
		ilmp_copy(dst,tp,limbs);

		TEMP_FREE;
	}
	return limbs;
}
