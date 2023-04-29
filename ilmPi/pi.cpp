#include<algorithm>
#include<cmath>
#include<cstdint>
#include<cstring>
#include"Number.h"

#define A ((mp_uint)13591409)
#define B ((mp_uint)545140134)
#define C ((mp_uint)640320)
#define MAX_DEPTH 64

namespace ilmp{
	typedef struct{
		//smallest factor for composite
		//otherwise 0
		uint32_t factor;

		//L 1byte:power of factor
		//H 3byte:low 24 bits of 56-bit next
		uint32_t pnex;

		//high 32 bits of 56-bit next
		uint32_t next;
	} sieve_t;

	class factor_t{
	public:
		mp_uint capacity;
		mp_uint size;
		mp_uint *factor;
		mp_uint *power;

		factor_t();
		~factor_t();
	};

	class pi_calc{
	public:
		Number pi;
		sieve_t *sieve;
		Integer *ps,*qs,*ts,*gcds;
		factor_t *fps,*fqs,*fmul,*fbp;

		void factor_resize(factor_t &,mp_uint);
		void factor_compact(factor_t &);
		void factor_set_bp(factor_t &,mp_uint,int);
		void factor_mul_bp(factor_t &,mp_uint,int);
		void factor_mul(factor_t &,const factor_t &);
		void factor_cancel(Integer &,factor_t &,Integer &,factor_t &);
		void factor_swap(factor_t &,factor_t &);
		void build_sieve(mp_uint n);
		void bs_mulgcd(mp_uint a,mp_uint b,int top);
		void bs_chudnovsky(mp_uint a,mp_uint b,int needp,int level,int top);
		pi_calc(mp_uint digits,int base=10);
	};

	factor_t::factor_t(){
		capacity=MAX_DEPTH;
		size=0;
		factor=new mp_uint[MAX_DEPTH*2];
		power=factor+MAX_DEPTH;
	}
	factor_t::~factor_t(){
		if(factor)delete[] factor;
	}

	void pi_calc::factor_resize(factor_t &fp,mp_uint new_size){
		if(fp.capacity<new_size){
			if(fp.factor)delete[] fp.factor;
			fp.capacity=new_size;
			fp.size=0;
			fp.factor=new mp_uint[new_size*2];
			fp.power=fp.factor+new_size;
		}
	}
	void pi_calc::factor_compact(factor_t &fp){
		mp_uint j=0;
		for(mp_uint i=0;i<fp.size;++i){
			if(fp.power[i]){
				if(j!=i){
					fp.factor[j]=fp.factor[i];
					fp.power[j]=fp.power[i];
				}
				++j;
			}
		}
		fp.size=j;
	}
	void pi_calc::factor_set_bp(factor_t &fp,mp_uint base,int pow){
		base/=2;
		mp_uint fsize=0;
		while(base){
			++fsize;
			mp_uint factor=sieve[base].factor;
			if(!factor)factor=base*2+1;
			fp.factor[fsize-1]=factor;
			fp.power[fsize-1]=pow*(sieve[base].pnex&0x000000FF);
			base=(mp_uint)sieve[base].next<<24|sieve[base].pnex>>8;
		}
		fp.size=fsize;
	}
	void pi_calc::factor_mul_bp(factor_t &fp,mp_uint base,int pow){
		factor_set_bp(fbp[0],base,pow);
		factor_mul(fp,fbp[0]);
	}
	void pi_calc::factor_mul(factor_t &fp1,const factor_t &fp2){
		factor_resize(fmul[0],fp1.size+fp2.size);
		mp_uint i=0,j=0,k=0;
		while(i<fp1.size&&j<fp2.size){
			if(fp1.factor[i]==fp2.factor[j]){
				fmul->factor[k]=fp1.factor[i];
				fmul->power[k]=fp1.power[i]+fp2.power[j];
				++i;++j;
			}
			else if(fp1.factor[i]<fp2.factor[j]){
				fmul->factor[k]=fp1.factor[i];
				fmul->power[k]=fp1.power[i];
				++i;
			}
			else{
				fmul->factor[k]=fp2.factor[j];
				fmul->power[k]=fp2.power[j];
				++j;
			}
			++k;
		}
		while(i<fp1.size){
			fmul->factor[k]=fp1.factor[i];
			fmul->power[k]=fp1.power[i];
			++i;++k;
		}
		while(j<fp2.size){
			fmul->factor[k]=fp2.factor[j];
			fmul->power[k]=fp2.power[j];
			++j;++k;
		}
		fmul->size=k;
		factor_swap(fmul[0],fp1);
	}
	void pi_calc::factor_cancel(Integer &p,factor_t &fp,Integer &q,factor_t &fq){
		factor_resize(fmul[0],std::min(fp.size,fq.size));
		mp_uint i=0,j=0,k=0;
		while(i<fp.size&&j<fq.size){
			if(fp.factor[i]==fq.factor[j]){
				mp_uint mp=std::min(fp.power[i],fq.power[j]);
				fp.power[i]-=mp;
				fq.power[j]-=mp;
				fmul->factor[k]=fp.factor[i];
				fmul->power[k]=mp;
				++i;++j;++k;
			}
			else if(fp.factor[i]<fq.factor[j]){
				++i;
			}
			else{
				++j;
			}
		}
		fmul->size=k;

		if(k){
			bs_mulgcd(0,k,0);

			p/=gcds[0];
			q/=gcds[0];
			factor_compact(fp);
			factor_compact(fq);
		}
	}
	void pi_calc::factor_swap(factor_t &f1,factor_t &f2){
		mp_uint tmp1,tmp2,*ptmp1,*ptmp2;
		tmp1=f1.capacity;
		tmp2=f1.size;
		ptmp1=f1.factor;
		ptmp2=f1.power;
		f1.capacity=f2.capacity;
		f1.size=f2.size;
		f1.factor=f2.factor;
		f1.power=f2.power;
		f2.capacity=tmp1;
		f2.size=tmp2;
		f2.factor=ptmp1;
		f2.power=ptmp2;
	}

