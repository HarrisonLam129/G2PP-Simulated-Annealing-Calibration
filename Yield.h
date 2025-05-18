#pragma once
#include <vector>
#include "spline.h"

class Yield {
private:
	const std::vector<double> zero_yields;
	tk::spline s;
	std::vector<double> zero_bond_prices;
public:
	Yield(const std::vector<double>& zero_yields_);
	double zero_bond(double mat) const;
	double instantaneous_forward(double mat) const;
};