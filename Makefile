NAME = webserv

SRCS_FOLDER = sources
INCLUDES_FOLDER = includes

SRCS_LIST = main.cpp \
            $(addprefix $(SRCS_FOLDER)/, Client.cpp Config.cpp \
			HttpRequest.cpp HttpResponse.cpp Location.cpp \
			Server.cpp Socket.cpp Utils.cpp)

SRCS = $(SRCS_LIST)

OBJS = $(SRCS:.cpp=.o)

CC = clang++
CFLAGS = -Wall -Wextra -Werror -std=c++98 -pthread
INCLUDES = -I $(INCLUDES_FOLDER)

RM = rm -f

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) $(OBJS) -o $(NAME)

%.o: %.cpp
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	$(RM) $(OBJS)

fclean: clean
	$(RM) $(NAME)

re: fclean all

.PHONY: all clean fclean re
