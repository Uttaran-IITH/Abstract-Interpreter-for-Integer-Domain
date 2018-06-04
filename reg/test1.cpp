#include <assert.h>
int main(){
	int c;
	int a;
	int b;
	int t ;
	if (c<0){
		a = 1;
		b = 2;
	}
	else{
		a = 5;
		b = 6;
	}

	while(c>0)
	{
		t = a ;
		a = b ;
		b = t ;
	}
	
	if(a<b){
		c = 0;
	}	
	else{
		c = 1;
		assert(b<=5);
	}
}