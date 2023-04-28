#include"Number.h"
#include<cstdlib>
#include<cstring>
#include<cmath>
#include<algorithm>

//#define DEBUG_LOG
#include<vector>

namespace ilmp{
    mp_prec_t MIN_PREC_BITS=32;
    mp_prec_t MAX_PREC_BITS=MAX_LIMBS*MP_LIMB_BITS;
    mp_int MAX_EXP_LIMBS=MAX_LIMBS;
    mp_prec_t MAX_EXP_BITS=MAX_EXP_LIMBS*MP_LIMB_BITS;
    mp_int MIN_EXP_SCI_OUT=6;
    namespace utils{
    mp_int precision_limbs(mp_int iprec){
        //1 for missing bits in MSlimb, 1 for floor and extraprecision
        const mp_int SAFE_PREC_LIMBS=2;
        return iprec/MP_LIMB_BITS+SAFE_PREC_LIMBS;
    }
    mp_int reserve_limbs_lower(mp_int size){
        return size+EXTRA_ALLOC_LIMB;
    }
    mp_int reserve_limbs_upper(mp_int size){
        return size+size/EXTRA_ALLOC_RATIO+EXTRA_ALLOC_LIMB;
    }

    mp_prec_t log2add(mp_prec_t x,mp_prec_t y){
        if(x==y)return x+1;
        mp_prec_t mxy,z;
        if(x>y){
            mxy=x;
            z=y-x;
        }
        else{
            mxy=y;
            z=x-y;
        }
        return mxy+std::log2(1+std::exp2(z));
    }
    int count_left_zeros(mp_limb_t x){
        int n=MP_LIMB_BITS,c=n/2;
        do{
            mp_limb_t y=x>>c;
            if(y){
                x=y;
                n-=c;
            }
        } while(c>>=1);
        return n-(int)x;
    }
    };
    using namespace ilmp::utils;

#define INIT_NUMBER       \
do{                       \
    data=value;           \
    ssize=x?(x>0?1:-1):0; \
    dotp=0;               \
    prec=INT_PREC;        \
    value[0]=x*ssize;     \
}while(0)

    Number::Number()                    { data=nullptr; }
    Number::Number(bool x)              { INIT_NUMBER;  }
    Number::Number(char x)              { INIT_NUMBER;  }
    Number::Number(signed char x)       { INIT_NUMBER;  }
    Number::Number(unsigned char x)     { INIT_NUMBER;  }
    Number::Number(signed short x)      { INIT_NUMBER;  }
    Number::Number(unsigned short x)    { INIT_NUMBER;  }
    Number::Number(signed int x)        { INIT_NUMBER;  }
    Number::Number(unsigned int x)      { INIT_NUMBER;  }
    Number::Number(signed long x)       { INIT_NUMBER;  }
    Number::Number(unsigned long x)     { INIT_NUMBER;  }
    Number::Number(signed long long x)  { INIT_NUMBER;  }
    Number::Number(unsigned long long x){ INIT_NUMBER;  }
    void Number::from_int(mp_int x){
        clear();
        INIT_NUMBER;
    }
    void Number::from_uint(mp_uint x){
        clear();
        INIT_NUMBER;
    }
#undef INIT_NUMBER
    
    Number::Number( float x){ data=nullptr; from_float(&x,sizeof(x)); }
    Number::Number(double x){ data=nullptr; from_float(&x,sizeof(x)); }

    Number::Number(const char *str,int base){
        data=nullptr;
        from_str(str,base);
    }
    Number::~Number(){ clear(); }
    void Number::clear(){
        if(data!=value&&data)delete[] data;
        data=nullptr;
    }

    Number::operator bool              () const{return !data||ssize;}
    Number::operator char              () const{return to_int();}
    Number::operator signed char       () const{return to_int();}
    Number::operator unsigned char     () const{return to_uint();}
    Number::operator signed short      () const{return to_int();}
    Number::operator unsigned short    () const{return to_uint();}
    Number::operator signed int        () const{return to_int();}
    Number::operator unsigned int      () const{return to_uint();}
    Number::operator signed long       () const{return to_int();}
    Number::operator unsigned long     () const{return to_uint();}
    Number::operator signed long long  () const{return to_int();}
    Number::operator unsigned long long() const{return to_uint();}
    Number::operator float             () const{
        float x;
        to_float(&x,sizeof(x));
        return x; 
    }
    Number::operator double            () const{
        double x;
        to_float(&x,sizeof(x));
        return x;
    }

    void Number::neg(){ ssize=-ssize; }
    int Number::sign() const{ return ssize?(ssize>0?1:-1):0; }
    mp_int Number::size() const{ return ssize<0?-ssize:ssize; }
    mp_int Number::capacity() const{ return data?(data==value?2:alloc_size):0; }
    mp_prec_t Number::precision() const{ return prec; }

