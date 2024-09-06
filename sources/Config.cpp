#include "Config.hpp"

Config::Config(std::ifstream &originalCofig)
{
	std::string line;
	std::string str;

	while (getline(originalCofig, line))
		str.append(line);
	_location = new std::map<std::string, Location>();
	_strConfig = str;
}

Config::~Config()
{
	if (_location != NULL)
		delete _location;

}

const std::map<std::string, Location>& Config::getLocation() const
{
	return(*_location);
}

const std::string& Config::getFdConfig() const
{ 
	return(_strConfig);
}