	void pi_calc::build_sieve(mp_uint n){
		mp_uint m=std::sqrt((mp_prec_t)(n+1));

		for(mp_uint i=3;i<=n;i+=2){
			mp_uint ih=i>>1;
			if(sieve[ih].factor==0){
				sieve[ih].pnex=1;
				if(i<=m){
					mp_uint jh=(mp_uint)i*i>>1,k=ih;
					while(jh*2+1<=n){
						if(sieve[jh].factor==0){
							sieve[jh].factor=i;
							mp_uint skf=sieve[k].factor;
							if(!skf)skf=2*k+1;
							if(skf==i){
								sieve[jh].pnex=sieve[k].pnex+1;
								sieve[jh].next=sieve[k].next;
							}
							else{
								sieve[jh].pnex=1+(k<<8);
								sieve[jh].next=k>>24;
							}
						}
						jh+=i;
						++k;
					}
				}
			}
		}
	}

	void pi_calc::bs_mulgcd(mp_uint a,mp_uint b,int top){
		Integer &gcd1=gcds[top],&gcd2=gcds[top+1];
		if(b-a<=32){
			gcd1=1;
			for(mp_uint i=a;i<b;++i){
				for(mp_uint j=0;j<fmul->power[i];++j){
					gcd1*=fmul->factor[i];
				}
			}
		}
		else{
			mp_uint mid=a+(b-a)/2;
			bs_mulgcd(a,mid,top);
			bs_mulgcd(mid,b,top+1);
			gcd1*=gcd2;
		}
	}

	void pi_calc::bs_chudnovsky(mp_uint a,mp_uint b,int needp,int level,int top){
		Integer &p1=ps[top],&p2=ps[top+1];
		Integer &q1=qs[top],&q2=qs[top+1];
		Integer &t1=ts[top],&t2=ts[top+1];
		factor_t &fp1=fps[top],&fp2=fps[top+1];
		factor_t &fq1=fqs[top],&fq2=fqs[top+1];
		if(b==a+1){
			p1=2*b-1;
			p1*=6*b-1;
			p1*=6*b-5;
			q1=b;
			q1*=b;
			q1*=b;
			q1*=C*C*C/24;
			t1=b;
			t1*=B;
			t1+=A;
			t1*=p1;
			if(b&1)t1.neg();
			factor_set_bp(fp1,2*b-1,1);
			factor_mul_bp(fp1,6*b-1,1);
			factor_mul_bp(fp1,6*b-5,1);

			while((b&1)==0)b>>=1;
			factor_set_bp(fq1,b,3);
			factor_mul_bp(fq1,C/64,3);
			fq1.power[0]-=1;

		}
		else{
			mp_uint mid=a+(b-a)/2;
			bs_chudnovsky(a,mid,1,level+1,top);
			bs_chudnovsky(mid,b,needp,level+1,top+1);
			if(level>=4)
				factor_cancel(p1,fp1,q2,fq2);
			q1*=q2;
			t1*=q2;
			t2*=p1;
			t1+=t2;

			factor_mul(fq1,fq2);

			if(needp){
				p1*=p2;
				factor_mul(fp1,fp2);
			}
		}
	}

