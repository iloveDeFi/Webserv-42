#pragma once
#ifndef CONFIG_HPP
# define CONFIG_HPP
# include "Location.hpp"
# include <iostream>
# include <fstream>
 
class Config
{
	private:
		std::vector<std::map<std::string, Location> >* _locations;
		std::ifstream _config;
		void parseLocation(std::istringstream &configLine);

	public:
		Config(std::ifstream &originalConfig);
		~Config();
		Config(const Config &other);
		Config& operator=(const Config &other);

		const std::vector<std::map<std::string, Location> >& getLocations() const;
		std::ifstream& getFdConfig();
		const std::ifstream& getFdConfig() const;
};

#endif
