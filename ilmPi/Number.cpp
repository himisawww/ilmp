#include"Number.h"
#include<cstdlib>
#include<cstring>
#include<cmath>
#include<algorithm>

namespace ilmp{
	//1 for missing bits in MSlimb, 1 for floor, 0.5 for extra prec
	const mp_prec_t SAFE_PREC_LIMBS=2.0+(mp_prec_t)MIN_PREC_BITS/MP_LIMB_BITS;
	const mp_prec_t RENORM_THRSH=1.5;
	//input & output maybe overprec or overflow
	Number _addsub(const Number&,const Number&,bool is_add);
	Number _mul(const Number&,const Number&);
	Number _div(const Number&,const Number&);
	Number _sqrt(const Number&);
	Number _pow(const Number&,mp_int);

	// log2(2^x+2^y)
	mp_prec_t log2add(mp_prec_t x,mp_prec_t y){
		if(x==y)return x+1.0;
		mp_prec_t mxy,z;
		if(x>y){
			mxy=x;
			z=y-x;
		}
		else{
			mxy=y;
			z=x-y;
		}
		return mxy+std::log2(1.0+std::exp2(z));
	}

	Number::Number(){
		data=nullptr;
		clear();
	}
	Number::Number(bool x){
		data=nullptr;
		from_int((int)x);
	}
	Number::Number(char x){
		data=nullptr;
		from_int(x);
	}	
	Number::Number(signed char x){
		data=nullptr;
		from_int(x);
	}
	Number::Number(unsigned char x){
		data=nullptr;
		from_uint(x);
	}
	Number::Number(signed short x){
		data=nullptr;
		from_int(x);
	}
	Number::Number(unsigned short x){
		data=nullptr;
		from_uint(x);
	}
	Number::Number(signed int x){
		data=nullptr;
		from_int(x);
	}
	Number::Number(unsigned int x){
		data=nullptr;
		from_uint(x);
	}
	Number::Number(signed long x){
		data=nullptr;
		from_int(x);
	}
	Number::Number(unsigned long x){
		data=nullptr;
		from_uint(x);
	}
	Number::Number(signed long long x){
		data=nullptr;
		from_int(x);
	}
	Number::Number(unsigned long long x){
		data=nullptr;
		from_uint(x);
	}
	Number::Number(float x){
		data=nullptr;
		from_float(&x,sizeof(float));
	}
	Number::Number(double x){
		data=nullptr;
		from_float(&x,sizeof(double));
	}
	Number::Number(long double x){
		data=nullptr;
		from_float(&x,sizeof(long double));
	}
	Number::Number(Integer &&i){
		data=nullptr;
		if(i.ssize==0){
			clear();
			if(!i.is_nan())prec=INT_PREC;
		}
		else{
			ssize=i.ssize;
			dotp=0;
			prec=INT_PREC;
			if(i.data!=i.value)data=i.data;
			else{
				mp_int isize=i.size();
				data=new mp_limb_t[isize];
				memcpy(data,i.data,isize*sizeof(mp_limb_t));
			}
			i.data=nullptr;
		}
		i.clear();
	}
	Number::Number(const Integer &i){
		data=nullptr;
		if(i.ssize==0){
			clear();
			if(!i.is_nan())prec=INT_PREC;
		}
		else{
			mp_int isize=i.size();
			data=new mp_limb_t[isize];
			memcpy(data,i.data,isize*sizeof(mp_limb_t));
			ssize=i.ssize;
			dotp=0;
			prec=INT_PREC;
		}
	}
	Number::Number(Number &&x){
		data=x.data;
		ssize=x.ssize;
		dotp=x.dotp;
		prec=x.prec;
		x.data=nullptr;
		x.clear();
	}
	Number::Number(const Number &x){
		data=nullptr;
		*this=x;
	}
	Number::Number(const char *str,int base){
		data=nullptr;
		from_str(str,base);
	}
	Number::~Number(){
		clear();
	}

	void Number::clear(){
		if(data)delete[] data;
		data=nullptr;
		ssize=0;
		dotp=0;
		prec=NAN_PREC;
	}
	void Number::from_int(mp_int n){
		clear();
		bool sign=n<0;
		if(n){
			data=new mp_limb_t[1];
			data[0]=sign?-n:n;
			ssize=sign?-1:1;
		}
		prec=INT_PREC;
	}
	void Number::from_uint(mp_uint n){
		clear();
		if(n){
			data=new mp_limb_t[1];
			data[0]=n;
			ssize=1;
		}
		prec=INT_PREC;
	}

