NAME=webserv
CXX=g++
CXXFLAGS+=-Wall -Wextra -Wreorder -std=c++98 -I .

WEBSERV=webserv/main.cpp

LIBHTTP=libhttp/Headers.cpp libhttp/Request.cpp libhttp/Reader.cpp libhttp/Uri.cpp
LIBHTTP_TEST=libhttp/Headers_test.cpp libhttp/Reader_test.cpp

# LIBNET=libnet/Net.cpp libnet/Session.cpp

CXXFILES=$(WEBSERV) $(LIBHTTP) $(LIBHTTP_TEST) $(LIBNET)
OBJFILES=$(patsubst %.cpp, %.o, $(CXXFILES))


all: $(NAME)

$(NAME): $(OBJFILES)
	@mkdir -p build
	$(CXX) $(CXXFLAGS) $(OBJFILES) -o build/$(NAME)

debug: CXXFLAGS += -g
debug: $(NAME)

fsanitize: CXXFLAGS += -fsanitize=address
fsanitize: debug

bear: compile_commands.json

compile_commands.json:
	bear -- make

clean:
	rm -rf $(OBJFILES)

fclean: clean
	rm -rf build/$(NAME)

build: all clean

re: clean all

.PHONY: all re clean fclean build
