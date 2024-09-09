
#ifndef CONFIG_HPP
# define CONFIG_HPP
# include "Location.hpp"
# include <iostream>
# include <fstream>
 
class Config
{
	private:
		std::map<std::string, Location>* _location;
		std::ifstream _config;

	public:
		Config(std::ifstream &originalCofig);
		~Config();
		Config(const Config &other);
		Config& operator=(const Config &other);

		const std::map<std::string, Location>& getLocation() const;
		std::ifstream& getFdConfig();
		const std::ifstream& getFdConfig() const;
};

#endif