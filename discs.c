#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <nlopt.h>
#include <unistd.h>
#include <sys/time.h>

void quadbezier(const double *a, const double *b, const double *c,
		double t, double *out)
{
  double onemt = 1.0 - t;
  out[0] = onemt * (onemt * a[0] + t * b[0]) + t * (onemt * b[0] + t * c[0]);
  out[1] = onemt * (onemt * a[1] + t * b[1]) + t * (onemt * b[1] + t * c[1]);
}

void linearinterp(const double *a, const double *b,
		  double t, double *out)
{
  out[0] = (b[0] - a[0]) * t + a[0];
  out[1] = (b[1] - a[1]) * t + a[1];
}

typedef struct {
  int N;
  int steps;
  double *starts;
  double *stops;
  double *scratch;
} problem_t;

double randd()
{
  int n = rand() % (1<<24);
  return ((double)n) / ((double)(1<<24));
}

double distance(const double *a, const double *b)
{
  return sqrt((b[0] - a[0]) * (b[0] - a[0]) +
	      (b[1] - a[1]) * (b[1] - a[1]));
}

#define RADIUS 5.0

double errorf(unsigned n, const double *pts, double *grad,
	      void *data)
{
  problem_t *setup = (problem_t *)data;
  double error = 0.0;

  for(int step=0; step<setup->steps; step++) {
    double t = (1.0+step) / (1.0+setup->steps);
    for(int i=0; i<setup->N; i++)
      quadbezier(&setup->starts[2*i],
		 &pts[2*i],
		 &setup->stops[2*i],
		 t,
		 &setup->scratch[2*i]);

    for(int i=0; i<setup->N; i++)
      for(int j=0; j<setup->N; j++)
	if(i!=j) {
	  double d = distance(&setup->scratch[2*i],
			      &setup->scratch[2*j]);
	  d /= RADIUS;
	  //if(d<=1.0)
	    error += (1.0/d) * (1.0/d);
	}
  }

  return error / setup->steps;
}

int main(int argc, char *argv[])
{
  int N = 50;

  double *starts = (double *)malloc(sizeof(double)*N*2);
  double *stops = (double *)malloc(sizeof(double)*N*2);

  srand(2);
  nlopt_srand(1);

  double scaling = 100.0;
  for(int i=0; i<N; i++) {
    starts[2*i+0] = scaling * randd();
    starts[2*i+1] = scaling * randd();
    stops[2*i+0]  = scaling * randd();
    stops[2*i+1]  = scaling * randd();
  }

  double hs = scaling / 2.0;
  for(int i=0; i<N; i++) {
    double theta = ((double)i)/N * 2 * 3.141592;
    starts[2*i+0] = hs * cos(theta) + hs;
    starts[2*i+1] = hs * sin(theta) + hs;
    stops[2*i+0]  = hs * cos(theta + 3.141592) + hs;
    stops[2*i+1]  = hs * sin(theta + 3.141592) + hs;
  }
  
  struct timeval start, stop;
  
  double *guesses = (double *)malloc(sizeof(double)*N*2);

  gettimeofday(&start, NULL);
  for(int i=0; i<N; i++) {
    linearinterp(&starts[2*i], &stops[2*i], 0.5, &guesses[2*i]);
  }

  nlopt_opt global = nlopt_create(NLOPT_GN_ISRES, 2*N);
  nlopt_opt local = nlopt_create(NLOPT_LN_SBPLX, 2*N);

  problem_t *setup = (problem_t *)malloc(sizeof(problem_t));
  setup->N = N;
  setup->starts = starts;
  setup->stops = stops;
  setup->steps = 50;
  setup->scratch = (double *)malloc(sizeof(double)*N*2);

  nlopt_result res;
  
  res = nlopt_set_min_objective(global, errorf, setup);
  if(res!=1) { exit(-1); }

  res = nlopt_set_lower_bounds1(global, -25.0);
  if(res!=1) { exit(-1); }

  res = nlopt_set_upper_bounds1(global, 125.0);
  if(res!=1) { exit(-1); }

  res = nlopt_set_stopval(global, 1.0);
  if(res!=1) { exit(-1); }

  res = nlopt_set_maxeval(global, 100);
  if(res!=1) { exit(-1); }

  double final_error;
  res = nlopt_optimize(global, guesses, &final_error);

  res = nlopt_set_min_objective(local, errorf, setup);
  if(res!=1) { exit(-1); }

  res = nlopt_set_lower_bounds1(local, -25.0);
  if(res!=1) { exit(-1); }

  res = nlopt_set_upper_bounds1(local, 125.0);
  if(res!=1) { exit(-1); }

  res = nlopt_set_stopval(local, 1.0);
  if(res!=1) { exit(-1); }

  res = nlopt_set_maxeval(local, 1000);
  if(res!=1) { exit(-1); }

  res = nlopt_optimize(local, guesses, &final_error);
  gettimeofday(&stop, NULL);

  fprintf(stderr, "final: %f\n", final_error);

  for(int i=0; i<N; i++) {
    printf("%f %f %f %f %f %f\n",
	   starts[2*i+0], starts[2*i+1],
	   guesses[2*i+0], guesses[2*i+1],
	   stops[2*i+0], stops[2*i+1]);
  }

  double startd, stopd;
  startd = start.tv_sec;
  startd += ((double)(start.tv_usec)) / 1e6;
  stopd = stop.tv_sec;
  stopd += ((double)(stop.tv_usec)) / 1e6;

  //printf("%f\n", stopd - startd);
}
