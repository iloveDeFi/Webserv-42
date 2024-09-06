#include "Config.hpp"

Config::Config(ifstream originalCofig)
{
	std::string line;
	std::string str;

	while (getline(config, line))
		str.append(line);
	_location = NULL;
	_strConfig = str;
}

Config::~Config()
{
}

Location& getLocation() { return(_location) }

std::string& getFdConfig() { return(_fdConfig) }