	pi_calc::pi_calc(mp_uint digits,int base){
		mp_uint terms;
		mp_prec_t log2b=std::log2((mp_prec_t)base);
		mp_prec_t precision=digits*log2b;
		if(precision<MIN_PREC_BITS)precision=MIN_PREC_BITS;
		if(precision>MAX_PREC_BITS)precision=MAX_PREC_BITS;
		terms=precision/(3*std::log2((mp_prec_t)C/12));
		terms+=1;

		mp_uint smax=std::max(C/64,6*terms-1);
		mp_uint ssize=(smax+1)/2;
		sieve=new sieve_t[ssize];
		memset(sieve,0,ssize*sizeof(sieve_t));
		build_sieve(smax);

		ps=new Integer[MAX_DEPTH*4];
		qs=ps+MAX_DEPTH;
		ts=qs+MAX_DEPTH;
		gcds=ts+MAX_DEPTH;
		fps=new factor_t[MAX_DEPTH*2+2];
		fqs=fps+MAX_DEPTH;
		fmul=fqs+MAX_DEPTH;
		fbp=fmul+1;

		bs_chudnovsky(0,terms,0,0,0);

		delete[] sieve;
		delete[] fps;

		ts[0]+=qs[0]*A;
		Number Q,T;
		Q=std::move(qs[0]);
		T=std::move(ts[0]);

		delete[] ps;

		pi=C/12*C/12*C;
		pi.set_precision(precision);
		pi=sqrt(pi)*Q/T;
	}

	Number Pi(mp_uint digits,int base){
		if(base<0)base=-base;
		if(base<2||base>36)return Number();
		return pi_calc(digits,base).pi;
	}

	//other constants
	class prime_sieve{
	public:
		//composite test bitmap
		mp_byte_t *p;
		mp_uint nbytes;
		//list of prime numbers
		mp_uint *plist;
		mp_uint nprimes;
		//build sieve for [0,N]
		prime_sieve(mp_uint N);
		~prime_sieve();
		//return count of prime numbers <= x, x shall not exceed nbytes*16
		mp_uint prime_pi(mp_uint x);

	};

	prime_sieve::prime_sieve(mp_uint N){
		plist=nullptr;
		//esieve
		nbytes=(N+15)/16;
		N=nbytes*16;
		p=new mp_byte_t[nbytes];
		mp_uint imax=(N-1)/2;
		memset(p,0,nbytes);
		mp_uint simax=std::floor(std::sqrt(double(N)));
		simax=(simax+1)/2;
		for(mp_uint i=1;i<=simax;++i){
			//2*i+1;
			if(!(p[i/8]&1<<(i&7))){
				mp_uint j=i*(i+1)*2;
				while(j<=imax){
					p[j/8]|=1<<(j&7);
					j+=2*i+1;
				}
			}
		}
		//plist
		plist=new mp_uint[1+prime_pi(nbytes*16)];
		plist[0]=1;
		plist[1]=2;
		mp_uint result=1;
		for(mp_uint i=1;i<=imax;++i)if(!(p[i/8]&1<<(i&7)))
			plist[++result]=2*i+1;
		nprimes=result;
	}
	prime_sieve::~prime_sieve(){
		delete[] p;
		delete[] plist;
	}
	mp_uint prime_sieve::prime_pi(mp_uint x){
		if(x<=2)return x==2;
		mp_uint imax=(x-1)/2;
		mp_uint result=1;
		for(mp_uint i=1;i<=imax;++i)result+=!(p[i/8]&1<<(i&7));
		return result;
	}

