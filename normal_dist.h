#pragma once
#include <vector>
#include <random>

const double PI = 3.14159265358979323846;

double norm_cdf(double x);

double norm_pdf(double x);

void generate_multivariate_normal(std::vector<double>& values1, std::vector<double>& values2, std::normal_distribution<double>& normal, 
	std::default_random_engine& generator, double var1, double var2, double cov);