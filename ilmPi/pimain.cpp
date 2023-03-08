#include<iostream>
#include"Number.h"
#include"CalcTime.h"

int main(int argc,char **argv){
	int base=0;
	ilmp::mp_uint digits=0;
	if(argc<=1){
	printf(
		"Pi Calculator using ilmp\n"
		"command line usage:\n\n"

		"    ilmpi.exe   [digits]   [[base]]\n\n"

		"        digits:  should be a decimal integer\n"
		"          base:  optional, default to 10\n\n"
	);
		base=10;
		printf("To calculate Pi\n");
		printf("     in base :   10\n");
		printf("    to digit :   ");
		char c;
		scanf("%llu%c",&digits,&c);
	}
	else {
		if(argc>=2)digits=atoll(argv[1]);
		if(argc>=3)base=atoi(argv[2]);
		else base=10;
	}
	printf("\nCalculating Pi in base %d to digit %llu.\n",base,digits);

	if(base<-36||base>-2&&base<2||base>36){
		fprintf(stderr,"  invalid base %d.\n\n",base);
		return 1;
	}
	if(digits==0||digits*std::log2((double)std::abs(base))>ilmp::MAX_PREC_BITS){
		fprintf(stderr,"  invalid # of digits %llu.\n\n",digits);
		return 1;
	}

	double t0=CalcTime();
	ilmp::Number pi=ilmp::Pi(1+digits,base);
	double t1=CalcTime();
	ilmp::mp_int len=pi.strlen(base);
	char *buf=new char[len];
	pi.to_str(buf,base);
	double t2=CalcTime();
	
	printf("\nDone. Time elapsed:\n"
		   "   calculate: %lfs\n",t1-t0);
	printf("base convert: %lfs\n",t2-t1);
	printf("       total: %lfs\n",t2-t0);

	char fname[64];
	sprintf(fname,"pi_%llu_%d.txt",digits,base);
	printf("\nWriting results to file: %s\n",fname);

	FILE *fout=fopen(fname,"w");
	fprintf(fout,"%s",buf);
	fclose(fout);
	delete[] buf;

	printf("Press Enter to Exit.");
	getchar();
	return 0;
}
