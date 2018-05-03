#include <assert.h>
int main(){
	int i = 5;
	int j = 1;
	int m = 100;
	for( int k = 1; k < i; k++)
		j = k * j;
	assert ( j > 0 );
	assert ( j < 720);
}
