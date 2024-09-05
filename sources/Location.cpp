
#include "Location.hpp"

Location::Location(/* args */)
{
}

Location::~Location()
{
}

void Location::setPath(const std::string& path)
{
	_path = path;
}
void Location::setMethods(const std::string& methods)
{
	std::istringstream stream(methods);
	std::string method;

	while (stream >> method)
	{
		if (method == GET)
			_methods.push_back(GET);
		else if (method == POST)
			_methods.push_back(POST);
		else if (method == DELETE)
			_methods.push_back(DELETE);
		else
			throw std::runtime_error("Error method not implemanted.");
	}
	
}
void Location::setErrorCode(const std::string& methods)
{

}

std::string& Location::getPath()
{

}
std::vector<httpRequestMethod>& Location::getMethods()
{

}
std::map<HttpStatusCode, std::string>& Location::getError()
{
	
}