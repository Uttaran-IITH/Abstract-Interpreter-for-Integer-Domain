main(){
	int i=9,j=1,c=-1;
	unsigned int u = i;
	while ( i >= 0) {
	if ( j + c == 0){
		j = 2;
		u = u-1;
	}
	else{
			i--;
			c = c * j;
		}
	}
	assert(u == 0);
}
