main(){
	int i = 0;
	int j;
	j = i + 2;
	while (i < j){
		i = i + 1;
	}
	assert(i + j >= 2);
	assert(i + j >= 4);
}
