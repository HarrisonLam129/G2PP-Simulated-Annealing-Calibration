#pragma once
#include "Yield.h"
#include "Caplet.h"
#include "Cap.h"

double black_caplet_price(const Yield& curve, const Caplet& caplet, double sigma);

double black_cap_price(const Yield& curve, const Cap& cap, double sigma);

double black_caplet_vol(const Yield& curve, const Caplet& caplet, double target_price);

double black_cap_vol(const Yield& curve, const Cap& cap, double target_price);