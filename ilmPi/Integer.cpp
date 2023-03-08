#include"Number.h"
#include<cstdlib>
#include<cstring>
#include<cmath>
#include<algorithm>

namespace ilmp{
	const mp_int EXTRA_ALLOC_RATIO=16;
	const mp_int EXTRA_ALLOC_LIMB=16;
	//compare(|num1|,|num2|), need(num1!=0, num2!=0)
	//no nan or zero allowed
	int _comparenzabs(const Integer&,const Integer&);
	void _addsub(Integer &x,const Integer&,const Integer&,bool isadd);
	
	mp_int extra_alloc(mp_int size){
		return size+size/EXTRA_ALLOC_RATIO+EXTRA_ALLOC_LIMB;
	}

	Integer::Integer(){
		data=nullptr;
		from_int(0);
	}
	Integer::Integer(bool x){
		data=nullptr;
		from_int((int)x);
	}
	Integer::Integer(char x){
		data=nullptr;
		from_int(x);
	}
	Integer::Integer(signed char x){
		data=nullptr;
		from_int(x);
	}
	Integer::Integer(unsigned char x){
		data=nullptr;
		from_uint(x);
	}
	Integer::Integer(signed short x){
		data=nullptr;
		from_int(x);
	}
	Integer::Integer(unsigned short x){
		data=nullptr;
		from_uint(x);
	}
	Integer::Integer(signed int x){
		data=nullptr;
		from_int(x);
	}
	Integer::Integer(unsigned int x){
		data=nullptr;
		from_uint(x);
	}
	Integer::Integer(signed long x){
		data=nullptr;
		from_int(x);
	}
	Integer::Integer(unsigned long x){
		data=nullptr;
		from_uint(x);
	}
	Integer::Integer(signed long long x){
		data=nullptr;
		from_int(x);
	}
	Integer::Integer(unsigned long long x){
		data=nullptr;
		from_uint(x);
	}
	Integer::Integer(Integer &&x){
		if(x.data==x.value)data=value;
		else data=x.data;
		ssize=x.ssize;
		value[0]=x.value[0];
		value[1]=x.value[1];
		x.data=nullptr;
		x.clear();
	}
	Integer::Integer(const Integer &x){
		data=nullptr;
		*this=x;
	}
	Integer::Integer(const Number &x){
		data=nullptr;
		if(x.is_inf()||x.is_nan()){
			clear();
			return;
		}
		
		if(x.ssize==0){
			from_int(0);
			return;
		}

		int signx=x.sign();
		mp_int sizex=x.size(),dotpx=x.dotp;
		mp_limb_t carry=0;
		if(dotpx>=1&&dotpx<=sizex)
			carry=x.data[dotpx-1]>>MP_LIMB_BITS-1;
		
		if(sizex<=dotpx){
			from_int((sizex==dotpx)*signx*carry);
			return;
		}

		mp_int maxsize=sizex-dotpx+carry;

		if(maxsize<=2)data=value;
		else{
			alloc=extra_alloc(maxsize);
			data=new mp_limb_t[alloc];
		}

		if(dotpx<=0){
			if(dotpx<0)memset(data,0,-dotpx*sizeof(mp_limb_t));
			memcpy(data-dotpx,x.data,sizex*sizeof(mp_limb_t));
			dotpx=0;
		}
		else{
			carry=ilmp_add_1_(data,x.data+dotpx,sizex-dotpx,carry);
			if(carry)data[maxsize-1]=1;
			else maxsize=sizex-dotpx;
		}
		ssize=maxsize*signx;
	}
	Integer::Integer(const char *str,int base){
		data=nullptr;
		from_str(str,base);
	}
	Integer::~Integer(){
		clear();
	}

	//this=NaN
	void Integer::clear(){
		if(data&&data!=value)delete[] data;
		data=nullptr;
		ssize=0;
		value[0]=0;
		value[1]=0;
	}
	void Integer::from_int(mp_int n){
		if(!data)data=value;
		if(n<0){
			data[0]=-n;
			ssize=-1;
		}
		else if(n>0){
			data[0]=n;
			ssize=1;
		}
	}
	void Integer::from_uint(mp_uint n){
		if(!data)data=value;
		data=value;
		if(n>0){
			data[0]=n;
			ssize=1;
		}
	}
	//[+-][N]
	void Integer::from_str(const char *pstr,int base){
		
		
	}

	//result may mod by 2^64
	mp_int Integer::to_int() const{
		return 0;
	}
	//result may mod by 2^64
	mp_uint Integer::to_uint() const{
		return 0;
	}
	//buffer length needed to store this as a string in a given base
	mp_int Integer::strlen(int base) const{
		return 1;
	}
	//convert this into a string in a given base
	void Integer::to_str(char *pstr,int base) const{
		pstr[0]=0;
	}

