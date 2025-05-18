#pragma once
#include <cmath>
#include <vector>
#include <cassert>
#include "Yield.h"
#include "Cap.h"
#include "Caplet.h"

double g2pp_zbc_price(const Yield& curve, double T, double S, double K, const std::vector<double>& params, double zero_bond_T, double zero_bond_S);

double g2pp_zbp_price(const Yield& curve, double T, double S, double K, const std::vector<double>& params, double zero_bond_T, double zero_bond_S);

std::vector<double> g2pp_zbp_price_gradient(const Yield& curve, double T, double S, double K, const std::vector<double>& params,
    std::vector<double>& grad, double zero_bond_T, double zero_bond_S);

double g2pp_caplet_price(const Yield& curve, const Caplet& caplet, const std::vector<double>& params);

std::vector<double> g2pp_caplet_price_gradient(const Yield& curve, const Caplet& caplet, const std::vector<double>& params, std::vector<double>& grad);

double g2pp_cap_price(const Yield& curve, const Cap& cap, const std::vector<double>& params);

std::vector<double> g2pp_cap_price_gradient(const Yield& curve, const Cap& cap, const std::vector<double>& params, std::vector<double>& grad);

double g2pp_shift(const Yield& curve, const std::vector<double>& params, double T);
