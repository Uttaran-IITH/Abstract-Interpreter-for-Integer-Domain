int main()
{
	int x ;
	int y ;
	int z ;

	x = 3 ;
	z = 4 ;

	int eti  ;
	eti = eti + 10 ;
	
	y = x+z*x*z;
	int a1;
	a1 = 3 + 4 + 5 + y;

	//int a = (x>z)? x : z ;
	if(x==y)
		x = x+1;
	else
		y = y+2;
	
	for(int i=0 ; i<5 ; i++)
	{
		x = x + 1 ;

		for(int j=0 ; j<10 ; j++)
		{
			y = y - 1;
		}
	}
	return 0 ;
}