NAME            = webserv

SRCS_LIST       = main.cpp HttpRequest.cpp

SRCS_FOLDER     = sources

SRCS            = $(addprefix $(SRCS_FOLDER)/, $(SRCS_LIST))

OBJS            = $(SRCS:.cpp=.o)

INCLUDES        = -I includes

CC              = clang++
CFLAGS           = -Wall -Wextra -Werror -std=c++98 -pthread
RM              = rm -f

all:            $(NAME)

$(NAME):        $(OBJS)
		$(CC) $(CFLAGS) $(INCLUDES) $(OBJS) -o $(NAME)

$(SRCS_FOLDER)/%.o: $(SRCS_FOLDER)/%.cpp
		$(CC) $(CFLAGS) $(INCLUDES) -o $@ -c $<

clean:
		$(RM) $(OBJS)

fclean:         clean
		$(RM) $(NAME)

re:             fclean all

.PHONY:         all fclean clean re