	//TODO
	//from half,single,extended,double,double extended,quadruple,octuple
	/*	void Number::from_float(double inflt){
	_setzero(MinI);
	word wflt=*(word*)&inflt;
	mp_int fpow=wflt>>52&2047;
	sign=wflt>>63;
	wflt<<=12;
	if(fpow==0&&wflt==0){
	sign=0;
	prec=MaxI;
	}
	else if(fpow!=2047){
	num=(word*)malloc(16);
	prec=53;
	words=1;
	if(fpow==0){
	dot=17;
	*num=wflt<<2;
	if(num[1]=wflt>>62)++words;
	}
	else{
	wflt>>=12;
	wflt|=(word)1<<52;
	fpow-=1075;
	dot=-(fpow>>6);
	fpow+=64*dot;
	*num=wflt<<fpow;
	if(fpow>11){
	num[1]=wflt>>64-fpow;
	++words;
	}
	}
	}
	}
	*/
	void Number::from_float(const void *pftype,mp_int tsize){
		clear();

	}

	void Number::from_str(const char *pstr,int base){
		clear();
		mp_int i=0;
		mp_byte_t covbuf[MP_LIMB_BITS];
		mp_limb_t covval[2];

		if(base<0)base=-base;
		if(base<2||base>36)return;

		//sign
		bool sign=pstr[i]=='-';
		if(pstr[i]=='+'||sign)++i;

		//inf
		if(pstr[i]=='#'&&(pstr[i+1]=='i'||pstr[i+1]=='I')){
			if((pstr[i+2]=='n'||pstr[i+2]=='N')&&(pstr[i+3]=='f'||pstr[i+3]=='F')){
				if(pstr[i+4]==0){
					ssize=sign?-1:1;
					return;
				}
			}
		}

		//data
		bool mhval=false;
		mp_int mdot=-1,mstart=-1,mend;
		do{
			char c=pstr[i];
			if(c=='.'){
				if(mdot==-1)mdot=i;
				else return;
			}
			else{
				int val;
				if(c>='0'&&c<='9')val=c-'0';
				else if(c>='A'&&c<='Z')val=c-'A'+10;
				else if(c>='a'&&c<='z')val=c-'a'+10;
				else {
					mend=i;
					break;
				}
				if(val){
					if(val>=base)return;
					if(mstart==-1)mstart=i;
				}
				mhval=true;
			}
			++i;
		} while(1);
		if(!mhval)return;

		bool haspval=false,haseval=false;
		mp_int pval,eval=0;
		for(int idx=0;idx<2;++idx){
			if(idx==0){
				if(pstr[i]!='`')continue;
				haspval=true;
				++i;
			}
			if(idx==1){
				if(pstr[i]!='*'||pstr[i+1]!='^')continue;
				haseval=true;
				i+=2;
			}
			bool pesign=pstr[i]=='-';
			if(pstr[i]=='+'||pesign)++i;

			bool pehval=false;
			mp_int pestart=-1;
			mp_int ip=MP_LIMB_BITS;
			do{
				char c=pstr[i];
				int val;
				if(c>='0'&&c<='9')val=c-'0';
				else if(c>='A'&&c<='Z')val=c-'A'+10;
				else if(c>='a'&&c<='z')val=c-'a'+10;
				else break;
				if(val){
					if(val>=base)return;
					if(pestart==-1)pestart=i;
				}
				pehval=true;
				if(pestart!=-1){
					covbuf[--ip]=val;
					if(ip==0)return;
				}
				++i;
			} while(1);
			if(!pehval)return;
			mp_int peres=0;
			if(pestart!=-1){
				if(ilmp_limbs_(covbuf+ip,MP_LIMB_BITS-ip,base)>2)
					return;
				if(ilmp_from_str_(covval,covbuf+ip,MP_LIMB_BITS-ip,base)==2)
					return;
				if(covval[0]>MAX_PREC_INT)return;
				peres=covval[0];
			}
			if(pesign)peres=-peres;
			if(idx==0)pval=peres;
			if(idx==1)eval=peres;
		}

		if(pstr[i])return;

		bool isint=!haspval&&mdot==-1&&!haseval;
		mp_int offset=mdot!=-1?mend-1-mdot:0;
		const mp_prec_t log2base=std::log2((mp_prec_t)base);

		if(mstart==-1){
			if(isint)prec=INT_PREC;
			else{
				prec=((haspval?pval:0)+offset-eval)*log2base;
				if(prec>=MAX_EXP_BITS)prec=INT_PREC;
				if(prec<-MAX_EXP_BITS)prec=NAN_PREC;
			}
			return;
		}

		mp_int digits=mend-mstart-(mdot>mstart);

		mp_int ih=MP_LIMB_BITS;
		mp_int mdigits=std::round(MP_LIMB_BITS/log2base);
		mp_int lcovlen;
		if(mdigits>digits)mdigits=digits;
		for(mp_int ipos=mstart;ih>MP_LIMB_BITS-mdigits;++ipos){
			if(ipos==mdot)continue;
			char c=pstr[ipos];
			int val;
			if(c<='9')val=c-'0';
			else val=(c&0x1F)+9;
			covbuf[--ih]=val;
		}
		mp_prec_t apprlog2,mprec=0;
		lcovlen=ilmp_from_str_(covval,covbuf+ih,mdigits,base);
		if(lcovlen==2)mprec=MP_BASE*covval[1];
		mprec+=covval[0];
		mprec=std::log2(mprec)+(digits-mdigits)*log2base;
		apprlog2=mprec+(eval-offset)*log2base;
		if(haspval)mprec=pval*log2base;
		else if(mprec<MIN_PREC_BITS)mprec=MIN_PREC_BITS;
		if(mprec>MAX_PREC_BITS)mprec=MAX_PREC_BITS;
		bool mpv=mprec<=0;
		if(mpv)apprlog2-=mprec;
		if(apprlog2>MAX_EXP_BITS){
			if(!mpv)ssize=sign?-1:1;
			return;
		}
		if(apprlog2<-MAX_EXP_BITS){
			prec=INT_PREC;
			return;
		}
		if(mpv){
			prec=-apprlog2;
			return;
		}

		if(isint)prec=INT_PREC;
		else{
			prec=mprec;
			mp_int pdigits=std::round((prec+MP_LIMB_BITS)/log2base);
			if(pdigits<digits){
				offset-=digits-pdigits;
				digits=pdigits;
			}
		}
		
		if(mdigits==digits){
			data=new mp_limb_t[lcovlen];
			memcpy(data,covval,sizeof(mp_limb_t)*lcovlen);
			ssize=lcovlen;
		}
		else{
			mp_byte_t *lcovbuf=new mp_byte_t[digits];
			mp_int lip=digits;
			for(mp_int ipos=mstart;lip>0;++ipos){
				if(ipos==mdot)continue;
				char c=pstr[ipos];
				int val;
				if(c<='9')val=c-'0';
				else val=(c&0x1F)+9;
				lcovbuf[--lip]=val;
			}
			lcovlen=ilmp_limbs_(lcovbuf,digits,base);
			data=new mp_limb_t[lcovlen];
			ssize=ilmp_from_str_(data,lcovbuf,digits,base);
			delete[] lcovbuf;
		}
		if(sign)ssize=-ssize;

		eval-=offset;

		if(eval){
			mp_prec_t pushprec=prec,calcprec=prec+1;
			Number basepow(base);
			basepow.prec=calcprec+std::log2(std::abs((mp_prec_t)eval));
			prec=calcprec;
			*this=_mul(*this,_pow(basepow,eval));
			prec=pushprec;
		}
		normalize();
	}

