#ifndef STATISTIC_H
#define STATISTIC_H

#include <vector>

class Statistic {
public:
    static double cumulative_distribution_function(double& x, double mu, double sigma);
    static double calculate_mean(const std::vector<double>& vector, int& window_size, int& second_window);
    static double calculate_stddev(const std::vector<double>& vector, int& window_size, int& second_window, double& mean);
};

#endif // STATISTIC_H
