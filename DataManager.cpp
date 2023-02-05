#include "DataManager.h"

DataManager::DataManager() {}

void DataManager::split_csv_order_book(
	std::string& line_in,
    std::vector<std::string>& line_out,
	char tosplit
)
{
	std::string topush;

	for(int i= 0; i< line_in.size();++i)
	{
		if (line_in[i] != tosplit)
		{
			topush.push_back(line_in[i]);
		}
		else
		{
			
            line_out.push_back(topush);
            topush.clear();
		}
		if (i == line_in.size() - 1 && topush != "")
		{
            line_out.push_back(topush);
            topush.clear();
		}
	}
};

void DataManager::read_order_book(std::string fileName,std::map<char, std::map<double, double>>& order_book)
{
    std::ifstream file(fileName);
    std::string line_in;
    std::vector<std::string> line_out;
    while (std::getline(file, line_in)) {
        DataManager::split_csv_order_book(line_in,line_out,',');
        try
        {
            OrderBook::SetPriceQuantity(order_book,std::stod(line_out[PositionsData::PRICE]),std::stod(line_out[PositionsData::QTY]),line_out[PositionsData::SIDE][0]);
        }
        catch(std::exception& e){
            for (int i = 0; i < line_out.size(); i++)
            {
                std::cout << i << " " <<  line_out[i] << " ";
            }
            std::cout << e.what() << std::endl;
        }
            
    	line_out.clear();
        
    }
    
    file.close();
};