	//TODO
	mp_int Number::to_int() const{
		return 0;
	}
	mp_uint Number::to_uint() const{
		return 0;
	}
	void Number::to_float(void *pftype,mp_int tsize) const{

	}
	mp_int Number::strlen(int base) const{
		if(base<0)base=-base;
		if(base<2||base>36)return 1;
		if(is_inf()||is_nan())return 6;
		const mp_prec_t log2base=std::log2((mp_prec_t)base);

		mp_int lm,ls=ssize<0,le;
		if(is_int()){
			le=0;
			if(ssize==0)lm=1;
			else lm=(mp_int)(log2()/log2base)+2;
		}
		else{
			mp_prec_t ev;
			if(ssize==0){
				ev=prec;
				lm=1;
			}
			else{
				ev=log2();
				lm=(mp_int)(prec/log2base)+3;
			}
			le=(mp_int)(std::log2(1+std::abs(ev))/log2base)+6;
		}
		return lm+ls+le+1;
	}
	void Number::to_str(char *pstr,int base) const{
		char c10='a';

		if(base<0){
			base=-base;
			c10='A';
		}
		if(base<2||base>36){
			*pstr=0;
			return;
		}

		const mp_prec_t log2base=std::log2((mp_prec_t)base);
		if(is_inf()){
			if(ssize<0){
				*pstr='-';
				++pstr;
			}
			pstr[0]='#';
			pstr[1]='i';
			pstr[2]='n';
			pstr[3]='f';
			pstr[4]=0;
			return;
		}
		if(is_nan()){
			pstr[0]='#';
			pstr[1]='n';
			pstr[2]='a';
			pstr[3]='n';
			pstr[4]=0;
			return;
		}
		
		mp_int eval=0;

		if(ssize==0){
			pstr[0]='0';
			if(is_int()){
				pstr[1]=0;
				return;
			}
			pstr[1]='.';
			pstr+=2;
			eval=std::round(-prec/log2base);
		}
		else{
			Number lint;
			const Number *plint;
			if(is_int())plint=this;
			else{
				eval=std::floor((prec-log2())/log2base);
				if(eval){
					Number basepow(base);
					basepow.prec=prec+std::log2(std::abs((mp_prec_t)eval));
					lint=_mul(*this,_pow(basepow,eval));
					eval=-eval;
					plint=&lint;
				}
				else plint=this;
			}

			mp_limb_t carry=0;
			mp_int pdotp=plint->dotp,psize=plint->size()-pdotp;//psize>=0
			mp_srcptr psrc=plint->data;
			mp_ptr intval=new mp_limb_t[psize+1];
			if(pdotp<=0){
				if(pdotp<0)memset(intval,0,-pdotp*sizeof(mp_limb_t));
				memcpy(intval-pdotp,psrc,(psize+pdotp)*sizeof(mp_limb_t));
			}
			if(pdotp>0){
				carry=psrc[pdotp-1]>>MP_LIMB_BITS-1;
				if(psize)carry=ilmp_add_1_(intval,psrc+pdotp,psize,carry);
				intval[psize]=carry;
				psize+=carry;
			}

			mp_int digits;
			mp_byte_t *intstr;
			if(psize==0){
				digits=1;
				intstr=new mp_byte_t[1];
				intstr[0]=0;
			}
			else{
				if(ssize<0){
					*pstr='-';
					++pstr;
				}
				digits=ilmp_digits_(intval,psize,base);
				intstr=new mp_byte_t[digits];
				digits=ilmp_to_str_(intstr,intval,psize,base);
			}
			delete[] intval;
		
			if(!is_int()){
				eval+=digits-1;
				mp_int front=eval+1;
				if(front>digits||std::abs(eval)>=MIN_EXP_SCI_OUT){
					front=1;
				}
				else{
					eval=0;
					if(front<=0){
						pstr[0]='0';
						pstr[1]='.';
						pstr+=2;
						while(front){
							++front;
							*pstr='0';
							++pstr;
						}
					}
				}

				if(front){
					do{
						int val=intstr[--digits];
						*pstr=val<10?'0'+val:c10+val-10;
						++pstr;
					} while(--front);
					*pstr='.';
					++pstr;
				}
			}
			
			while(digits){
				int val=intstr[--digits];
				*pstr=val<10?'0'+val:c10+val-10;
				++pstr;
			}

			delete[] intstr;
		}

		if(eval!=0){
			pstr[0]='*';
			pstr[1]='^';
			pstr+=2;
			mp_limb_t scov=eval;
			if(eval<0){
				*pstr='-';
				++pstr;
				scov=-eval;
			}
			mp_byte_t covbuf[MP_LIMB_BITS];
			mp_int len=ilmp_to_str_(covbuf,&scov,1,base);
			do{
				int val=covbuf[--len];
				*pstr=val<10?'0'+val:c10+val-10;
				++pstr;
			} while(len);
		}
		*pstr=0;
	}

