#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>

char *sector_usage;	//block 단위의 Disk의 usage falg
int DISK_SIZE_SECTOR;

void periodic(FILE* input);
void sequential(FILE* input);
void random_(FILE* input);
void init_sector_usage();
int set_start_sector(int file_size);

int main(int argc, char*argv[])
{
	char pattern[10];
	FILE *input;

	if (argc != 2)
	{
		printf ("Usage: tracegen DiskSize(GB)\n");
		return 0;
	}

	DISK_SIZE_SECTOR = atoi(argv[1])*pow(2,21);
	sector_usage = calloc(DISK_SIZE_SECTOR, sizeof(char));
	if (sector_usage == NULL) {
	    perror("calloc");
	    exit(1);
	}

	input = fopen("input", "r");	
//	input = stdin;
	fscanf(input, "pattern file_size(MB) io_size(sector) bandwidth(Mbps) io횟수 inter_arrival_time(jiffies)\n");
	init_sector_usage();
	srand(time(NULL));

//	printf("type\tmajor(minor)\tI/O Pattern\tstart_sector\tlength\tjiffies\tdeadline\n");
	
	
	while (fscanf(input, "%s", pattern) != -1)
	{
		if (strcmp(pattern, "periodic") == 0)
		{
			periodic(input);
		}
		else if (strcmp(pattern, "random") == 0)
		{
			random_(input);
		}
		else if (strcmp(pattern, "sequential") == 0)
		{
			sequential(input);
		}
		else
		{
			printf ("Wrong Input\n");
		}
	}

	fclose(input);	
	free(sector_usage);
	return 0;
}


void init_sector_usage()
{
	int i;

	for (i = 0; i < DISK_SIZE_SECTOR/2; i++)
	{
		sector_usage[i] = 0;
	}
}

int set_start_sector(int file_size)
{
	int start, end;
	int block_size;
	int i, hit;

	block_size = file_size / 2;
	
	while(1)
	{
		hit = 1;
		start = rand() % (DISK_SIZE_SECTOR/2);
		end = start + block_size;

		if (end >= DISK_SIZE_SECTOR / 2)
			continue;
		
		for (i = start; i < end; i++)
		{
			if (sector_usage[i] == 1)
			{
				hit = 0;
				break;
			}
		}

		if (hit == 1)
			break;
	}
	
	for (i = start; i < end; i++)
		sector_usage[i] = 1;

	return start * 2;
}

/*
void periodic(FILE *input)
{
	int file_size;
	int bandwidth;
	int io_size;
	double deadline;
	int file_size_sector;
	int start_sector;
	int count;
	int jiffies;
	FILE *output;
		
	output = fopen("process_info", "a");
	
	fscanf(input, "%d", &file_size);	
	fscanf(input, "%d", &io_size);
	fscanf(input, "%d", &bandwidth);

	fprintf(output, "Pattern: %s\n", "periodic");
	fprintf(output, "File Size: %d MB\n", file_size);
	fprintf(output, "Bandwidth: %d Mbps\n", bandwidth);
	fprintf(output, "I/O Size: %d sectors\n", io_size);
	
	//calculation deadline
	deadline = ((double)bandwidth * pow(10, 6)) / ((double)io_size * pow(2, 12));
	deadline = 1000 / deadline;
	fprintf (output, "Deadline : %d jiffies\n", (int)deadline);
		
	//set file location by uniform distribution
	file_size_sector = file_size * pow(2, 11);
	start_sector = set_start_sector(file_size_sector);
	fprintf(output, "File Size (by sector): %d\n", file_size_sector);
	fprintf(output, "File Location (by sector number): %d ~ %d\n", start_sector, start_sector+file_size_sector);	
	fprintf(output, "========================================================\n");
	fclose(output);
	
	count = 0;
//	jiffies = rand() % 10000;
	jiffies = 10;
		
	while(1)
	{	
		printf("%c\t%d(%d)\t\t%s\t%.9d\t%d\t%d\t%d\n",'r', 3, 3, "periodic", 
							start_sector, io_size, jiffies, (int)deadline);
		
		count = count + io_size;
		if (count >= file_size_sector)
			break;

		start_sector += io_size;
		jiffies += (int)deadline;
		
		if (count + io_size > file_size_sector)
		{
			io_size = file_size_sector - count;
		}

	}	

}
*/
void sequential(FILE* input)
{
	int file_size;
	int io_size;
	int file_size_sector;
	int start_sector;
	int count;
	int jiffies; 
	FILE* output;

	output = fopen("process_info", "a");
	
	fscanf(input, "%d", &file_size);
	fscanf(input, "%d", &io_size);

	fprintf(output, "pattern: %s\n", "sequential");
	fprintf(output, "File Size: %d MB\n", file_size);
	fprintf(output, "I/O Size: %d sectors\n", io_size);

	//set file location by uniform distribution
	file_size_sector = file_size * pow(2, 11);
	start_sector = set_start_sector(file_size_sector);
	fprintf(output, "File Size (by sector): %d\n", file_size_sector);
	fprintf (output, "File Location (by sector number): %d ~ %d\n", start_sector, start_sector+file_size_sector);
	fprintf(output, "========================================================\n");
	fclose(output);

	count = 0;
	jiffies = rand() % 5000;
	
	while(1)
	{	
		printf("%c\t%d(%d)\t\t%s\t%.9d\t%d\t%d\t%d\n",'r', 3, 3, "sequential", start_sector, io_size, jiffies, 0);
		
		count = count + io_size;
		if (count >= file_size_sector)
			break;

		start_sector += io_size;
		jiffies += 15;
		
		if (count + io_size > file_size_sector)
		{
			io_size = file_size_sector - count;
		}
	}	
}

