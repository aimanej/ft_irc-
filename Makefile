CC= c++

Flags= -std=c++98 -Wall -Wextra -Werror

Srcs= main.cpp client.cpp channel.cpp server.cpp

Headers= client.hpp

Name= server

all:$(Name)

$(Name): $(Srcs) $(Headers)
	$(CC)  $(Srcs) -o $(Name)

clean:
	rm -rf $(Name)

fclean: clean

re: clean all