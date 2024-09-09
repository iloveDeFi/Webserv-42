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
	_location = new std::map<std::string, Location>();
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
		_location = other._location;
	return (*this);
}

Config::~Config()
{
	if (_location != NULL)
		delete _location;
	_config.close();
}

const std::map<std::string, Location>& Config::getLocation() const
{
	return(*_location);
}

std::ifstream& Config::getFdConfig()
{ 
	return(_config);
}

const std::ifstream& Config::getFdConfig() const
{ 
	return(_config);
}