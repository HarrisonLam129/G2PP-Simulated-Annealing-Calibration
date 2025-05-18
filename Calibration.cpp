#include <iostream>
#include <cmath>
#include <vector>
#include <tuple>
#include <chrono>
#include <random>
#include "Yield.h"
#include "Black.h"
#include "g2pp.h"
#include "Cap.h"
#include "spline.h"

template<typename T, typename... S>
double time_function(T func, S... args) {
    auto start = std::chrono::high_resolution_clock::now();
    auto output = func(args...);
    auto stop = std::chrono::high_resolution_clock::now();
    const std::chrono::duration<double, std::micro> execution_time = stop - start;
    std::cout << "Time: " << execution_time.count() << " microseconds." << std::endl;

    return output;
}

double minimisation_objective(const Yield& curve, const std::vector<Cap>& caps, const std::vector<double>& params) {
    double calibration_error = 0, error, g2pp_price;
    for (int i = 0; i < caps.size(); i++) {
        g2pp_price = g2pp_cap_price(curve, caps[i], params);
        error = (caps[i].get_price() - g2pp_price) / caps[i].get_price();
        calibration_error += error * error;
    }
    return calibration_error;
}

std::tuple<double, double, double> vector_statistics(const std::vector<double>& v) {
    double l = DBL_MAX, u = -DBL_MAX;
    double sum = 0, sq_sum = 0;
    for (auto elem : v) {
        if (elem < l) {
            l = elem;
        } else if (elem > u) {
            u = elem;
        }
        sum += elem;
        sq_sum += elem * elem;
    }
    return std::make_tuple(l, u, std::sqrt(sq_sum / v.size() - (sum / v.size()) * (sum / v.size())));
}

