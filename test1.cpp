#include <assert.h>
int main(){
	int i = 1;
	int j = 5;
	while (i < j){
		int k = i;
		int sum = 0;
		while ( k > 0){
			sum = sum + i;
			k--;
		}
		int s = i*(i+1)/2;
		assert (sum <= s);
		i++;
	}
}
