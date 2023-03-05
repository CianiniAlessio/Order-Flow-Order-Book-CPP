#include "VPIN.h"


#pragma region GETTER SETTER

	VPIN::VPIN(double _bucket_size, int _windows_size, int _second_window_size, double _minimum_quantity)
		: vpins{ 0.0 }, cdfs{ 0.0 }
	{
		_bucket.threshold_size = _bucket_size;
		_bucket.windows_size = _windows_size;
		_bucket.second_window_size = _second_window_size;
		_bucket.minimum_quantity = _minimum_quantity;
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
		
		if (quantity*price > _bucket.minimum_quantity)
		{
			_bucket.quantity_total += quantity;

			/*
			if (entry[5] == "false")
			{
				_bucket.quantity_buy += quantity;
			}
			else if (entry[5] == "true")
			{
				_bucket.quantity_sell += quantity;
			}*/
		}
		

		_bucket.update_average_price_and_volume(price, quantity);

		if (_bucket.quantity_total >= _bucket.threshold_size) {


			//prices_deltas
			_bucket.prices_buckets.push_back(_bucket.averaged_price);
			
			
			// behaviour like a queue like this, TO CHECK 
//			if(_bucket.prices_buckets.size() > _bucket.window_size) _bucket.prices_buckets.erase(_bucket.prices_buckets.begin());

			
			if(_bucket.prices_buckets.size() > 1)_bucket.prices_deltas.push_back(_bucket.averaged_price - _bucket.prices_buckets[_bucket.prices_buckets.size()-2]);
			_bucket.update_volume_imbalance();		


			if (_bucket.volume_imbalance.size() >= _bucket.windows_size )
			{
				auto a = _bucket.sum_orders_imbalance();
				auto b = (_bucket.windows_size > _bucket.volume_imbalance.size()) ? _bucket.volume_imbalance.size() : _bucket.windows_size;
				auto c = _bucket.threshold_size;
				vpins.push_back( a/ (b *c ));
				mean = 0;
				std_dev = 1;
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
