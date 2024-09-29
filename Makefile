NAME			= webserv

SRCS_LIST	= 	main.cpp HttpRequest.cpp HttpResponse.cpp 

SRCS_FOLDER		= sources

SRCS			= $(addprefix ${SRCS_FOLDER}/, ${SRCS_LIST})

OBJS			= ${SRCS:.cpp=.o}

INCLUDES		= -I includes/Config -I includes/Utils -I includes/HTTP

CC				= clang++
CFLAGS 			= -Wall -Wextra -Werror -std=c++98 -pthread
RM				= rm -f

all:				$(NAME)

$(NAME):		$(OBJS)
						$(CC) $(CFLAGS) $(INCLUDES) $(OBJS) -o $(NAME)

%.o: %.cpp
						${CC} ${CFLAGS} $(INCLUDES) -o $@ -c $<

clean:
						${RM} ${OBJS}

fclean:			clean
						${RM} ${NAME}

re:					fclean all

.PHONY: 		all fclean clean re