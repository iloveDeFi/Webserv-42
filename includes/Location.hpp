
#ifndef LOCATION_HPP
# define LOCATION_HPP
# include <iostream>
# include <vector>
# include <map>
# include "Socket.hpp"
# include "Client.hpp"
# include "Server.hpp"
# include "Utils.hpp"

class Location
{
	private:
		std::string	_root;
		std::string _index;
		std::vector<httpRequestMethod>	_methods;
		std::map<HttpStatusCode, std::string>	_return;
		std::map<int, std::string> _errorPages;
		std::string	_cgiPath;

	public:
		Location(/* args */);
		~Location();

		
};

#endif