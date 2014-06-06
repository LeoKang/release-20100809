#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

//second = jiffy / HZ
#define HZ 1000
#define SECTOR_PER_PAGE 8

#define READ_RATIO	3
#define WRITE_RATIO	2

#define TRUE 1
#define FALSE 0

int zipf(double alpha, int n);
double rand_val(int seed);

int main(int argc, char*argv[])
{
	char line[80];
	char iotype;
	int io, no_dev;
	double io_ratio;
	char *dev, *trace_type;
	int logical_adrs, tmp;
	int size;
	double jiffies;
	FILE *input, *output;
/*
	if (argc != 2)
	{
		printf ("Usage: parsing trace_name\n");
		return 0;
	}
*/
	if((input = fopen("input_trace.data", "r")) == NULL){
		printf("fopen error\n");
		exit(-1);	
	}
	if((output = fopen("micro.trace", "wr")) == NULL){
		printf("fopen error2\n");
		exit(-1);	
	}
	
	srand((unsigned)time(NULL));


	while(fgets(line, 80, input) != NULL){
		sscanf(line, "%lf\t%d\t%9d\t%d\t%d\n", &jiffies, &no_dev, &logical_adrs, &size, &io);

		io_ratio = rand() % 100;
		if((double)(WRITE_RATIO/(double)(READ_RATIO+WRITE_RATIO)) * 100 < io_ratio){
			io = 1;	// READ
		}else{
			io = 0;	// WRITE
		}
	
		logical_adrs = zipf(0.8, 10000000);

//		printf("%lf\t0\t%9d\t%d\t%d\n", jiffies, logical_adrs, size, io);
		fprintf(output, "%lf\t0\t%9d\t%d\t%d\n", jiffies, logical_adrs, size, io);
	}


	return 0;
}
/*
int zipfRandom(double skew)
{
	double a = skew;
	double b = pow(2.0, a - 1.0);
	double X, T, U, V;
	
	srand((unsigned)time(NULL));
	do{
		U = ((double)rand() / (double)(RAND_MAX)+(double)(1));
		V = ((double)rand() / (double)(RAND_MAX)+(double)(1));
//		X = floor(pow(U, (double)((-1.0)/a - 1.0)));
		X = pow(U, (double)((-1.0)/a - 1.0));
		T = pow(1.0 + (double)1.0/(double)X, a - 1.0);
	}while(V*X*(T-1.0)/(double)(b-1.0)>((double)T/(double)b));


	return (int)(X);
}
*/

//===========================================================================
//=  Function to generate Zipf (power law) distributed random variables     =
//=    - Input: alpha and N                                                 =
//=    - Output: Returns with Zipf distributed random variable              =
//===========================================================================
int zipf(double alpha, int n)
{
  static int first = TRUE;      // Static first time flag
  static double c = 0;          // Normalization constant
  double z;                     // Uniform random number (0 < z < 1)
  double sum_prob;              // Sum of probabilities
  double zipf_value;            // Computed exponential value to be returned
  int    i;                     // Loop counter

  // Compute normalization constant on first call only
  if (first == TRUE)
  {
    for (i=1; i<=n; i++)
      c = c + (1.0 / pow((double) i, alpha));
    c = 1.0 / c;
    first = FALSE;
  }

  // Pull a uniform random number (0 < z < 1)
  do
  {
//    z = rand_val(0);
		z = ((double)rand() / (double)(RAND_MAX));
  }
  while ((z == 0) || (z == 1));

  // Map z to the value
  sum_prob = 0;
  for (i=1; i<=n; i++)
  {
    sum_prob = sum_prob + c / pow((double) i, alpha);
    if (sum_prob >= z)
    {
      zipf_value = i;
      break;
    }
  }

  // Assert that zipf_value is between 1 and N
  assert((zipf_value >=1) && (zipf_value <= n));

  return(zipf_value);
}

//=========================================================================
//= Multiplicative LCG for generating uniform(0.0, 1.0) random numbers    =
//=   - x_n = 7^5*x_(n-1)mod(2^31 - 1)                                    =
//=   - With x seeded to 1 the 10000th x value should be 1043618065       =
//=   - From R. Jain, "The Art of Computer Systems Performance Analysis," =
//=     John Wiley & Sons, 1991. (Page 443, Figure 26.2)                  =
//=========================================================================
double rand_val(int seed)
{
  const long  a =      16807;  // Multiplier
  const long  m = 2147483647;  // Modulus
  const long  q =     127773;  // m div a
  const long  r =       2836;  // m mod a
  static long x;               // Random int value
  long        x_div_q;         // x divided by q
  long        x_mod_q;         // x modulo q
  long        x_new;           // New x value

  // Set the seed if argument is non-zero and then return zero
  if (seed > 0)
  {
    x = seed;
    return(0.0);
  }

  // RNG using integer arithmetic
  x_div_q = x / q;
  x_mod_q = x % q;
  x_new = (a * x_mod_q) - (r * x_div_q);
  if (x_new > 0)
    x = x_new;
  else
    x = x_new + m;

  // Return a random value between 0.0 and 1.0
  return ((double)x / m);
}