	void Number::neg(){
		ssize=-ssize;
	}
	int Number::sign() const{
		if(ssize<0)return -1;
		if(ssize>0)return 1;
		return 0;
	}
	mp_int Number::size() const{
		return ssize<0?-ssize:ssize;
	}
	mp_int Number::precision_size() const{
		if(!data)return 0;
		if(is_int())return MP_MAX_INT;
		return prec/MP_LIMB_BITS+SAFE_PREC_LIMBS;
	}
	mp_prec_t Number::log2(mp_int shift) const{
		if(!data)return -prec;
		mp_int asize=size();
		mp_prec_t ms=0.;
		if(asize>=2)ms=data[asize-2]/MP_BASE;
		ms=(ms+data[asize-1])/MP_BASE;
		ms=std::log2(ms);
		if(std::abs(shift)<MAX_PREC_INT)
			shift+=MP_LIMB_BITS*(asize-dotp);
		else
			ms+=MP_LIMB_BITS*(asize-dotp);
		return ms+shift;
	}
	mp_prec_t Number::precision() const{
		return prec;
	}
	void Number::set_precision(mp_prec_t new_prec){
		if(is_nan())return;
		if(is_inf()){
			if(new_prec<=0)clear();
			return;
		}
		if(!data){
			if(is_int())return;
			prec+=new_prec;
			if(prec>=MAX_EXP_BITS)prec=INT_PREC;
			if(prec<-MAX_EXP_BITS)prec=NAN_PREC;
			return;
		}
		
		if(new_prec<=0){
			mp_prec_t old_prec=log2();
			clear();
			prec=new_prec-old_prec;
			if(prec>=MAX_EXP_BITS)prec=INT_PREC;
			if(prec<-MAX_EXP_BITS)prec=NAN_PREC;
			return;
		}

		prec=new_prec;
		
		normalize();
	}
	int Number::is_inf() const{
		return prec==NAN_PREC?ssize:0;
	}
	bool Number::is_nan() const{
		return prec==NAN_PREC&&ssize==0;
	}
	bool Number::is_int() const{
		return prec==INT_PREC;
	}
	void Number::swap(Number &x){
		mp_ptr _tm=data;
		mp_int _ts=ssize;
		mp_int _td=dotp;
		mp_prec_t _tp=prec;
		data=x.data;
		ssize=x.ssize;
		dotp=x.dotp;
		prec=x.prec;
		x.data=_tm;
		x.ssize=_ts;
		x.dotp=_td;
		x.prec=_tp;
	}
	void Number::normalize(){
		if(is_inf()||is_nan())return;
		if(ssize==0){
			if(prec>=MAX_EXP_BITS)prec=INT_PREC;
			if(prec<-MAX_EXP_BITS)prec=NAN_PREC;
			return;
		}

		if(!is_int()){
			if(prec<=0){
				mp_prec_t old_prec=prec-log2();
				clear();
				prec=old_prec;
				if(prec>=MAX_EXP_BITS)prec=INT_PREC;
				if(prec<-MAX_EXP_BITS)prec=NAN_PREC;
				return;
			}
			if(prec>MAX_PREC_BITS)prec=MAX_PREC_BITS;
		}

		mp_int asize=size();
		if(asize-dotp>MAX_EXP_LIMBS){
			int old_sign=sign();
			clear();
			ssize=old_sign;
			return;
		}
		if(asize-dotp<=-MAX_EXP_LIMBS){
			clear();
			prec=INT_PREC;
			return;
		}

		mp_int nsize;
		if(is_int()){
			if(asize<=dotp){
				clear();
				prec=INT_PREC;
				return;
			}
			else nsize=asize-dotp;
		}
		else nsize=precision_size();
	
		if(nsize>asize)nsize=asize;
		while(data[asize-nsize]==0)--nsize;

		if(is_int()&&dotp>0||asize-RENORM_THRSH*nsize>0){
			mp_int cutoff=asize-nsize;
			mp_ptr nm=new mp_limb_t[nsize];
			memcpy(nm,data+cutoff,nsize*sizeof(mp_limb_t));
			delete[] data;
			data=nm;
			dotp-=cutoff;
			ssize=ssize<0?-nsize:nsize;
		}
	}

