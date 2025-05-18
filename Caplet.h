#pragma once
class Caplet {
private:
	const double T;
	const double S;
	double zero_bond_T;
	double zero_bond_S;
	const double K;
	const int N;
public:
	Caplet(double T_, double S_, double K_, int N_);
	const double get_T() const;
	const double get_S() const;
	void set_zero_bond_T(double p_T);
	void set_zero_bond_S(double p_S);
	double get_zero_bond_T() const;
	double get_zero_bond_S() const;
	const double get_K() const;
	const int get_N() const;
};