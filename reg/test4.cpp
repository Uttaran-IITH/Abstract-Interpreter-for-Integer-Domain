#include <assert.h>
int main(){
	int a = 2;
	int b = 7;
	int c = -b/a;
	int max;
	if(a > b){
		if(a > c)
			max = a;
		else
			max = c;
	}
	else{
		if (b > c)
			max = b;
		else 
			max = c;
	}
	assert (max == b);
}