	Number &Number::operator =(Number &&Num){
		if(this!=&Num){
			swap(Num);
			Num.clear();
		}
		return *this;
	}
	Number &Number::operator =(const Number &Num){
		if(this!=&Num){
			clear();
			ssize=Num.ssize;
			dotp=Num.dotp;
			prec=Num.prec;
			if(Num.data){
				mp_int asize=size();
				data=new mp_limb_t[asize];
				memcpy(data,Num.data,asize*sizeof(mp_limb_t));
			}
		}
		return *this;
	}
	Number &Number::operator+=(const Number &Num){
		*this=*this+Num;
		return *this;
	}
	Number &Number::operator-=(const Number &Num){
		*this=*this-Num;
		return *this;
	}
	Number &Number::operator*=(const Number &Num){
		*this=*this*Num;
		return *this;
	}
	Number &Number::operator/=(const Number &Num){
		*this=*this/Num;
		return *this;
	}

	int compare(const Number &Num1,const Number &Num2,bool abscomp){
		if(Num1.is_nan()||Num2.is_nan())return -2;
		int sign1=Num1.sign(),sign2=Num2.sign();
		if(abscomp){
			sign1=std::abs(sign1);
			sign2=std::abs(sign2);
		}
		if(sign1>sign2)return 1;
		if(sign1<sign2)return -1;
		if(sign1){
			if(!Num1.data)return Num2.data?sign1:0;
			if(!Num2.data)return -sign1;
			mp_int na=Num1.size(),nb=Num2.size();
			mp_int ea=na-Num1.dotp,eb=nb-Num2.dotp;
			if(ea>eb)return sign1;
			if(ea<eb)return -sign1;
			mp_ptr pa=Num1.data+na,pb=Num2.data+nb;
			do{
				if(*--pa!=*--pb)
					return sign1*(*pa>*pb?1:-1);
				--na;--nb;
			} while(na>0&&nb>0);
			int coef=1;
			if(nb>0){
				na=nb;pa=pb;coef=-1;
			}
			while(na>0){
				if(*--pa!=0)return sign1*coef;
				--na;
			}
			return 0;
		}
		return 0;
	}
	bool operator==(const Number &Num1,const Number &Num2){ return compare(Num1,Num2)==0; }
	bool operator!=(const Number &Num1,const Number &Num2){ return (compare(Num1,Num2)&1)==1; }
	bool operator>(const Number &Num1,const Number &Num2){ return compare(Num1,Num2)==1; }
	bool operator>=(const Number &Num1,const Number &Num2){ return compare(Num1,Num2)>=0; }
	bool operator<(const Number &Num1,const Number &Num2){ return compare(Num2,Num1)==1; }
	bool operator<=(const Number &Num1,const Number &Num2){ return compare(Num2,Num1)>=0; }