#define HZ 1000
#define SECTOR_PER_PAGE 8
#define IO_READ_RATIO	2
#define IO_WRITE_RATIO	1

void random_(FILE* input)
{
	int file_size;
	int io_frequency;
//	int inter_arrival_time;
	double inter_arrival_time;
	int min_io_size, max_io_size;	
	int file_size_sector;
	int size_count, num_count;
//	int jiffies;
	int io_size;
	FILE* output;
	
	const gsl_rng_type *T;				// 각 generator에 대한 static information
	gsl_rng *r_exp, *r_uni, *r_par;	// 주어진 gsl_rng_type으로 부터 생성되는 generator instance

	int ilocation;
	double io_location;
//	int interval;
	double interval;
	double jiffies;

	double io_ratio;
	int io_type;

	if((output = fopen("process_info", "wr")) == NULL){
		printf("fopen error\n");
		exit(-1);
	}
	
	fscanf(input, "%d", &file_size);
	fscanf(input, "%d~%d", &min_io_size, &max_io_size);
	fscanf(input, "%d", &io_frequency);
	fscanf(input, "%lf", &inter_arrival_time);

	fprintf(output, "pattern: %s\n", "random");
	fprintf(output, "File_Size: %d MB\n", file_size);
	fprintf(output, "I/O 횟수: %d 회\n", io_frequency);
	fprintf(output, "Inter Arrival Time: %lf jiffies\n", inter_arrival_time);
	fprintf(output, "I/O Range: %d - %d sectors\n", min_io_size, max_io_size);

        //set file location by uniform distribution
        file_size_sector = file_size * pow(2, 11);
        fprintf(output, "File Size (by sector): %d\n", file_size_sector);
	fprintf(output, "========================================================\n");
	fclose(output);
	
	gsl_rng_env_setup();		//GSL_RNG_TYPE, GSL_RNG_SEED를 이용해
					//gsl_rng_default, gsl_rng_default_seed를 설정한다.

	T = gsl_rng_default;		
	r_exp = gsl_rng_alloc(T);	//type T로 부터 새로운 generator instance를 할당
	r_uni = gsl_rng_alloc(T);
	r_par = gsl_rng_alloc(T);
	
	gsl_rng_set(r_exp, time(NULL));
	gsl_rng_set(r_uni, time(NULL));
	gsl_rng_set(r_par, time(NULL));

	size_count = 0;
	num_count = 0;
//	jiffies = rand() % 10000;
	jiffies = 1;

	while (size_count < file_size_sector && num_count < io_frequency)
	{
		//IO Size를 uniform하게 생성한다.
		io_size = gsl_rng_uniform_int(r_uni, max_io_size - min_io_size);
		io_size += min_io_size;

		if (size_count + io_size > file_size_sector)
		{
			io_size = file_size_sector - size_count;
		}

		//IO Type 생성
		io_ratio = rand() % 100;
		if((double)(IO_READ_RATIO/(double)(IO_READ_RATIO+IO_WRITE_RATIO)) * 100 < io_ratio){
			io_type = 1;	// READ
		}else{
			io_type = 0;	// WRITE
		}

		//IO Location을 uniform하게 생성한다.
//		io_location = set_start_sector(io_size);
		// Pareto Distribution
		io_location = gsl_ran_pareto(r_par, 7, 1);
		ilocation = ((int)((int)(io_location*1000000)%10000000));
//		ilocation = ((int)(io_location*10000)%9999999);

		//request의 inter arrival time을 exponential distribution으로 생성한다.
//		interval = (int)gsl_ran_exponential(r_exp, inter_arrival_time);
		interval = gsl_ran_exponential(r_exp, inter_arrival_time);
		
//		printf("%c\t%d(%d)\t\t%s\t\t%.9d\t%d\t%d\t%d\n",'r', 3, 3, "random", io_location, io_size, jiffies, 0);
//		printf("%c\t%d(%d)\t\t%s\t\t%8d\t%d\t%d\t%d\n",'r', 3, 3, "random", ilocation, io_size, jiffies, 0);
//		printf("%lf\t0\t%9d\t%d\t%d\n",((double)jiffies/HZ/(double)1000), ilocation, io_size/SECTOR_PER_PAGE, io_type);
		printf("%lf\t0\t%9d\t%d\t%d\n", jiffies, ilocation, io_size/SECTOR_PER_PAGE, io_type);
		
		size_count += io_size;
		num_count++;
		jiffies += interval;
	
	}

	gsl_rng_free(r_exp);
	gsl_rng_free(r_uni);
	gsl_rng_free(r_par);
}

