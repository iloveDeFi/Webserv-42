#include "Config.hpp"

Config::Config(std::ifstream &originalCofig)
{
	std::string line;
	std::string str;
	std::ofstream config;

	config.open("config.txt");
	if (!config)
		throw std::runtime_error("Error creating config file.");
	while (getline(originalCofig, line))
	{
		config << line << "\n";
/* 		str.append(line);
		str.append("\n"); */
	}
	_locations = new std::vector<std::map<std::string, Location> >();
	//std::cout << str << std::endl;
	config.close();
	_config.open("config.txt");
}

Config::Config(const Config &other)
{
	(void)other;
}

Config& Config::operator=(const Config &other)
{
	if (this != &other)
		_locations = other._locations;
	return (*this);
}

Config::~Config()
{
/* 	for (std::vector<std::map<std::string, Location>>::iterator \
	it = _locations->begin();  it != _locations->end(); it++) */
	delete _locations;
	_config.close();
}

const std::vector<std::map<std::string, Location> >& Config::getLocations() const
{
	return(*_locations);
}

std::ifstream& Config::getFdConfig()
{ 
	return(_config);
}

const std::ifstream& Config::getFdConfig() const
{ 
	return(_config);
}