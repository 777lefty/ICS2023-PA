#include<stdio.h>
#include<NDL.h>
int main(){
	struct timeval tv;
	int32_t cnt=0;
	while(1){
	  //gettimeofday(&tv,NULL);
	  //uint32_t time=tv.tv_sec*1000+tv.tv_usec/1000;
	  uint32_t time=NDL_GetTicks();
	  while(NDL_GetTicks()-time<500){
	 //   gettimeofday(&tv,NULL);
	  }
	  cnt++;
	  printf("(NDL) %d ms have passed\n",cnt*500);
	}
	return 0;
}
