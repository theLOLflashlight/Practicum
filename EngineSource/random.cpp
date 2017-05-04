#include "random.h"
#include <random>

thread_local std::mt19937 rand_engine( std::random_device {}() );

thread_local std::normal_distribution< double >             normal_dist;
const thread_local std::uniform_int_distribution< int >     uni_int_dist;
const thread_local std::uniform_real_distribution< double > uni_real_dist;


void reset_normal()
{
    normal_dist.reset();
}

double rand_normal()
{
    return normal_dist( rand_engine );
}

double rand_normal( double sigma )
{
    return rand_normal( 0.0, sigma );
}

double rand_normal( double mean, double sigma )
{
    return normal_dist( rand_engine, std::normal_distribution< double >::param_type( mean, sigma ) );
}


int rand_int()
{
    return uni_int_dist( rand_engine );
}

int rand_int( int max )
{
    return rand_int( 0, max );
}

int rand_int( int min, int max )
{
    return uni_int_dist( rand_engine, std::uniform_int< int >::param_type( min, max ) );
}


double rand_real()
{
    return uni_real_dist( rand_engine );
}

double rand_real( double max )
{
    return rand_real( 0.0, max );
}

double rand_real( double min, double max )
{
    return uni_real_dist( rand_engine, std::uniform_real< double >::param_type( min, max ) );
}

bool chance( double ratio )
{
    return rand_real() < ratio;
}