template<typename T>
std::pair<std::vector<double>, double> simulated_annealing(const Yield& curve, const std::vector<Cap>& caps, T objective, const std::vector<std::pair<double, double>>& bounds, const std::vector<double>& jump_stds, double rate,
    size_t max_iter, double temperature, double termination_score, int seed, bool disp = false, const std::vector<double>& x0 = std::vector<double>()) {
    std::vector<double> current_params(5);
    std::default_random_engine generator(seed);
    std::uniform_real_distribution<double> uniform(0.0, 1.0);
    std::normal_distribution<double> normal(0.0, 1.0);
    double temperature_threshold = pow(10, -6);
    if (x0.size() != 5) {
        for (int i = 0; i < 5; i++) {
            current_params[i] = bounds[i].first + uniform(generator) * (bounds[i].second - bounds[i].first);
        }
    }
    else {
        current_params = x0;
    }
    double current_score = objective(curve, caps, current_params);
    size_t accepted_count = 0;
    std::vector<double> next_params(5);
    double next_score, acceptance_prob;
    for (size_t i = 0; i < max_iter; i++) {
        for (int j = 0; j < 5; j++) {
            next_params[j] = current_params[j] + normal(generator) * jump_stds[j];
            if (next_params[j] < bounds[j].first) {
                next_params[j] = bounds[j].first;
            }
            else if (next_params[j] > bounds[j].second) {
                next_params[j] = bounds[j].second;
            }
        }
        next_score = objective(curve, caps, next_params);
        acceptance_prob = exp((current_score - next_score) / temperature);
        if (next_score < current_score || uniform(generator) < acceptance_prob) {
            current_params = next_params;
            current_score = next_score;
            accepted_count++;
        }
        if (temperature > temperature_threshold) {
            temperature *= rate;
        }
        if (current_score < termination_score) {
            std::cout << i << " " << accepted_count << " ";
            break;
        }
    }
    return std::make_pair(current_params, current_score);
}
int main() {
    auto curve = Yield(std::vector<double> { 4.840355907, 4.745692608, 4.776552898, 4.852996706, 4.944066306, 5.036961092,
        5.126129493, 5.209143662, 5.285021281, 5.353489664, 5.414638913, 5.468747505,
        5.516189651, 5.557383935, 5.592763946, 5.622761184, 5.647795099, 5.668267386,
        5.684558899, 5.697028194, 5.706011101, 5.71182094, 5.714749147, 5.715066143,
        5.713022348, 5.70884927, 5.702760616, 5.694953404, 5.685609049, 5.674894415 });
    std::vector<int> cap_maturities{ 1, 2, 3, 4, 5, 7, 10, 15, 20 };
    std::vector<double> cap_market_vols{ 0.1520, 0.1620, 0.1640, 0.1630, 0.1605, 0.1555, 0.1475, 0.1350, 0.1260 };
    int notional = 1000000;

    std::vector<Cap> caps;
    caps.reserve(cap_maturities.size());
    double total, atm_swap_rate;
    for (int i = 0; i < cap_maturities.size(); i++) {
        std::vector<double> payment_schedule{ 0.25, 0.5, 0.75, 1. };
        payment_schedule.reserve(2 + 2 * cap_maturities[i]);
        total = 0.25 * curve.zero_bond(0.5) + 0.25 * curve.zero_bond(0.75) + 0.25 * curve.zero_bond(1);
        for (int j = 1; j < 2 * cap_maturities[i] - 1; j++) {
            payment_schedule.push_back(1 + 0.5 * j);
            total += 0.5 * curve.zero_bond(1 + 0.5 * j);
        }
        atm_swap_rate = (curve.zero_bond(0.25) - curve.zero_bond(cap_maturities[i])) / total;
        caps.push_back(Cap{ cap_maturities[i], cap_market_vols[i], atm_swap_rate, notional, payment_schedule });
        caps[i].set_price(black_cap_price(curve, caps[i], cap_market_vols[i]));
        std::vector<Caplet>& caplets = caps[i].get_caplets();
        for (auto& caplet : caplets) {
            caplet.set_zero_bond_T(curve.zero_bond(caplet.get_T()));
            caplet.set_zero_bond_S(curve.zero_bond(caplet.get_S()));
        }
    }

    //std::vector<std::pair<double, double>> bounds{ {0.01, 1}, {0.001, 0.2}, {0.01, 1}, {0.001, 0.2}, {-0.9999, 0.999} };
    //std::vector<std::pair<double, double>> bounds{ {0.1, 0.7}, {0.002, 0.025}, {0.05, 0.2}, {0.005, 0.03}, {-0.9999, -0.75} };
    std::vector<std::pair<double, double>> bounds{ {0.15, 0.5}, {0.003, 0.025}, {0.08, 0.18}, {0.008, 0.035}, {-0.999, -0.9} };

    std::vector<double> jump_stds(5);
    for (int i = 0; i < 5; i++) {
        jump_stds[i] = (bounds[i].second - bounds[i].first) / 20;
    }

    std::vector<double> optimal_params{ 0.271001,0.0209998,0.141665,0.0264076,-0.982809 };
    /*
    std::vector<double> fitted_prices(9);
    std::vector<double> fitted_vols(9);
    for (int i = 0; i < 9; i++) {
        fitted_prices[i] = g2pp_cap_price(curve, caps[i], params);
        fitted_vols[i] = black_cap_vol(curve, caps[i], fitted_prices[i]);
        std::cout << fitted_vols[i] << ", ";
    }
    std::cout << std::endl;
    */
    std::cout << minimisation_objective(curve, caps, optimal_params) << std::endl;

    const int trials = 100;
    double temperature = 50;
    int seed = 0;
    int max_iter = 50000;
    //double score_threshold = 0.0005;
    //double score_threshold = 4e-5;
    double score_threshold = 3.5e-5;
    double score_reduction_ratio = 2;
    std::vector<std::vector<double>> param_sets;
    std::vector<double> scores;
    std::pair<std::vector<double>, double> simulated_annealing_results;

    for (int i = 0; i < trials; i++) {
        //auto start = std::chrono::high_resolution_clock::now();
        simulated_annealing_results = simulated_annealing(curve, caps, minimisation_objective, bounds, jump_stds, 0.995, max_iter, temperature, score_threshold, seed, true);
        //auto stop = std::chrono::high_resolution_clock::now();
        //const std::chrono::duration<double, std::micro> execution_time = stop - start;
        //std::cout << "Time: " << execution_time.count() << " microseconds." << std::endl;
        if (simulated_annealing_results.second < score_threshold) {
            if (simulated_annealing_results.first[1] / simulated_annealing_results.first[0] > simulated_annealing_results.first[3] / simulated_annealing_results.first[2]) {
                double temp0 = simulated_annealing_results.first[0], temp1 = simulated_annealing_results.first[1];
                simulated_annealing_results.first[0] = simulated_annealing_results.first[2];
                simulated_annealing_results.first[1] = simulated_annealing_results.first[3];
                simulated_annealing_results.first[2] = temp0;
                simulated_annealing_results.first[3] = temp1;
            }
            param_sets.push_back(simulated_annealing_results.first);
            scores.push_back(simulated_annealing_results.second);

            std::cout << i+1 << "/" << trials << " [";
            for (int i = 0; i < 5; i++) {
                std::cout << simulated_annealing_results.first[i];
                if (i != 4) { std::cout << ","; }
            }
            std::cout << "] : " << simulated_annealing_results.second << std::endl;
        }
        seed++;
    }
    std::tuple<double, double, double> param_statistics;
    double l, u, std;
    while (param_sets.size() > 5) {
        std::cout << "-------------------------" << std::endl << "New parameter range: " << std::endl;
        // Adjust parameter range and jump magnitudes
        for (int i = 0; i < 5; i++) {
            std::vector<double> param_range;
            for (int j = 0; j < param_sets.size(); j++) {
                param_range.push_back(param_sets[j][i]);
            }
            param_statistics = vector_statistics(param_range);
            l = std::get<0>(param_statistics);
            u = std::get<1>(param_statistics);
            std = std::get<2>(param_statistics);
            if (l - 2*std > bounds[i].first) {
                bounds[i].first = l - 2*std;
            }
            if (u + 2*std < bounds[i].second) {
                bounds[i].second = u + 2*std;
            }
            std::cout << l << ", " << u << ", " << std << ", [" << bounds[i].first << ", " << bounds[i].second << "]" << std::endl << std::endl;
        }
        for (int i = 0; i < 5; i++) {
            jump_stds[i] = (bounds[i].second - bounds[i].first) / 20;
        }
        temperature = score_threshold * 10;
        score_threshold /= score_reduction_ratio;
        std::cout << "Target: " << score_threshold << std::endl;
        std::vector<std::vector<double>> new_param_sets;
        std::vector<double> new_scores;
        for (int i = 0; i < trials; i++) {
            if (i < param_sets.size()) {
                simulated_annealing_results = simulated_annealing(curve, caps, minimisation_objective, bounds, jump_stds, 0.995, max_iter, temperature,
                    score_threshold, seed, true, param_sets[i]);
            }
            else {
                simulated_annealing_results = simulated_annealing(curve, caps, minimisation_objective, bounds, jump_stds, 0.995, max_iter, temperature,
                    score_threshold, seed, true);
            }
            if (simulated_annealing_results.second < score_threshold) {
                new_param_sets.push_back(simulated_annealing_results.first);
                new_scores.push_back(simulated_annealing_results.second);
                std::cout << i+1 << "/" << param_sets.size() << " [";
                for (int j = 0; j < 5; j++) {
                    std::cout << simulated_annealing_results.first[j];
                    if (j != 4) { std::cout << ","; }
                }
                if (i < param_sets.size()) {
                    std::cout << "] : " << scores[i] << " -> " << simulated_annealing_results.second << std::endl;
                }
                else {
                    std::cout << "] : " << simulated_annealing_results.second << std::endl;
                }
            }
            seed++;
        }
        param_sets = new_param_sets;
        scores = new_scores;
    }
    for (int i = 0; i < param_sets.size(); i++) {
        std::cout << std::endl << " [";
        for (int j = 0; j < 5; j++) {
            std::cout << param_sets[i][j];
            if (j != 4) { std::cout << ","; }
        }
        std::cout << "] : " << scores[i] << std::endl;
    }
    return EXIT_SUCCESS;
}
