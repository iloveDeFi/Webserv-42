
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
		std::string	_path;
		std::vector<httpRequestMethod>	_methods;
		std::map<HttpStatusCode, std::string> _Error;

	public:
		Location(/* args */);
		~Location();
		Location(const Location &other) = delete;
		Location& operator=(const Location &other) = delete;

		void setPath(const std::string& path);
		void setMethods(const std::string& methods);
		void setErrorCode(const std::string& methods);

		std::string& getPath();
		std::vector<httpRequestMethod>& getMethods();
		std::map<HttpStatusCode, std::string>& getError();

};

#endif