	class pba_calc{
	public:
		Number pba;
		mp_uint *prime;
		Integer *qs,*ts;
		void bs_calc(mp_uint a,mp_uint b,int level,int top);
		pba_calc(mp_uint digits,int base);
	};

	void pba_calc::bs_calc(mp_uint a,mp_uint b,int level,int top){
		Integer &q1=qs[top],&q2=qs[top+1];
		Integer &t1=ts[top],&t2=ts[top+1];
		if(b==a+1){
			q1=prime[b-1];
			t1=prime[b]-1;
		}
		else{
			mp_uint mid=a+(b-a)/2;
			bs_calc(a,mid,level+1,top);
			bs_calc(mid,b,level+1,top+1);
			q1*=q2;
			t1*=q2;
			t1+=t2;
		}
	}
	pba_calc::pba_calc(mp_uint digits,int base){
		mp_uint terms;
		mp_prec_t log2b=std::log2((mp_prec_t)base);
		mp_prec_t precision=digits*log2b;
		if(precision<MIN_PREC_BITS)precision=MIN_PREC_BITS;
		if(precision>MAX_PREC_BITS)precision=MAX_PREC_BITS;

		mp_prec_t smax=precision*std::log((mp_prec_t)2);
		smax+=std::sqrt(smax)*4+4;
		{
			prime_sieve ps(std::ceil(smax));
			terms=ps.nprimes;
			prime=ps.plist;

			qs=new Integer[MAX_DEPTH*2];
			ts=qs+MAX_DEPTH;

			bs_calc(0,terms,0,0);
			prime=nullptr;
		}
		Number Q,T;
		Q=std::move(qs[0]);
		T=std::move(ts[0]);
		delete[] qs;
		T.set_precision(precision);
		pba=T/Q;
	}

	Number Prime_BuenosAires(mp_uint digits,int base){
		if(base<0)base=-base;
		if(base<2||base>36)return Number();
		return pba_calc(digits,base).pba;
	}

	class e_calc{
	public:
		Number e;
		Integer *qs,*ts;
		void bs_calc(mp_uint a,mp_uint b,int level,int top);
		e_calc(mp_uint digits,int base);
	};

	void e_calc::bs_calc(mp_uint a,mp_uint b,int level,int top){
		Integer &q1=qs[top],&q2=qs[top+1];
		Integer &t1=ts[top],&t2=ts[top+1];
		if(b==a+1){
			q1=b;
			t1=1;
		}
		else{
			mp_uint mid=a+(b-a)/2;
			bs_calc(a,mid,level+1,top);
			bs_calc(mid,b,level+1,top+1);
			q1*=q2;
			t1*=q2;
			t1+=t2;
		}
	}
	e_calc::e_calc(mp_uint digits,int base){
		mp_uint terms;
		mp_prec_t log2b=std::log2((mp_prec_t)base);
		mp_prec_t precision=digits*log2b;
		if(precision<MIN_PREC_BITS)precision=MIN_PREC_BITS;
		if(precision>MAX_PREC_BITS)precision=MAX_PREC_BITS;

		mp_prec_t eprec=precision*std::log((mp_prec_t)2);
		mp_prec_t z=eprec/std::exp((mp_prec_t)1),w=std::log(1+z),ew=1+z;
		mp_prec_t curerr=z-w*ew,olderr;
		//w=productlog(z)
		do{
			mp_prec_t curw=w+curerr/(ew*(1+w));
			ew=std::exp(curw);
			olderr=curerr;
			curerr=z-curw*ew;
			if(std::abs(curerr)>=std::abs(olderr))break;
			w=curw;
		} while(1);

		terms=eprec/w;
		terms+=1;
		qs=new Integer[MAX_DEPTH*2];
		ts=qs+MAX_DEPTH;

		bs_calc(0,terms,0,0);

		Number Q,T;
		Q=std::move(qs[0]);
		T=std::move(ts[0]);
		delete[] qs;
		T.set_precision(precision);
		e=T/Q;
		//e+=1
		e.data[e.dotp]+=1;
	}

	Number E(mp_uint digits,int base){
		if(base<0)base=-base;
		if(base<2||base>36)return Number();
		return e_calc(digits,base).e;
	}
}
