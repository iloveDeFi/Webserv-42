
#ifndef CONFIG_HPP
# define CONFIG_HPP
# include "Location.hpp"
# include <iostream>
# include <fstream>

class Config
{
	private:
		std::map<std::string, Location>* _location;
		std::string _strConfig;

	public:
		Config(std::ifstream &originalCofig);
		~Config();

		const std::map<std::string, Location>& getLocation() const;
		const std::string& getFdConfig() const;
};

#endif