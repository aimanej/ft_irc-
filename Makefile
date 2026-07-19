CC= c++

Flags= -Wall -Wextra -Werror

Srcs= server.cpp client.cpp

Headers= client.hpp

Name= server

all:$(Name)

$(Name): $(Srcs) $(Headers)
	$(CC)  $(Srcs) -o $(Name)

clean:
	rm -rf $(Name)

fclean: clean