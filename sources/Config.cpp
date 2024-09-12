#include "Config.hpp"

Config::Config(std::ifstream &originalCofig)
{
	std::string line;
	std::string str;
	std::ofstream config;

	config.open("config.txt");
	if (!config)
		throw std::runtime_error("Error creating config file.");
	_locations = new std::vector<std::map<std::string, Location> >();
	while (getline(originalCofig, line))
	{
		config << line << "\n";
		std::istringstream lineStream(line);
		parseLocation(lineStream);
/* 		str.append(line);
		str.append("\n"); */
	}
	
	//std::cout << str << std::endl;
	config.close();
	_config.open("config.txt");
}

void Config::parseLocation(std::istringstream &configLine)
{
	std::string pair;
	Location newLoc;
	std::map<std::string, Location> newMap;
	while (getline(configLine, pair, ' '))
	{
		size_t delimiterPos = pair.find(':');
		if (delimiterPos == std::string::npos)
			throw std::runtime_error("Invalid configuration format.");
			
		std::string key = pair.substr(0, delimiterPos);
		std::string value = pair.substr(delimiterPos + 1);

		if (key == "root")
		{
			newLoc.setPath(value);
			newMap[key] = newLoc;
		}
		//faire le reste
	}
	_locations->push_back(newMap);
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