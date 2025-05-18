#pragma once
#include <vector>
#include "Yield.h"
#include "Caplet.h"

class Cap {
private:
	int maturity;
	double market_vol;
	double market_price;
	double swap_rate;
	int N;
	std::vector<double> payment_schedule;
	std::vector<Caplet> caplets;
public:
	Cap(int maturity_, double market_vol_, double swap_rate_, int N_, const std::vector<double>& payment_schedule_);
    int get_maturity() const;
	double get_vol() const;
	double get_price() const;
	void set_price(double price);
	double get_swap_rate() const;
	int get_N() const;
	std::vector<double> get_schedule() const;
	std::vector<Caplet>& get_caplets();
	const std::vector<Caplet>& get_caplets() const;
};