CC = c++
CFLAG = -Wall -Wextra -Werror -std=c++98
SRC = Irc.cpp Server.cpp Client.cpp
HEADER = Server.hpp Client.hpp
NAME =	ircserv
OBJ = $(SRC:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $(CFLAG) $(OBJ) -o $(NAME)

%.o: %.cpp $(HEADER)
	$(CC) $(CFLAG) -c $< -o $@

clean:
	rm -rf $(OBJ)

fclean: clean
	rm -rf $(NAME)

re: fclean all