    int Number::is_inf() const{ return data?0:sign(); }
    bool Number::is_nan() const{ return !data&&ssize==0; }
    bool Number::is_int() const{ return prec==INT_PREC; }
    bool Number::is_num() const{ return data; }

    void Number::swap(Number &x){
        const size_t n=sizeof(Number);
        mp_byte_t buffer[n];
        memcpy(buffer,this,n);
        memcpy(this,&x,n);
        memcpy(&x,buffer,n);
        if(  data==x.value)  data=  value;
        if(x.data==value)  x.data=x.value;
    }

#define TAKE_ASSIGN                 \
do{                                 \
    memcpy(this,&x,sizeof(Number)); \
    if(data==x.value)data=value;    \
    x.data=nullptr;                 \
}while(0)
#define COPY_ASSIGN                                  \
do{                                                  \
    memcpy(this,&x,sizeof(Number));                  \
    if(data==x.value)data=value;                     \
    else if(data){                                   \
        mp_int asize=size();                         \
        alloc_size=reserve_limbs_lower(asize);       \
        data=new mp_limb_t[alloc_size];              \
        memcpy(data,x.data,asize*sizeof(mp_limb_t)); \
    }                                                \
}while(0)

    Number::Number(Number &&x){ TAKE_ASSIGN; }
    Number &Number::operator =(Number &&x){
        if(this!=&x){
            clear();
            TAKE_ASSIGN;
        }
        return *this;
    }
    Number::Number(const Number &x){ COPY_ASSIGN; }
    Number &Number::operator =(const Number &x){
        if(this!=&x){
            clear();
            COPY_ASSIGN;
        }
        return *this;
    }
#undef TAKE_ASSIGN
#undef COPY_ASSIGN

    Number::Number(Integer &&x){
        data=x.data;
        ssize=x.ssize;
        dotp=0;
        prec=INT_PREC;
        value[0]=x.value[0];
        value[1]=x.value[1];
        if(data==x.value)data=value;
        x.data=x.value;
        x.ssize=0;
    }
    Number::Number(const Integer &x){
        data=x.data;
        ssize=x.ssize;
        dotp=0;
        prec=INT_PREC;
        value[0]=x.value[0];
        value[1]=x.value[1];
        if(data==x.value)data=value;
        else if(data){
            mp_int asize=size();
            alloc_size=reserve_limbs_lower(asize);
            data=new mp_limb_t[alloc_size];
            memcpy(data,x.data,asize*sizeof(mp_limb_t));
        }
    }

    mp_int Number::to_int() const{
        if(!data||!ssize||dotp<0)return 0;
        mp_int asize=size();
        mp_limb_t hlimb=0,llimb=0;
        if(dotp<asize)hlimb=data[dotp];
        if(dotp>0&&dotp<=asize)llimb=data[dotp-1];
        mp_int ilimb=hlimb+(llimb>>MP_LIMB_BITS-1);
        return ssize>0?ilimb:-ilimb;
    }
    mp_uint Number::to_uint() const{
        if(!data||!ssize||dotp<0)return 0;
        mp_int asize=size();
        mp_limb_t hlimb=0,llimb=0;
        if(dotp<asize)hlimb=data[dotp];
        if(dotp>0&&dotp<=asize)llimb=data[dotp-1];
        mp_uint ilimb=hlimb+(llimb>>MP_LIMB_BITS-1);
        return ssize>0?ilimb:-ilimb;
    }