	//|num1|+-|num2|, need(num1!=0, num2!=0, isadd|||num1|>=|num2|)
	//no nan or inf or zero allowed
	void _addsubnzabs(Number &result,const Number &Num1,const Number &Num2,bool is_add){
		mp_int size1=Num1.size(),size2=Num2.size();
		mp_int sstarta=size1-Num1.dotp;
		mp_int sstartb=size2-Num2.dotp;
		mp_int rstart=std::max(sstarta,sstartb);
		mp_int senda=Num1.is_int()?MP_MIN_INT:sstarta-Num1.precision_size();
		mp_int sendb=Num2.is_int()?MP_MIN_INT:sstartb-Num2.precision_size();
		mp_int hend=-std::max(Num1.dotp,Num2.dotp);
		mp_int rend=std::max(std::max(senda,sendb),hend);
		mp_int rn=rstart-rend+1;
		
		mp_int sha=Num1.dotp+rend,shb=Num2.dotp+rend;
		mp_srcptr pa=Num1.data,pb=Num2.data;
		mp_ptr pr=new mp_limb_t[rn];
	
		mp_limb_t cb=0;
		for(mp_int i=0;i<rn;++i){
			mp_limb_t srca=0,srcb=0;
			mp_int ia=sha+i,ib=shb+i;
			if(ia>=0&&ia<size1)srca=pa[ia];
			if(ib>=0&&ib<size2)srcb=pb[ib];
			mp_limb_t dst;
			if(is_add){
				dst=srca+srcb+cb;
				cb=cb?dst<=srca:dst<srca;
			}
			else{
				dst=srca-srcb-cb;
				cb=cb?dst>=srca:dst>srca;
			}
			pr[i]=dst;
		}
		
		while(rn>0&&pr[rn-1]==0)--rn;

		if(rn>0){
			result.ssize=rn;
			result.data=pr;
			result.dotp=-rend;
		}
		else{
			delete[] pr;
		}
	}
	Number _addsub(const Number &Num1,const Number &Num2,bool is_add){
		Number result;
		if(Num1.is_nan()||Num2.is_nan()){
			return result;
		}
		int inf1=Num1.is_inf(),inf2=Num2.is_inf();
		if(inf1||inf2){
			if(!inf2)result.ssize=inf1;
			else if(!inf1)result.ssize=is_add?inf2:-inf2;
			else if((inf1==inf2)==is_add)result.ssize=inf1;
			return result;			
		}
		if(Num1.ssize==0&&Num2.ssize==0){
			result.prec=-log2add(-Num1.prec,-Num2.prec);
			return result;
		}

		mp_prec_t abs1=Num1.log2(),abs2=Num2.log2();
		mp_int absbase=std::round(std::fmax(abs1,abs2));
		abs1=Num1.log2(-absbase);
		abs2=Num2.log2(-absbase);

		mp_prec_t sigma1,sigma2,sigma;
		if(Num1.ssize==0)sigma1=-absbase-Num1.prec;
		else sigma1=abs1-Num1.prec;
		if(Num2.ssize==0)sigma2=-absbase-Num2.prec;
		else sigma2=abs2-Num2.prec;
		sigma=log2add(sigma1,sigma2);

		if(Num1.ssize==0){
			result=Num2;
			if(!is_add)result.neg();
			result.prec=abs2-sigma;
			return result;
		}
		if(Num2.ssize==0){
			result=Num1;
			result.prec=abs1-sigma;
			return result;
		}

		int sign1=Num1.sign(),sign2=Num2.sign(),signres=sign1;
	
		if(is_add==(sign1==sign2)){
			_addsubnzabs(result,Num1,Num2,true);
		}
		else{
			int cmp12=compare(Num1,Num2,true);
			signres*=cmp12;
			if(cmp12!=0){
				if(cmp12>0){
					_addsubnzabs(result,Num1,Num2,false);
				}
				else{
					_addsubnzabs(result,Num2,Num1,false);
				}
			}
		}
	
		result.ssize*=signres;
		if(result.ssize==0)
			result.prec=-absbase-sigma;
		else
			result.prec=result.log2(-absbase)-sigma;
		return result;
	}
	Number _mul(const Number &Num1,const Number &Num2){
		Number result;
		if(Num1.is_nan()||Num2.is_nan())return result;
		int inf1=Num1.is_inf(),inf2=Num2.is_inf();
		int sign1=Num1.sign(),sign2=Num2.sign();
		if(inf1||inf2){
			result.ssize=sign1*sign2;
			return result;
		}
		mp_int size1=Num1.size(),size2=Num2.size();
		if(size1==0||size2==0){
			result.prec=-(Num1.log2()+Num2.log2());
			return result;
		}
		mp_srcptr pa=Num1.data+size1;
		mp_srcptr pb=Num2.data+size2;
		
		mp_int pn=std::min(Num1.precision_size(),Num2.precision_size());

		mp_int na=std::min(size1,pn);
		mp_int nb=std::min(size2,pn);

		while(pa[-na]==0)--na;
		while(pb[-nb]==0)--nb;

		mp_int rn=na+nb;
		mp_ptr pr=new mp_limb_t[rn];

		if(na>=nb)ilmp_mul_(pr,pa-na,na,pb-nb,nb);
		else ilmp_mul_(pr,pb-nb,nb,pa-na,na);
		while(pr[rn-1]==0)--rn;

		result.data=pr;
		result.ssize=sign1==sign2?rn:-rn;
		result.dotp=(na-size1+Num1.dotp)+(nb-size2+Num2.dotp);
		result.prec=-log2add(-Num1.prec,-Num2.prec);
		return result;
	}
	Number _div(const Number &Num1,const Number &Num2){
		Number result;
		if(Num1.is_nan()||Num2.ssize==0)return result;
		int inf1=Num1.is_inf(),inf2=Num2.is_inf();
		if(inf2){
			if(!inf1)result.prec=INT_PREC;
			return result;
		}
		int sign1=Num1.sign(),sign2=Num2.sign();
		if(inf1){
			result.ssize=sign1*sign2;
			return result;
		}
		mp_int size1=Num1.size(),size2=Num2.size();
		if(size1==0){
			result.prec=Num1.prec+Num2.log2();
			return result;
		}

		mp_srcptr pa=Num1.data+size1,pb=Num2.data+size2;
		mp_int na,nb;
		int signres=sign1==sign2?1:-1;
		if(Num1.is_int()&&Num2.is_int()){
			int cmp12=compare(Num1,Num2,true);
			if(cmp12==-1){
				result.prec=INT_PREC;
				return result;
			}
			if(cmp12==0){
				result.from_int(signres);
				return result;
			}
			
			nb=size2;
			while(pb[-nb]==0)--nb;
			na=size1-Num1.dotp+(nb-size2+Num2.dotp);
		}
		else{
			mp_int pn=std::min(Num1.precision_size(),Num2.precision_size());
			nb=std::min(size2,pn);
			while(pb[-nb]==0)--nb;
			na=nb+pn;
		}

		mp_ptr tmp;
		if(na>size1){
			tmp=new mp_limb_t[na];
			memset(tmp,0,(na-size1)*sizeof(mp_limb_t));
			memcpy(tmp+na-size1,Num1.data,size1*sizeof(mp_limb_t));
			pa=tmp;
		}
		else pa-=na;

		mp_int rn=na-nb+1;
		mp_ptr pr=new mp_limb_t[rn];
		ilmp_div_(pr,nullptr,pa,na,pb-nb,nb);

		while(pr[rn-1]==0)--rn;
		if(na>size1)delete[] tmp;

		result.data=pr;
		result.ssize=signres*rn;
		result.dotp=(na-size1+Num1.dotp)-(nb-size2+Num2.dotp);
		result.prec=-log2add(-Num1.prec,-Num2.prec);

		return result;
	}

