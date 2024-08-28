#include <string>
#include <iostream> 

int main(int ac, char **av) {
    if (ac != 2) {
        std::cout << "Binary must be: ./webserv [configuration file]" << std::endl;
        return (1);
    }
}