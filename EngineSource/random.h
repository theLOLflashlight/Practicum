#pragma once

void reset_normal();
double rand_normal();
double rand_normal( double sigma );
double rand_normal( double mean, double sigma );

int rand_int();
int rand_int( int max );
int rand_int( int min, int max );

double rand_real();
double rand_real( double max );
double rand_real( double min, double max );

bool chance( double ratio );
