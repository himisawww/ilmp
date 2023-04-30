#include<iostream>
#include"Number.h"
#include"CalcTime.h"

const char *const_names[]={"pi", "e", "ln2", "prime_buenosaires"};
ilmp::Number (*const const_funcs[])(ilmp::mp_prec_t)={
    ilmp::Pi,
    ilmp::E,
    ilmp::const_ln2,
    ilmp::Prime_BuenosAires
};

int main(int argc,char **argv){
    int base=10,option=0;
    ilmp::mp_int digits=0;
    if(argc<=1||argc>4){
    printf(
        "Pi Calculator using ilmp\n"
        "command line usage:\n\n"

        "    ilmpi.exe [digits] [[base]] [[option]]\n\n"

        "        digits:  should be a decimal integer\n"
        "          base:  optional, default to 10\n"
        "        option:  0 for Pi, 1 for E, 2 for ln2, 3 for Prime_BuenosAires\n"
        "                           default to 0\n\n"

        "prompt usage:\n"
        "   Input a positive integer as number of desired decimal digits.\n"
        "   or input negative option index(0~-3) to change the constant.\n\n"
    );
        do{
            printf("To calculate %s\n",const_names[option]);
            printf("     in base :   10\n");
            printf("    to digit :   ");
            char c;
            scanf("%lld%c",&digits,&c);
            if(digits<=0&&digits>=-3)option=-digits;
            else break;
        } while(1);
    }
    else {
        if(argc>=2)digits=atoll(argv[1]);
        if(argc>=3)base=atoi(argv[2]);
        if(argc>=4)option=std::abs(atoi(argv[3]));
    }

    if(option<0||option>3){
        fprintf(stderr,"  invalid option %d.\n\n",option);
        return 1;
    }
    if(base<-36||base>-2&&base<2||base>36){
        fprintf(stderr,"  invalid base %d.\n\n",base);
        return 1;
    }
    ilmp::mp_prec_t target_precision=ilmp::convert_precision(1+digits,base);
    if(digits<=0||target_precision>ilmp::MAX_PREC_BITS){
        fprintf(stderr,"  invalid # of digits %lld.\n\n",digits);
        return 1;
    }
    printf("\nCalculating %s in base %d to digit %lld.\n",const_names[option],base,digits);

    double t0=CalcTime();
    ilmp::Number pi=const_funcs[option](target_precision);
    double t1=CalcTime();
    ilmp::mp_int len=pi.strlen(base);
    char *buf=new char[len];
    len=pi.to_str(buf,base);
    double t2=CalcTime();
    
    printf("\nDone. Time elapsed:\n"
           "   calculate: %lfs\n",t1-t0);
    printf("base convert: %lfs\n",t2-t1);
    printf("       total: %lfs\n",t2-t0);

    char fname[64];
    sprintf(fname,"%s_%lld_%d.txt",const_names[option],digits,base);
    printf("\nWriting results to file: %s\n",fname);

    FILE *fout=fopen(fname,"w");
    fwrite(buf,1,len,fout);//fprintf(fout,"%s",buf);
    fclose(fout);
    delete[] buf;

    printf("Press Enter to Exit.");
    getchar();
    return 0;
}
