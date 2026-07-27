CC= c++

Flags= -Wall -Wextra -Werror

Srcs= main.cpp client.cpp channel.cpp server.cpp

Headers= client.hpp

Name= server

all:$(Name)

$(Name): $(Srcs) $(Headers)
	$(CC)  $(Srcs) -o $(Name)

clean:
	rm -rf $(Name)

fclean: clean