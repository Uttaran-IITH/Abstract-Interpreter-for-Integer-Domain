// TESTING OF IF CONDITIONS
#include <assert.h>

int main()
{
	int a  = 0 ;
	int b = -3 ;
	int c ;
	int d;

	//works fine
	c = a + (-b);
	a = a+1 ;
	//c = c -1 ;

	assert(c <10);
	if(c<=2 || a == 0)
	{
		c = c+1 ;
		if(a == 0)
		{
			a = a+1 ;
		}

		else
		{
			a = a+3 ;
		}
	}

	else
	{
		c = c-1 ;

		if(d<5)
		{
			b=-b;
		}
		else
		{
		b = b - 4 ;
		}


			if(b > 0)
			{
				a = a + 2 ;
			}
			else
			{
				a = a - 2 ;
			}
		

		assert(a>0);

		c = c - 2 ; 
	}

	assert(c<10);
	//assert(c<10 && a>0);
	// c = c -2 ;

	return 0 ;
}