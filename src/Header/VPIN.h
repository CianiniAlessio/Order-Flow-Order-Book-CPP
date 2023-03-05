#ifndef VPIN_H
#define VPIN_H

#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <string>
#include <random>
#include "Statistic.h"
class BUCKET
{
public:
	BUCKET()
	{

	}
	std::vector<double> prices_buckets, prices_deltas;
	std::vector<double> volume_imbalance;
	double averaged_price{ 0 }, quantity_total{ 0 }, quantity_sell{ 0 }, quantity_buy{ 0 }, threshold_size{ 0 }, minimum_quantity{ 0 };
	int windows_size{ 20 }, second_window_size{ 0 };
	
	void update_average_price_and_volume(double price_new, double quantity_new)
	{
		//averaged_price = (averaged_price* quantity_total + price_new * quantity_new) / (quantity_total + quantity_new);
		quantity_total += quantity_new;
		averaged_price = price_new;
	}
	void update_volume_imbalance()
	{
		if (prices_buckets.size() > 2)
		{
			int window = (windows_size > prices_deltas.size()) ? prices_deltas.size() : windows_size;
			double _mean = Statistic::calculate_mean(prices_deltas, window, second_window_size);
			double _std_dev = Statistic::calculate_stddev(prices_deltas, window, second_window_size,_mean);
			double price_now = prices_buckets.back();
			double price_before = prices_buckets[prices_buckets.size() - 2];
			double ratio = (price_now - price_before)/_std_dev;
			quantity_buy = threshold_size * Statistic::cumulative_distribution_function(ratio, _mean, _std_dev);
			quantity_sell = threshold_size - quantity_buy;
			volume_imbalance.push_back((quantity_sell - quantity_buy));
		}
		
	}
	double sum_orders_imbalance()
	{
		double sum{ 0 };
		if (volume_imbalance.size() < windows_size) 
		{
			for (int i = 0; i < volume_imbalance.size(); i++) 
			{
				sum += std::abs(volume_imbalance[volume_imbalance.size() - i - 1]);
			}
		}
		else
		{
			for (int i = windows_size - 1; i >= 0; i--)
			{
				sum += std::abs(volume_imbalance[volume_imbalance.size() - i - 1]);
			}
		}			
		return sum;
	}
	void restore_bucket()
	{
		averaged_price = 0;
		quantity_total = 0 ;
		quantity_sell = 0 ; 
		quantity_buy = 0;
		
	}
	
};
class VPIN {
public:
	VPIN(double bucket_size, int window_size, int second_window_size, double minimum_quantity);
	double get_vpin();
	double get_cdf();
	std::vector<double> get_cdfs();
	std::vector<double> get_vpins();
	std::vector<double> get_both_cdf_vpin();
	bool update(const std::vector<std::string>& entry);

private:
	BUCKET _bucket;
	double mean,std_dev;
	std::vector<double > vpins;
	std::vector<double > cdfs;

};

#endif