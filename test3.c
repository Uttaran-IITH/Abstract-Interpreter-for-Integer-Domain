main(){
	int i = 0;
	int j = 0;
	int h = 10;
	while(i < 5){
		h = h / 2;
		while( j < 2 * h ){
			h = h * 2;
			j = j + h;
		}
		assert ( h <= 10 ); 
	}
	assert ( h < 0 );
}
