#include <assert.h>
int main(){
	int i = 0;
	int j = 2;
	while( i < 10){
		if(i <= 2 || j == i)
			i++;
		else
			j++;
	}
	int k = i * j;
	assert(	k > 0);
}