void periodic(FILE* input)
{
	int file_size;
	int io_frequency;
	int inter_arrival_time;
	int min_io_size, max_io_size;	
	int file_size_sector;
	int size_count, num_count;
	int jiffies;
	int io_size;
	FILE* output;
	
	const gsl_rng_type *T;		//각 generator에 대한 static information
	gsl_rng *r_exp, *r_uni;		//주어진 gsl_rng_type으로 부터 생성되는 generator instance

	int io_location;
	int interval;

	output = fopen("process_info", "a");
	
	fscanf(input, "%d", &file_size);
	fscanf(input, "%d~%d", &min_io_size, &max_io_size);
	fscanf(input, "%d", &io_frequency);
	fscanf(input, "%d", &inter_arrival_time);

	fprintf(output, "pattern: %s\n", "periodic");
	fprintf(output, "File_Size: %d MB\n", file_size);
	fprintf(output, "I/O 횟수: %d 회\n", io_frequency);
	fprintf(output, "Inter Arrival Time: %d jiffies\n", inter_arrival_time);
	fprintf(output, "I/O Range: %d - %d sectors\n", min_io_size, max_io_size);

        //set file location by uniform distribution
        file_size_sector = file_size * pow(2, 11);
        fprintf(output, "File Size (by sector): %d\n", file_size_sector);
	fprintf(output, "========================================================\n");
	fclose(output);
	
	gsl_rng_env_setup();		//GSL_RNG_TYPE, GSL_RNG_SEED를 이용해
					//gsl_rng_default, gsl_rng_default_seed를 설정한다.

	T = gsl_rng_default;		
	r_exp = gsl_rng_alloc(T);	//type T로 부터 새로운 generator instance를 할당
	r_uni = gsl_rng_alloc(T);
	
	gsl_rng_set(r_exp, time(NULL));
	gsl_rng_set(r_uni, time(NULL));

	size_count = 0;
	num_count = 0;
//	jiffies = rand() % 10000;
	jiffies = 1;

	while (size_count < file_size_sector && num_count < io_frequency)
	{
		//IO Size를 uniform하게 생성한다.
		io_size = gsl_rng_uniform_int(r_uni, max_io_size - min_io_size);
		io_size += min_io_size;

		if (size_count + io_size > file_size_sector)
		{
			io_size = file_size_sector - size_count;
		}

		//IO Location을 uniform하게 생성한다.
		io_location = set_start_sector(io_size);

		//request의 inter arrival time을 exponential distribution으로 생성한다.
		interval = (int)gsl_ran_exponential(r_exp, inter_arrival_time);
		
		printf("%c\t%d(%d)\t\t%s\t\t%.9d\t%d\t%d\t%d\n",'r', 3, 3, "periodic", io_location, io_size, jiffies, io_size);
		
		size_count += io_size;
		num_count++;
		jiffies += interval;
	}	

	gsl_rng_free(r_exp);
	gsl_rng_free(r_uni);
}