	Number operator-(const Number &Num){
		Number result(Num);
		result.neg();
		return result;
	}
	Number operator+(const Number &Num){
		Number result(Num);
		return result;
	}
	Number operator+(const Number &Num1,const Number &Num2){
		Number result=_addsub(Num1,Num2,true);
		result.normalize();
		return result;
	}
	Number operator-(const Number &Num1,const Number &Num2){
		Number result=_addsub(Num1,Num2,false);
		result.normalize();
		return result;
	}
	Number operator*(const Number &Num1,const Number &Num2){
		Number result;
		if(Num1.data&&Num2.data){
			mp_prec_t nlog2x=Num1.log2()+Num2.log2();
			if(nlog2x>MAX_EXP_BITS){
				result.ssize=Num1.sign()*Num2.sign();
				return result;
			}
			if(nlog2x<-MAX_EXP_BITS){
				result.prec=INT_PREC;
				return result;
			}
		}
		result=_mul(Num1,Num2);
		result.normalize();
		return result;
	}
	Number operator/(const Number &Num1,const Number &Num2){
		Number result;
		if(Num1.data&&Num2.data){
			mp_prec_t nlog2x=Num1.log2()-Num2.log2();
			if(nlog2x>MAX_EXP_BITS){
				result.ssize=Num1.sign()*Num2.sign();
				return result;
			}
			if(nlog2x<-MAX_EXP_BITS){
				result.prec=INT_PREC;
				return result;
			}
		}
		result=_div(Num1,Num2);
		result.normalize();
		return result;
	}