	void Integer::neg(){
		ssize=-ssize;
	}
	int Integer::sign() const{
		if(ssize>0)return 1;
		if(ssize<0)return -1;
		return 0;
	}
	mp_int Integer::size() const{
		return ssize<0?-ssize:ssize;
	}
	mp_int Integer::capacity() const{
		if(!data||data==value)return 2;
		return alloc;
	}
	mp_prec_t Integer::log2(mp_int shift) const{
		if(is_nan())return -NAN_PREC;
		if(ssize==0)return -INT_PREC;
		mp_int asize=size();
		mp_prec_t ms=0.;
		if(asize>=2)ms=data[asize-2]/MP_BASE;
		ms=(ms+data[asize-1])/MP_BASE;
		ms=std::log2(ms);
		if(std::abs(shift)<MAX_PREC_INT)
			shift+=MP_LIMB_BITS*asize;
		else
			ms+=MP_LIMB_BITS*asize;
		return ms+shift;
	}
	bool Integer::is_nan() const{
		return !data;
	}
	void Integer::swap(Integer &x){
		bool local=data==value;
		bool xlocal=x.data==x.value;
		mp_ptr _tm=data;
		mp_int _ts=ssize;
		mp_limb_t _tv[2]={value[0],value[1]};
		if(xlocal)data=value;
		else data=x.data;
		ssize=x.ssize;
		value[0]=x.value[0];
		value[1]=x.value[1];
		if(local)x.data=x.value;
		else x.data=_tm;
		x.ssize=_ts;
		x.value[0]=_tv[0];
		x.value[1]=_tv[1];
	}
	
	void _addsub(Integer &x,const Integer &Num1,const Integer &Num2,bool isadd){
		if(Num1.is_nan()||Num2.is_nan()){
			x.clear();
			return;
		}
		if(Num2.ssize==0){
			x=Num1;
			return;
		}
		if(Num1.ssize==0){
			x=Num2;
			if(!isadd)x.neg();
			return;
		}

		int sign1=Num1.sign(),sign2=Num2.sign(),signres=sign1;
		mp_int sizex,size1=Num1.size(),size2=Num2.size();
		mp_int maxsize,capax=x.capacity();
		mp_ptr pr=x.data;
		mp_int carry;
		bool realloc=false;
		if((sign1==sign2)==isadd){
			maxsize=1+std::max(size1,size2);
			if(maxsize>capax){
				realloc=true;
				capax=extra_alloc(maxsize);
				pr=new mp_limb_t[capax];
			}
			if(size1>=size2)
				carry=ilmp_add_(pr,Num1.data,size1,Num2.data,size2);
			else
				carry=ilmp_add_(pr,Num2.data,size2,Num1.data,size1);
			pr[--maxsize]=carry;
			sizex=maxsize+carry;
		}
		else{
			int cmp12=_comparenzabs(Num1,Num2);
			if(cmp12==0){
				x.from_int(0);
				return;
			}
			
			if(cmp12>0){
				ilmp_sub_(pr,Num1.data,size1,Num2.data,size2);
				sizex=size1;
			}
			else{
				maxsize=size2;
				if(maxsize>capax){
					realloc=true;
					capax=extra_alloc(maxsize);
					pr=new mp_limb_t[capax];
				}
				ilmp_sub_(pr,Num2.data,size2,Num1.data,size1);
				signres=-signres;
				sizex=size2;
			}
			while(pr[sizex-1]==0)--sizex;
		}

		if(realloc){
			if(x.data&&x.data!=x.value)delete[] x.data;
			x.data=pr;
			x.alloc=capax;
		}
		x.ssize=sizex*signres;
	}

	void add(Integer &x,const Integer &Num1,const Integer &Num2){
		_addsub(x,Num1,Num2,true);
	}
	void sub(Integer &x,const Integer &Num1,const Integer &Num2){
		_addsub(x,Num1,Num2,false);
	}
	void mul(Integer &x,const Integer &Num1,const Integer &Num2){
		if(Num1.is_nan()||Num2.is_nan()){
			x.clear();
			return;
		}
		if(Num1.ssize==0||Num2.ssize==0){
			x.from_int(0);
			return;
		}

		mp_int size1=Num1.size(),size2=Num2.size();
		int signres=Num1.sign()*Num2.sign();
		mp_int maxsize=size1+size2,capax=x.capacity();
		mp_int nz1=0,nz2=0,nzx;
		while(Num1.data[nz1]==0)++nz1;
		while(Num2.data[nz2]==0)++nz2;
		nzx=nz1+nz2;
		mp_ptr pr=x.data;

		bool realloc=false;
	
		if(maxsize>capax
			||size2>2&&(&x==&Num1)&&size1>nzx
			||size1>2&&(&x==&Num2)&&size2>nzx){
			realloc=true;
			capax=extra_alloc(maxsize);
			pr=new mp_limb_t[capax];
		}

		size1-=nz1;
		size2-=nz2;
		if(size1>=size2)
			ilmp_mul_(pr+nzx,Num1.data+nz1,size1,Num2.data+nz2,size2);
		else
			ilmp_mul_(pr+nzx,Num2.data+nz2,size2,Num1.data+nz1,size1);
		if(nzx)memset(pr,0,nzx*sizeof(mp_limb_t));

		if(realloc){
			if(x.data&&x.data!=x.value)delete[] x.data;
			x.data=pr;
			x.alloc=capax;
		}

		maxsize-=(x.data[maxsize-1]==0);
		x.ssize=maxsize*signres;
	}
	void div(Integer &x,const Integer &Num1,const Integer &Num2){
		if(Num1.is_nan()||Num2.ssize==0){
			x.clear();
			return;
		}
		if(Num1.ssize==0){
			x.from_int(0);
			return;
		}

		int cmpres=_comparenzabs(Num1,Num2);
		if(cmpres<=0){
			x.from_int(cmpres==0);
			return;
		}

		mp_int size1=Num1.size(),size2=Num2.size();
		int signres=Num1.sign()*Num2.sign();
		mp_int maxsize=size1-size2+1,capax=x.capacity();
		mp_int nz2=0;
		while(Num2.data[nz2]==0)++nz2;
		mp_ptr pr=x.data;
		bool realloc=false;

		if(maxsize>capax
			||size2>2&&(&x==&Num1||&x==&Num2)&&maxsize>nz2){
			realloc=true;
			capax=extra_alloc(maxsize);
			pr=new mp_limb_t[capax];
		}
		size1-=nz2;
		size2-=nz2;
		ilmp_div_(pr,nullptr,Num1.data+nz2,size1,Num2.data+nz2,size2);
		if(realloc){
			if(x.data&&x.data!=x.value)delete[] x.data;
			x.data=pr;
			x.alloc=capax;
		}

		maxsize-=(x.data[maxsize-1]==0);
		x.ssize=maxsize*signres;
	}