    mp_int Number::loglimb() const{
        if(!data)return MP_MAX_INT;
        if(!ssize){
            if(is_int())return MP_MIN_INT;
            else return std::ceil(prec*(mp_prec_t(-1)/MP_LIMB_BITS));
        }
        return size()-dotp;
    }
    mp_prec_t Number::logbit(mp_int shift) const{
        if(!data)return ssize?INFINITY:NAN;
        if(!ssize)return shift-prec;
        mp_int asize=size(),alog2=MP_LIMB_BITS*(asize-dotp);
        mp_prec_t ms=0;
        if(asize>=2)ms=data[asize-2]*(1/MP_BASE);
        ms=(ms+data[asize-1])*(1/MP_BASE);
        ms=std::log2(ms);
        //if alog2 and shift have different signs
        if((alog2^shift)<0)shift+=alog2;
        else ms+=alog2;
        return ms+shift;
    }
    void Number::set_precision(mp_prec_t new_prec){
        if(!data){
            if(ssize&&new_prec<=0)ssize=0;
        }
        else{
            if(!ssize)prec+=new_prec;
            else prec=new_prec;
            if(std::isnan(new_prec)){
                clear();
                ssize=0;
            }
            else normalize();
        }
    }
    void Number::normalize(){
        if(!data)return;
        if(!ssize){
            if(data!=value){
                delete[] data;
                data=value;
            }
            if(prec>=MAX_EXP_BITS)prec=INT_PREC;
            if(prec<-MAX_EXP_BITS)data=nullptr;
            return;
        }

        if(!is_int()){
            if(prec<=0){
                mp_prec_t new_prec=prec-logbit();
                clear();
                data=value;
                ssize=0;
                prec=new_prec;
                if(prec>=MAX_EXP_BITS)prec=INT_PREC;
                if(prec<-MAX_EXP_BITS)data=nullptr;
                return;
            }
            if(prec>MAX_PREC_BITS)prec=MAX_PREC_BITS;
        }

        mp_int asize=size(),nsize=asize-dotp;
        if(nsize>MAX_EXP_LIMBS){
            int old_sign=sign();
            clear();
            ssize=old_sign;
            return;
        }
        if(nsize<=-MAX_EXP_LIMBS){
            clear();
            data=value;
            ssize=0;
            prec=INT_PREC;
            return;
        }

        if(is_int()){
            if(nsize<=0){
                clear();
                data=value;
                ssize=0;
                prec=INT_PREC;
                return;
            }
        }
        else{
            nsize=precision_limbs(prec);
        }
        if(nsize>asize)nsize=asize;
        while(data[asize-nsize]==0)--nsize;
        
        mp_int cutoff=asize-nsize;
        dotp-=cutoff;
        ssize=ssize<0?-nsize:nsize;
        //realloc
        mp_int capa=capacity();
        if(nsize>capa||reserve_limbs_upper(nsize)<capa){
            mp_int newcapa=reserve_limbs_lower(nsize);
            mp_ptr newdata=new mp_limb_t[newcapa];

            memcpy(newdata,data+cutoff,nsize*sizeof(mp_limb_t));
            clear();
            data=newdata;
            alloc_size=newcapa;
        }
        else if(cutoff){
            memmove(data,data+cutoff,nsize*sizeof(mp_limb_t));
        }
    }
    int compare(const Number &Num1,const Number &Num2,bool abscomp){
        if(!Num1.data||!Num2.data){
            if(Num1.is_nan()||Num2.is_nan())return -2;
            int inf1,inf2;
            if(abscomp){
                inf1=!Num1.data;
                inf2=!Num2.data;
            }
            else{
                inf1=Num1.is_inf();
                inf2=Num2.is_inf();
            }
            if(inf1>inf2)return 1;
            if(inf1<inf2)return -1;
            return 0;
        }
        int sign1,sign2;
        if(abscomp){
            sign1=Num1.ssize!=0;
            sign2=Num2.ssize!=0;
        }
        else{
            sign1=Num1.sign();
            sign2=Num2.sign();
        }
        if(sign1>sign2)return 1;
        if(sign1<sign2)return -1;
        if(sign1){
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
    bool operator> (const Number &Num1,const Number &Num2){ return compare(Num1,Num2)==1; }
    bool operator>=(const Number &Num1,const Number &Num2){ return compare(Num1,Num2)>=0; }
    bool operator< (const Number &Num1,const Number &Num2){ return compare(Num2,Num1)==1; }
    bool operator<=(const Number &Num1,const Number &Num2){ return compare(Num2,Num1)>=0; }

#ifdef DEBUG_LOG
#define DEBUG_PRINT debug_print
    void debug_print(const char *src,const std::vector<const Number*> &pNums){
        static bool print_flag=true;
        if(print_flag){
            print_flag=false;
            static mp_int i=0;
            static const char *dst="F:\\Temp\\debug_new\\%lld.%s";
            static char fbuf[256];
            sprintf(fbuf,dst,i,src);
            FILE *fout=fopen(fbuf,"w");
            fprintf(fout,"%s\n",src);
            for(const Number *p:pNums){
                char *cp=new char[p->strlen()+1];
                p->to_str(cp);
                fprintf(fout,"%s\n",cp);
                delete[] cp;
            }
            fclose(fout);
            ++i;
            print_flag=true;
            if(i==542){
                printf("");
            }
        }
    }
#else
#define DEBUG_PRINT(x,y) if(0)
#endif

    Number operator-(const Number &Num){
        Number result(Num);
        result.neg();
        return result;
    }
    Number operator+(const Number &Num){
        Number result(Num);
        return result;
    }
    
    Number _mul(const Number &Num1,const Number &Num2){
        Number result;
        if(!Num1.data||!Num2.data){
            result.ssize=Num1.sign()*Num2.sign();
            return result;
        }
        if(!Num1.ssize||!Num2.ssize){
            result.data=result.value;
            result.ssize=0;
            result.prec=-(Num1.logbit()+Num2.logbit());
            return result;
        }

        int sign1=Num1.sign(),sign2=Num2.sign();
        mp_int size1=Num1.size(),size2=Num2.size();
        mp_srcptr pa=Num1.data+size1;
        mp_srcptr pb=Num2.data+size2;

        mp_prec_t pmin=std::min(Num1.prec,Num2.prec);

        mp_int na=size1;
        mp_int nb=size2;
        if(pmin==INT_PREC)result.prec=INT_PREC;
        else{
            mp_int pn=precision_limbs(pmin);
            na=std::min(na,pn);
            nb=std::min(nb,pn);
            result.prec=-log2add(-Num1.prec,-Num2.prec);
        }

        while(pa[-na]==0)--na;
        while(pb[-nb]==0)--nb;

        mp_int rn=na+nb;
        mp_ptr pr;
        if(rn<=2)pr=result.value;
        else{
            result.alloc_size=reserve_limbs_lower(rn);
            pr=new mp_limb_t[result.alloc_size];
        }

        if(na>=nb)ilmp_mul_(pr,pa-na,na,pb-nb,nb);
        else ilmp_mul_(pr,pb-nb,nb,pa-na,na);
        while(pr[rn-1]==0)--rn;

        result.data=pr;
        result.ssize=sign1==sign2?rn:-rn;
        result.dotp=(na-size1+Num1.dotp)+(nb-size2+Num2.dotp);
        return result;
    }
    Number operator*(const Number &Num1,const Number &Num2){
        Number result;
        result=_mul(Num1,Num2);
        result.normalize();
        DEBUG_PRINT("mul",{&Num1,&Num2,&result});
        return result;
    }
    Number &Number::operator*=(const Number &Num){
        *this=*this*Num;
        return *this;
    }
    
    Number _div(const Number &Num1,const Number &Num2){
        Number result;
        if(!Num1.data||!Num2.data||!Num2.ssize){
            if(!Num2.ssize||Num1.is_nan()||!Num1.data&&!Num2.data)//nan
                result.ssize=0;
            else if(!Num1.data)//inf
                result.ssize=Num1.sign()*Num2.sign();
            else{//0
                result.data=result.value;
                result.ssize=0;
                result.prec=INT_PREC;
            }
            return result;
        }
        if(!Num1.ssize){//0.
            result.data=result.value;
            result.ssize=0;
            result.prec=Num1.prec+Num2.logbit();
            return result;
        }

        int sign1=Num1.sign(),sign2=Num2.sign();
        mp_int size1=Num1.size(),size2=Num2.size();
        mp_srcptr pa=Num1.data+size1;
        mp_srcptr pb=Num2.data+size2;
        
        mp_prec_t pmin=std::min(Num1.prec,Num2.prec);
        mp_int na;
        mp_int nb=size2;

        int signres=sign1==sign2?1:-1;
        if(pmin==INT_PREC){//int div
            result.prec=INT_PREC;
            int cmp12=compare(Num1,Num2,true);
            if(cmp12<=0){
                result.data=result.value;
                result.ssize=signres&~cmp12;
                result.dotp=0;
                result.value[0]=1;
                return result;
            }

            while(pb[-nb]==0)--nb;
            na=size1-Num1.dotp+(nb-size2+Num2.dotp);
        }
        else{//float div
            mp_int pn=precision_limbs(pmin);
            nb=std::min(nb,pn);
            while(pb[-nb]==0)--nb;
            na=nb+pn;
            result.prec=-log2add(-Num1.prec,-Num2.prec);
        }

        mp_ptr tmp=nullptr;
        if(na>size1){
            tmp=new mp_limb_t[na];
            memset(tmp,0,(na-size1)*sizeof(mp_limb_t));
            memcpy(tmp+na-size1,Num1.data,size1*sizeof(mp_limb_t));
            pa=tmp+na;
        }

        mp_int rn=na-nb+1;
        mp_ptr pr;
        if(rn<=2)pr=result.value;
        else{
            result.alloc_size=reserve_limbs_lower(rn);
            pr=new mp_limb_t[result.alloc_size];
        }
        ilmp_div_(pr,nullptr,pa-na,na,pb-nb,nb);

        while(pr[rn-1]==0)--rn;
        if(tmp)delete[] tmp;

        result.data=pr;
        result.ssize=signres*rn;
        result.dotp=(na-size1+Num1.dotp)-(nb-size2+Num2.dotp);
        return result;
    }
    Number operator/(const Number &Num1,const Number &Num2){
        Number result;
        result=_div(Num1,Num2);
        result.normalize();
        DEBUG_PRINT("div",{&Num1,&Num2,&result});
        return result;
    }
    Number &Number::operator/=(const Number &Num){
        *this=*this/Num;
        return *this;
    }

    Number _pow(const Number &x,mp_int n){
        Number result(1);
        if(n==0)return result;
        if(!x.data){//inf, nan
            result.ssize=0;
            if(!x.ssize)result.data=nullptr;
            else if(n>0){
                result.data=nullptr;
                result.ssize=x.ssize<0&&(n&1)?-1:1;
            }
            return result;
        }
        if(!x.ssize){//0
            result.ssize=0;
            if(n>0)result.prec=x.prec*n;
            else result.data=nullptr;
            return result;
        }
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
        if(x.data&&x.ssize){//non-zero number
            mp_prec_t nlog2x=n*x.logbit();
            if(nlog2x>MAX_EXP_BITS){
                result.ssize=x.ssize<0&&(n&1)?-1:1;
                return result;
            }
            if(nlog2x<-MAX_EXP_BITS){
                result.data=result.value;
                result.ssize=0;
                result.prec=INT_PREC;
                return result;
            }
        }
        result=_pow(x,n);
        result.normalize();
        return result;
    }

    void Number::from_str(const char *pstr,int base){
        clear();
        ssize=0;
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

        mp_prec_t fracpval=0,fracpval_base=1;
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
            bool hasfracpval=false;
            bool pehval=false;
            mp_int pestart=-1;
            mp_int ip=MP_LIMB_BITS;
            do{
                char c=pstr[i];
                if(c=='.'&&idx==0&&!hasfracpval){
                    c=pstr[++i];
                    hasfracpval=true;
                }
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
                if(hasfracpval){
                    fracpval_base/=base;
                    fracpval+=fracpval_base*val;
                }
                else if(pestart!=-1){
                    covbuf[--ip]=val;
                    if(ip==0)return;
                }
                ++i;
            } while(1);
            if(!pehval)return;
            mp_int peres=0;
            if(ip!=MP_LIMB_BITS){
                if(ilmp_limbs_(covbuf+ip,MP_LIMB_BITS-ip,base)>2)
                    return;
                if(ilmp_from_str_(covval,covbuf+ip,MP_LIMB_BITS-ip,base)==2)
                    return;
                if(covval[0]>MAX_PREC_INT)return;
                peres=covval[0];
            }
            if(pesign)peres=-peres;
            if(idx==0){
                pval=peres;
                if(pesign)fracpval=-fracpval;
            }
            if(idx==1)eval=peres;
        }

        if(pstr[i])return;

        bool isint=!haspval&&mdot==-1&&!haseval;
        mp_int offset=mdot!=-1?mend-1-mdot:0;
        const mp_prec_t log2base=std::log2((mp_prec_t)base);

        if(mstart==-1){//0
            data=value;
            if(isint)prec=INT_PREC;
            else{
                prec=(((haspval?pval:0)+offset-eval)+fracpval)*log2base;
                if(prec>=MAX_EXP_BITS)prec=INT_PREC;
                if(prec<-MAX_EXP_BITS)data=nullptr;
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
        if(haspval)mprec=(pval+fracpval)*log2base;
        else if(mprec<MIN_PREC_BITS)mprec=MIN_PREC_BITS;
        if(mprec>MAX_PREC_BITS)mprec=MAX_PREC_BITS;
        bool mpv=mprec<=0;
        if(mpv)apprlog2-=mprec;
        if(apprlog2>MAX_EXP_BITS){
            if(!mpv)ssize=sign?-1:1;
            return;
        }
        if(apprlog2<-MAX_EXP_BITS){
            data=value;
            prec=INT_PREC;
            return;
        }
        if(mpv){
            data=value;
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
            data=value;
            memcpy(data,covval,sizeof(mp_limb_t)*2);
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
            if(lcovlen<=2)data=value;
            else{
                alloc_size=reserve_limbs_lower(lcovlen);
                data=new mp_limb_t[alloc_size];
            }
            ssize=ilmp_from_str_(data,lcovbuf,digits,base);
            delete[] lcovbuf;
        }
        if(sign)ssize=-ssize;
        dotp=0;

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
    mp_int Number::strlen(int base) const{
        if(base<0)base=-base;
        if(base<2||base>36)return 1;
        if(!is_num())return 6;
        const mp_prec_t log2base=std::log2((mp_prec_t)base);

        mp_int lm,ls=ssize<0,le;
        if(is_int()){
            le=0;
            if(ssize==0)lm=1;
            else lm=(mp_int)(logbit()/log2base)+2;
        }
        else{
            mp_prec_t ev;
            if(ssize==0){
                ev=prec;
                lm=1;
            }
            else{
                ev=logbit();
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
        if(!is_num()){
            if(ssize<0){
                *pstr='-';
                ++pstr;
            }
            strcpy(pstr,ssize==0?"#nan":"#inf");
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
                eval=std::ceil((prec-logbit())/log2base-mp_prec_t(1)/8);
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

    //|num1|+-|num2|, need(num1!=0, num2!=0, isadd|||num1|>=|num2|)
    //no nan or inf or zero allowed
    //result can be Num1 or Num2
    void _addsubnzabs(Number &result,const Number &Num1,const Number &Num2,bool is_add){
        mp_int size1=Num1.size(),size2=Num2.size();
        mp_int sstarta=size1-Num1.dotp;
        mp_int sstartb=size2-Num2.dotp;
        mp_int rstart=std::max(sstarta,sstartb);
        mp_int senda=Num1.is_int()?MP_MIN_INT:sstarta-precision_limbs(Num1.prec);
        mp_int sendb=Num2.is_int()?MP_MIN_INT:sstartb-precision_limbs(Num2.prec);
        mp_int hend=-std::max(Num1.dotp,Num2.dotp);
        mp_int rend=std::max(std::max(senda,sendb),hend);
        mp_int rn=rstart-rend+1;

        mp_int sha=Num1.dotp+rend,shb=Num2.dotp+rend;
        mp_srcptr pa=Num1.data,pb=Num2.data;
        mp_ptr pr=result.data;
        mp_int newsize=0;
        if(result.capacity()<rn
        ||&result==&Num1&&sha<0
        ||&result==&Num2&&shb<0){
            newsize=reserve_limbs_lower(rn);
            pr=new mp_limb_t[newsize];
        }

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

        if(result.data!=pr){
            result.clear();
            result.data=pr;
            result.alloc_size=newsize;
        }

        if(!rn){
            result.clear();
            result.data=result.value;
        }
        else{
            result.ssize=rn;
            result.dotp=-rend;
        }
    }
    //will affect data, ssize, dotp of result
    //result can be Num1 or Num2
    inline void _addsubnz(Number &result,const Number &Num1,const Number &Num2,bool is_add){
        int sign1=Num1.sign(),sign2=Num2.sign(),signres=sign1;

        if(is_add==(sign1==sign2))
            _addsubnzabs(result,Num1,Num2,true);
        else{
            int cmp12=compare(Num1,Num2,true);
            signres*=cmp12;
            if(cmp12){
                if(cmp12>0)
                    _addsubnzabs(result,Num1,Num2,false);
                else
                    _addsubnzabs(result,Num2,Num1,false);
            }
            else{
                result.clear();
                result.data=result.value;
            }
        }

        result.ssize*=signres;
    }
    //result can be Num1 or Num2
    void _addsub(Number &result,const Number &Num1,const Number &Num2,bool is_add){
        if(!Num1.data||!Num2.data){
            int inf1=Num1.is_inf(),inf2=Num2.is_inf();
            result.clear();
            result.ssize=0;
            if(inf1||inf2){
                if(!inf2)result.ssize=inf1;
                else if(!inf1)result.ssize=is_add?inf2:-inf2;
                else if((inf1==inf2)==is_add)result.ssize=inf1;
            }
            return;
        }
        if(Num1.is_int()&&Num2.is_int()){
            if(!Num1.ssize||!Num2.ssize){
                if(!Num2.ssize)result=Num1;
                else {
                    result=Num2;
                    if(!is_add)result.neg();
                }
                return;
            }
            _addsubnz(result,Num1,Num2,is_add);
            result.prec=INT_PREC;
            return;
        }
        if(!Num1.ssize&&!Num2.ssize){
            result.clear();
            result.data=result.value;
            result.ssize=0;
            result.prec=-log2add(-Num1.prec,-Num2.prec);
            return;
        }

        mp_int absbase=std::max(Num1.loglimb(),Num2.loglimb())*MP_LIMB_BITS;
        mp_prec_t abs1=Num1.logbit(-absbase);
        mp_prec_t abs2=Num2.logbit(-absbase);

        mp_prec_t sigma1=abs1,sigma2=abs2,sigma;
        if(Num1.ssize)sigma1-=Num1.prec;
        if(Num2.ssize)sigma2-=Num2.prec;
        sigma=log2add(sigma1,sigma2);

        if(!Num1.ssize){
            result=Num2;
            if(!is_add)result.neg();
            result.prec=abs2-sigma;
            return;
        }
        if(!Num2.ssize){
            result=Num1;
            result.prec=abs1-sigma;
            return;
        }

        _addsubnz(result,Num1,Num2,is_add);

        if(result.ssize==0)
            result.prec=-absbase-sigma;
        else
            result.prec=result.logbit(-absbase)-sigma;
    }

    Number operator+(const Number &Num1,const Number &Num2){
        Number result;
        _addsub(result,Num1,Num2,true);
        result.normalize();
        DEBUG_PRINT("add",{&Num1,&Num2,&result});
        return result;
    }
    Number operator-(const Number &Num1,const Number &Num2){
        Number result;
        _addsub(result,Num1,Num2,false);
        result.normalize();
        DEBUG_PRINT("sub",{&Num1,&Num2,&result});
        return result;
    }
    Number &Number::operator+=(const Number &Num){
#ifdef DEBUG_LOG
        Number temp=*this;
#endif
        _addsub(*this,*this,Num,true);
        normalize();
        DEBUG_PRINT("addto",{&temp,&Num,this});
        return *this;
    }
    Number &Number::operator-=(const Number &Num){
#ifdef DEBUG_LOG
        Number temp=*this;
#endif
        _addsub(*this,*this,Num,false);
        normalize();
        DEBUG_PRINT("subfrom",{&temp,&Num,this});
        return *this;
    }

    Number _sqrt(const Number &x){
        Number result;
        if(!x.data){
            result.ssize=x.ssize>0;
            return result;
        }
        if(!x.ssize){
            result.data=result.value;
            result.ssize=0;
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
            mp_int pn=precision_limbs(x.prec);
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
        DEBUG_PRINT("sqrt",std::vector<const Number*>{&x,&result});
        return result;
    }

    void Number::from_float(const void *fptr,mp_int type_bytes,mp_int exp_bits,mp_int hidden_bit){
        clear();
        ssize=0;
        if(type_bytes<=0||hidden_bit>1||exp_bits>53)return;
        if(exp_bits<0){
            const char default_bits[]={
                 0, 5, 8, 0,11,15, 0, 0,
                15, 0, 0, 0, 0, 0, 0, 0, 19
            };
            if(!(type_bytes&1)&&type_bytes<=32)
                exp_bits=default_bits[type_bytes/2];
            if(exp_bits<=0)return;
        }
        if(hidden_bit<0)
            //MSbit=1 is hidden except for extended precision format(10 bytes)
            hidden_bit=(type_bytes!=10);
        //mantissa bits
        mp_int mantissa=type_bytes*8-1-exp_bits+hidden_bit;
        if(mantissa<1)return;

        //get exp
        const mp_byte_t *pb=(const mp_byte_t *)fptr;
        mp_int explimb=0,signbit,expbias=(mp_limb_t(1)<<exp_bits-1)-1;
        int eloadbits=exp_bits+1;
        mp_int eloadpos=type_bytes;
        do{
            explimb<<=8;
            explimb|=pb[--eloadpos];
            eloadbits-=8;
        } while(eloadbits>=hidden_bit);
        mp_limb_t mlimb=explimb&(mp_limb_t(1)<<-eloadbits)-1;
        explimb>>=-eloadbits;
        signbit=explimb>>exp_bits;
        explimb-=signbit<<exp_bits;
        if(explimb==0||explimb==expbias*2+1){
            expbias=1-expbias-mantissa+1;

            if(explimb&&hidden_bit==0){
                //clear integer bit
                mlimb&=~(mp_limb_t(1)<<-eloadbits-1);
            }

            while(!mlimb&&eloadpos){
                mlimb<<=8;
                mlimb|=pb[--eloadpos];
            }

            if(explimb){
                //inf, nan
                if(!mlimb)ssize=signbit?-1:1;
                return;
            }
        }
        else{
            expbias=explimb-expbias-mantissa+1;
            //normal
            mlimb|=hidden_bit<<-eloadbits;

            //when hidden_bit=0 but the integer part is also 0,
            //this could happen in unnormalized extended precision format.
            while(!mlimb&&eloadpos){
                mlimb<<=8;
                mlimb|=pb[--eloadpos];
            }
        }

        if(!mlimb){//0.
            from_uint(0);
            prec=1-expbias;
            return;
        }
        prec=1-expbias;
        mp_int mbits=MP_LIMB_BITS-count_left_zeros(mlimb);
        dotp=-(expbias>>MP_LOG2_LIMB_BITS);
        mp_int rbits=expbias+dotp*MP_LIMB_BITS+eloadpos*8;
        mp_int msize=(mbits+rbits+(MP_LIMB_BITS-1))/MP_LIMB_BITS;
        if(msize<=2)data=value;
        else{
            alloc_size=reserve_limbs_lower(msize);
            data=new mp_limb_t[alloc_size];
        }
        ssize=signbit?-msize:msize;
        
        mp_limb_t hlimb=0;
        mp_int reqbits=mbits+rbits-(msize-1)*MP_LIMB_BITS;
        do{
            if(mbits>=reqbits){
                mbits-=reqbits;
                mp_limb_t wlimb=mlimb>>mbits;
                if(mbits)wlimb|=hlimb<<MP_LIMB_BITS-mbits;
                data[--msize]=wlimb;
                if(!msize)break;
                reqbits=MP_LIMB_BITS;
            }
            mp_limb_t loadbyte=0;
            if(eloadpos)loadbyte=pb[--eloadpos];
            mbits+=8;
            hlimb=hlimb<<8|mlimb>>MP_LIMB_BITS-8;
            mlimb=mlimb<<8|loadbyte;
        } while(1);
        prec+=logbit();
        return;
    }
    void Number::to_float(void *fptr,mp_int type_bytes,mp_int exp_bits,mp_int hidden_bit) const{
        if(type_bytes<=0||hidden_bit>1||exp_bits>53)return;
        if(exp_bits<0){
            const char default_bits[]={
                 0, 5, 8, 0,11,15, 0, 0,
                15, 0, 0, 0, 0, 0, 0, 0, 19
            };
            if(!(type_bytes&1)&&type_bytes<=32)
                exp_bits=default_bits[type_bytes/2];
            if(exp_bits<=0)return;
        }
        if(hidden_bit<0)
            //MSbit=1 is hidden except for extended precision format(10 bytes)
            hidden_bit=(type_bytes!=10);
        //mantissa bits
        mp_int mantissa=type_bytes*8-1-exp_bits+hidden_bit;
        if(mantissa<1)return;

        mp_int expbias=(mp_limb_t(1)<<exp_bits-1)-1;
        mp_int msize=0,explimb=0,signbit=0;
        if(!data){
            signbit=ssize<=0;
            explimb=expbias*2+1;
        }
        else if(ssize){
            signbit=ssize<0;
            msize=size();
            explimb=(msize-dotp)*MP_LIMB_BITS-count_left_zeros(data[msize-1]);
            explimb=explimb+expbias-1;
            if(explimb<=0){//subnormal
                explimb=0;
                expbias=1-expbias-mantissa+1;
            }
            else if(explimb<expbias*2+1){//normal finite
                expbias=explimb-expbias-mantissa+1;
            }
            else{//inf
                explimb=expbias*2+1;
                msize=0;
            }
        }
        mp_limb_t mlimb=explimb,hlimb=0;
        mp_int mbits=exp_bits,is_normal=mlimb!=0;
        if(!hidden_bit){
            mlimb=mlimb*2+is_normal;
            ++mbits;
        }
        if(is_nan()){
            mlimb=mlimb*2+1;
            ++mbits;
        }
        mlimb|=signbit<<mbits;
        ++mbits;
        mp_byte_t *pb=(mp_byte_t*)fptr;
        mp_int wpos=type_bytes;
        mp_byte_t wbyte=0;
        do{
            while(mbits<8){
                mp_limb_t loadlimb=0;
                mp_int hbitpos=expbias+wpos*8-mbits-1;
                mp_int loadpos=dotp+(hbitpos>>MP_LOG2_LIMB_BITS);
                if(loadpos<msize&&loadpos>=0)loadlimb=data[loadpos];
                int shl=1+(hbitpos&MP_LIMB_BITS-1);
                hlimb=mlimb>>MP_LIMB_BITS-shl;
                if(shl==MP_LIMB_BITS)mlimb=loadlimb;
                else{
                    loadlimb&=(mp_limb_t(1)<<shl)-1;
                    mlimb=mlimb<<shl|loadlimb;
                }
                mbits+=shl;
            }
            mbits-=8;
            wbyte=mlimb>>mbits;
            if(mbits)wbyte|=hlimb<<MP_LIMB_BITS-mbits;
            if(wpos==0){
                wbyte=wbyte>=mp_byte_t(0x80);
                break;
            }
            pb[--wpos]=wbyte;
        } while(1);

        //carry
        if(wbyte){
            do{
                wbyte=++pb[wpos]==0;
                ++wpos;
            } while(wbyte);

            if(!hidden_bit){
                //Note: for floating point format without hidden bit, 
                //      direct bitwise carrying can not correctly handle overflow
                //      i.e. mantissa over flow will change explicit MSbit
                mp_int ipos=mantissa-1;
                mp_int test_bit=pb[ipos/8]>>(ipos&7)&1;
                if(is_normal!=test_bit){
                    ipos+=test_bit;
                    pb[ipos/8]|=mp_byte_t(1)<<(ipos&7);
                }
            }
        }

        return;
    }
};