	Number _sqrt(const Number &x){
		Number result;
		if(x.is_nan()||x.ssize<0)return result;
		if(x.is_inf()){
			result.ssize=1;
			return result;
		}
		if(x.ssize==0){
			result.prec=x.prec*0.5;
			return result;
		}

		mp_int na=x.size(),dotpx=x.dotp;
		mp_srcptr px=x.data+na;
		int shr32=dotpx&1;
		dotpx-=shr32;
		mp_int nf;
		bool intx=x.is_int();
		if(intx){
			nf=-dotpx/2;
		}
		else{
			mp_int pn=x.precision_size();
			nf=pn-na/2;
			if(nf<0){
				na+=nf*2;
				nf=0;
			}
		}

		mp_int rn=nf+na/2+1;
		mp_ptr pa=nullptr,pr=new mp_limb_t[rn];
		if(intx)pa=new mp_limb_t[rn];
		ilmp_sqrt_(pr,pa,px-na,na,nf);
		if(intx)delete[] pa;
		if(shr32)ilmp_shr_(pr,pr,rn,32);
		while(pr[rn-1]==0)--rn;

		result.data=pr;
		result.ssize=rn;
		result.dotp=nf+dotpx/2;
		result.prec=x.prec+1;
		return result;
	}
	Number sqrt(const Number &x){
		Number result=_sqrt(x);
		result.normalize();
		return result;
	}

	Number _pow(const Number &x,mp_int n){
		Number result;
		if(x.is_nan())return result;
		if(x.is_inf()){
			result.ssize=x.ssize<0&&(n&1)?-1:1;
			return result;
		}
		if(x.ssize==0){
			if(n>0)result.prec=x.prec*n;
			return result;
		}		
		result.from_int(1);
		if(n==0)return result;
		bool sign=n<0;
		mp_uint un=sign?-n:n;
		mp_uint i=(mp_uint)1<<MP_LIMB_BITS-1;
		mp_prec_t resprec=x.prec-std::log2((mp_prec_t)un);
		while(!(un&i))i>>=1;
		do{
			result=_mul(result,result);
			if(un&i)result=_mul(x,result);
		} while(i>>=1);
		if(sign)result=_div(1,result);
		result.prec=resprec;
		return result;
	}
	Number pow(const Number &x,mp_int n){
		Number result;
		if(x.data){
			mp_prec_t nlog2x=n*x.log2();
			if(nlog2x>MAX_EXP_BITS){
				result.ssize=x.ssize<0&&(n&1)?-1:1;
				return result;
			}
			if(nlog2x<-MAX_EXP_BITS){
				result.prec=INT_PREC;
				return result;
			}
		}
		result=_pow(x,n);
		result.normalize();
		return result;
	}
}


