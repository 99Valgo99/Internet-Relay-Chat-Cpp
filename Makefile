CC = c++
CFLAG = -Wall -Wextra -Werror -std=c++98
SRC = Irc.cpp Server/Server.cpp Client/Client.cpp Channel/Channel.cpp Commands/Validators.cpp \
		Commands/Handlers.cpp Commands/Senders.cpp Dispatchers/DispatchInit.cpp Dispatchers/DispatchJoin.cpp Dispatchers/DispatchPrvMsg.cpp \
		Dispatchers/DispatchKick.cpp Dispatchers/DispatchTopic.cpp \
		Dispatchers/DispatchInvite.cpp Dispatchers/DispatchMode.cpp
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