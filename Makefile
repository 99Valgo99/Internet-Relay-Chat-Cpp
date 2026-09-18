CC = c++
CFLAG = -Wall -Wextra -Werror -std=c++98
SRC = Irc.cpp Server/Server.cpp Client/Client.cpp Channel/Channel.cpp Commands/Validators.cpp \
		Commands/Handlers.cpp Commands/Senders.cpp
HEADER = Server/Server.hpp Client/Client.hpp Channel/Channel.hpp
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