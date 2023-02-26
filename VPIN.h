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
	std::vector<double> prices_buckets;
	std::vector<double> volume_imbalance;
	double averaged_price{ 0 }, quantity_averaged{ 0 }, quantity_sell{ 0 },
		quantity_buy{ 0 }, threshold_size{ 0 };
	int windows_size{ 20 }, second_window_size{ 10 };
	
	void update_average_price_and_volume(double price_new, double quantity_new)
	{
		//averaged_price = (averaged_price*quantity_averaged + price_new * quantity_new) / (quantity_averaged + quantity_new);
		quantity_averaged += quantity_new;
		//averaged_price = price_new;
	}
	void update_volume_imbalance()
	{
		//if (volume_imbalance.size() > 1)
		//{
			//double _mean = Statistic::calculate_mean(prices_buckets, windows_size);
			//double std_dev = Statistic::calculate_stddev(prices_buckets, windows_size);
			//double price_now = prices_buckets.back();
			//double price_before = prices_buckets[prices_buckets.size() - 2];
			//double ratio = (price_now - price_before);
			//quantity_buy = threshold_size * Statistic::cumulative_distribution_function(ratio);
			//quantity_sell = threshold_size - quantity_buy;
			volume_imbalance.push_back((quantity_buy - quantity_sell) / (quantity_buy + quantity_sell));
		//}
		
	}
	double sum_orders_imbalance()
	{
		double sum{ 0 };
		for (int i = 1; i < windows_size; i++)
		{
			if (i < volume_imbalance.size())
			{
				sum += std::abs(volume_imbalance[volume_imbalance.size() - i]);
			}
		}
			
		return sum;
	}
	void restore_bucket()
	{
		averaged_price = 0;
		quantity_averaged = 0 ; 
		quantity_sell = 0 ; 
		quantity_buy = 0;
		
	}
	
};
class VPIN {
public:
	VPIN(double bucket_size, int window_size, int second_window_size);
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