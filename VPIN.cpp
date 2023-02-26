#include "VPIN.h"


#pragma region GETTER SETTER

	VPIN::VPIN(double _bucket_size, int _windows_size, int _second_window_size) : vpins{ 0.0 }, cdfs{ 0.0 }
	{
		_bucket.threshold_size = _bucket_size;
		_bucket.windows_size = _windows_size;
		_bucket.second_window_size = _windows_size;
	};
	double VPIN::get_cdf()
	{
		return cdfs.back();
	}
	double VPIN::get_vpin()
	{
		return vpins.back();
	}
	std::vector<double> VPIN::get_cdfs()
	{
		return cdfs;
	}
	std::vector<double> VPIN::get_vpins()
	{
		return vpins;
	}
	std::vector<double> VPIN::get_both_cdf_vpin()
	{
		std::vector<double> ret;
		ret.push_back(get_cdf());
		ret.push_back(get_vpin());
		return ret;
	}

#pragma endregion

#pragma region FUNCTION FOR UPDATE


	bool VPIN::update(const std::vector<std::string>& entry) {
		double price = std::stod(entry[1]);
		double quantity = std::stod(entry[2]);

		if (entry[5] == "false")
		{
			_bucket.quantity_buy += quantity;
		}
		else if (entry[5] == "true")
		{
			_bucket.quantity_sell += quantity;
		}

		_bucket.update_average_price_and_volume(price, quantity);

		if (_bucket.quantity_averaged >= _bucket.threshold_size) {


			//_bucket.prices_buckets.push_back(_bucket.averaged_price);
			_bucket.update_volume_imbalance();		

			// DO VPIN E CDF
			if (_bucket.volume_imbalance.size() > 1)
			{
				vpins.push_back(_bucket.sum_orders_imbalance() / (_bucket.volume_imbalance.size() * _bucket.threshold_size));
				mean = Statistic::calculate_mean(vpins, _bucket.windows_size, _bucket.second_window_size);
				std_dev = Statistic::calculate_stddev(vpins, _bucket.windows_size, _bucket.second_window_size, mean);
				cdfs.push_back(Statistic::cumulative_distribution_function(vpins.back(),mean, std_dev));
				_bucket.restore_bucket();
				return true;				
			}
			else
			{
				_bucket.restore_bucket();
				return false;
			}

		}
	
		return false;
	}


#pragma endregion