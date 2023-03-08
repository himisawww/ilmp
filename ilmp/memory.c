#include"ilmpn.h"

#define HSIZE sizeof(void *)

void *ilmp_temp_alloc_(void **pmarker,size_t size){
	void *p=ilmp_alloc(size+HSIZE);
	*(void **)p=*pmarker;
	*pmarker=p;
	return (mp_byte_t *)p+HSIZE;
}

void ilmp_temp_free_(void *marker){
	while(marker){
		void *next=*(void **)marker;
		ilmp_free(marker);
		marker=next;
	}
}

void *ilmp_alloc(size_t size){
	void *ret=malloc(size);
	ilmp_assert(ret);
	return ret;
}

void *ilmp_realloc(void *oldptr,size_t new_size){
	void *ret=realloc(oldptr,new_size);
	ilmp_assert(ret);
	return ret;
}

void ilmp_free(void *ptr){
	free(ptr);
}
