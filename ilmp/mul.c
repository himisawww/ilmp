#include"ilmpn.h"
//This is a simplified version of mul.c
/*
	Areas where the different toom algorithms can be called
0         1/5 1/4  1/3 2/5  1/2 5/9 3/5 2/3 3/4 4/5  9/10   1  nb/na
		                5/12            11/16
                             |-------------------|xxxxxxxxxx+  toom22
                    |------------|xxxxxxx+xxxxxxx|----------|  toom32
               |----|xxxxxxxx+xxx|-------|                     toom42
                                         |-------|xxxxxxxxxx+  toom33
***********************    NOT IMPLEMENTED    ***********************
                                             |--------|xxxxx+  toom44
                                     |-------|xxx+xxxx|-----|  toom54
                             |------------|xx+xxx|----------|  toom43
                        |--------|xxx+xxxx|--|                 toom53
                    |----|xxx+xxx|---|                         toom63
           |--------|xxx+|---|                                 toom52
		                5/12            11/16
0         1/5 1/4  1/3 2/5  1/2 5/9 3/5 2/3 3/4 4/5  9/10   1  nb/na
*/


void ilmp_sqr_(mp_ptr dst,mp_srcptr numa,mp_size_t na){
	if(na<MUL_TOOM22_THRESHOLD)ilmp_sqr_basecase_(dst,numa,na);
	else if(na<MUL_TOOM33_THRESHOLD)ilmp_sqr_toom2_(dst,numa,na);
	else if(na<MUL_FFT_THRESHOLD)ilmp_sqr_toom3_(dst,numa,na);
	else ilmp_mul_fft_(dst,numa,na,numa,na);
}

void ilmp_mul_n_(mp_ptr dst,mp_srcptr numa,mp_srcptr numb,mp_size_t n){
	if(n<MUL_TOOM22_THRESHOLD)ilmp_mul_basecase_(dst,numa,n,numb,n);
	else if(n<MUL_TOOM33_THRESHOLD)ilmp_mul_toom22_(dst,numa,n,numb,n);
	else if(n<MUL_FFT_THRESHOLD)ilmp_mul_toom33_(dst,numa,n,numb,n);
	else ilmp_mul_fft_(dst,numa,n,numb,n);
}

void ilmp_mul_(mp_ptr dst,mp_srcptr numa,mp_size_t na,mp_srcptr numb,mp_size_t nb){
	if(na==nb){
		if(numa==numb)ilmp_sqr_(dst,numa,na);
		else ilmp_mul_n_(dst,numa,numb,na);
	}
	else if(nb<MUL_TOOM22_THRESHOLD||nb<MUL_TOOMX2_THRESHOLD&&!(4*na<5*nb)){
		if(na<=PART_SIZE||nb<=2)ilmp_mul_basecase_(dst,numa,na,numb,nb);
		else{
			mp_limb_t tp[MUL_TOOMX2_THRESHOLD];
			ilmp_mul_basecase_(dst,numa,PART_SIZE,numb,nb);
			dst+=PART_SIZE;
			numa+=PART_SIZE;
			na-=PART_SIZE;
			ilmp_copy(tp,dst,nb);
			while(na>PART_SIZE){
				ilmp_mul_basecase_(dst,numa,PART_SIZE,numb,nb);
				if(ilmp_add_n_(dst,dst,tp,nb))ilmp_inc(dst+nb);
				dst+=PART_SIZE;
				numa+=PART_SIZE;
				na-=PART_SIZE;
				ilmp_copy(tp,dst,nb);
			}
			if(na>=nb)ilmp_mul_basecase_(dst,numa,na,numb,nb);
			else ilmp_mul_basecase_(dst,numb,nb,numa,na);
			if(ilmp_add_n_(dst,dst,tp,nb))ilmp_inc(dst+nb);
		}
	}
	else if((na+nb>>1)<MUL_FFT_THRESHOLD||2*nb<MUL_FFT_THRESHOLD){
		if(na<3*nb){
			if(4*na<5*nb){
				if(nb<MUL_TOOM33_THRESHOLD)ilmp_mul_toom22_(dst,numa,na,numb,nb);
				else ilmp_mul_toom33_(dst,numa,na,numb,nb);
			}
			else if(5*na<9*nb)ilmp_mul_toom32_(dst,numa,na,numb,nb);
			else ilmp_mul_toom42_(dst,numa,na,numb,nb);
		}
		else{
			mp_limb_t *ws=SALLOC_TYPE(nb,mp_limb_t);
			ilmp_mul_toom42_(dst,numa,2*nb,numb,nb);
			dst+=2*nb;
			numa+=2*nb;
			na-=2*nb;
			ilmp_copy(ws,dst,nb);
			while(2*na>=5*nb){
				ilmp_mul_toom42_(dst,numa,2*nb,numb,nb);
				if(ilmp_add_n_(dst,dst,ws,nb))ilmp_inc(dst+nb);
				dst+=2*nb;
				numa+=2*nb;
				na-=2*nb;
				ilmp_copy(ws,dst,nb);
			}
			// 0.5 nb <= na < 2.5 nb
			if(na>=nb)ilmp_mul_(dst,numa,na,numb,nb);
			else ilmp_mul_(dst,numb,nb,numa,na);
			if(ilmp_add_n_(dst,dst,ws,nb))ilmp_inc(dst+nb);
		}
	}
	else{
		if(na<8*nb)ilmp_mul_fft_(dst,numa,na,numb,nb);
		else{
			mp_ptr ws=ALLOC_TYPE(nb,mp_limb_t);
			ilmp_mul_fft_(dst,numa,3*nb,numb,nb);
			dst+=3*nb;
			numa+=3*nb;
			na-=3*nb;
			ilmp_copy(ws,dst,nb);
			while(2*na>=7*nb){
				ilmp_mul_fft_(dst,numa,3*nb,numb,nb);
				if(ilmp_add_n_(dst,dst,ws,nb))ilmp_inc(dst+nb);
				dst+=3*nb;
				numa+=3*nb;
				na-=3*nb;
				ilmp_copy(ws,dst,nb);
			}
			//0.5 nb <= na < 3.5 nb
			if(na>=nb)ilmp_mul_(dst,numa,na,numb,nb);
			else ilmp_mul_(dst,numb,nb,numa,na);
			if(ilmp_add_n_(dst,dst,ws,nb))ilmp_inc(dst+nb);
			FREE(ws);
		}
	}
}

