#include "Statistic.h"
#include <cmath>

double Statistic::cumulative_distribution_function(double& x, double mu, double sigma) 
{
    x = (x - mu) / sigma;
    auto cd = 0.5 * std::erfc((-x*sqrt(2)));
    return cd;
}

double Statistic::calculate_mean(const std::vector<double>& vector, int& window_size, int& second_window) {
    double sum = 0.0;
    int n = vector.size();

    // Compute the starting index for the last window
    int start_index = std::max(n - window_size - second_window, 0);

    // Sum the last window_size elements
    for (int i = start_index; i < n - second_window; i++) {
        sum += vector[i];
    }

    return sum / (n - start_index - second_window);
}

double Statistic::calculate_stddev(const std::vector<double>& vector, int& window_size, int& second_window, double& mean) 
{
    double sum_sq = 0.0;
    int n = vector.size();


    // Compute the starting index for the last window
    int start_index = std::max(n - window_size - second_window, 0);

    // Sum the squares of the deviations from the mean for the last window_size elements
    for (int i = start_index; i < n - second_window; i++) {
        sum_sq += pow(vector[i] - mean, 2);
    }

    return sqrt(sum_sq / (n - start_index - second_window - 1));
}