	Integer &Integer::operator =(Integer &&Num){
		if(this!=&Num){
			swap(Num);
			Num.clear();
		}
		return *this;
	}
	Integer &Integer::operator =(const Integer &Num){
		if(this!=&Num){
			if(!Num.data)clear();
			else{
				mp_int nsize=Num.size();
				bool replace=capacity()<nsize;
				if(replace){
					clear();
					alloc=extra_alloc(nsize);
					data=new mp_limb_t[alloc];
					memcpy(data,Num.data,nsize*sizeof(mp_limb_t));
				}
				else if(data){
					if(nsize)memcpy(data,Num.data,nsize*sizeof(mp_limb_t));
				}
				else{
					data=value;
					value[0]=Num.value[0];
					value[1]=Num.value[1];
				}
				ssize=Num.ssize;
			}
		}
		return *this;
	}
	Integer &Integer::operator+=(const Integer &Num){
		_addsub(*this,*this,Num,true);
		return *this;
	}
	Integer &Integer::operator-=(const Integer &Num){
		_addsub(*this,*this,Num,false);
		return *this;
	}
	Integer &Integer::operator*=(const Integer &Num){
		mul(*this,*this,Num);
		return *this;
	}
	Integer &Integer::operator/=(const Integer &Num){
		div(*this,*this,Num);
		return *this;
	}

	int _comparenzabs(const Integer &Num1,const Integer &Num2){
		mp_int na=Num1.size(),nb=Num2.size();
		if(na>nb)return 1;
		if(na<nb)return -1;
		return ilmp_cmp_(Num1.data,Num2.data,na);
	}
	int compare(const Integer &Num1,const Integer &Num2,bool abscomp){
		if(Num1.is_nan()||Num2.is_nan())return -2;
		int sign1=Num1.sign(),sign2=Num2.sign();
		if(abscomp){
			sign1=std::abs(sign1);
			sign2=std::abs(sign2);
		}
		if(sign1>sign2)return 1;
		if(sign1<sign2)return -1;
		if(sign1)return _comparenzabs(Num1,Num2)*sign1;
		return 0;
	}
	bool operator==(const Integer &Num1,const Integer &Num2){ return compare(Num1,Num2)==0; }
	bool operator!=(const Integer &Num1,const Integer &Num2){ return (compare(Num1,Num2)&1)==1; }
	bool operator>(const Integer &Num1,const Integer &Num2){ return compare(Num1,Num2)==1; }
	bool operator>=(const Integer &Num1,const Integer &Num2){ return compare(Num1,Num2)>=0; }
	bool operator<(const Integer &Num1,const Integer &Num2){ return compare(Num2,Num1)==1; }
	bool operator<=(const Integer &Num1,const Integer &Num2){ return compare(Num2,Num1)>=0; }

	Integer operator-(const Integer &Num){
		Integer result(Num);
		result.neg();
		return result;
	}
	Integer operator+(const Integer &Num){
		Integer result(Num);
		return result;
	}
	Integer operator+(const Integer &Num1,const Integer &Num2){
		Integer result;
		_addsub(result,Num1,Num2,true);
		return result;
	}
	Integer operator-(const Integer &Num1,const Integer &Num2){
		Integer result;
		_addsub(result,Num1,Num2,false);
		return result;
	}
	Integer operator*(const Integer &Num1,const Integer &Num2){
		Integer result;
		mul(result,Num1,Num2);
		return result;
	}
	Integer operator/(const Integer &Num1,const Integer &Num2){
		Integer result;
		div(result,Num1,Num2);
		return result;
	}

}
