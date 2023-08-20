NAME=w
CXX=g++-11
CXXFLAGS+=-Wall -Wextra -Wreorder -I . -I/usr/include/c++/11 -I/usr/include/x86_64-linux-gnu/c++/11 -L /usr/lib/gcc/x86_64-linux-gnu/11

WEBSERV=webserv/main.cpp

LIBHTTP=libhttp/Headers.cpp libhttp/Request.cpp libhttp/Reader.cpp libhttp/Uri.cpp
LIBHTTP_TEST=libhttp/Headers_test.cpp libhttp/Reader_test.cpp

LIBNET=libnet/Net.cpp libnet/Session.cpp libnet/SessionState.cpp

CXXFILES=$(WEBSERV) $(LIBHTTP) $(LIBHTTP_TEST) $(LIBNET)
OBJFILES=$(patsubst %.cpp, %.o, $(CXXFILES))


all: $(NAME)

$(NAME): $(OBJFILES)
	@mkdir -p build
	$(CXX) $(CXXFLAGS) $(OBJFILES) -o $(NAME)

debug: CXXFLAGS += -ggdb
debug: $(NAME)

fsanitize: CXXFLAGS += -fsanitize=address
fsanitize: debug

bear: compile_commands.json

compile_commands.json:
	bear -- make

clean:
	rm -rf $(OBJFILES)

fclean: clean
	rm -rf $(NAME)

build: all clean

re: clean all

.PHONY: all re clean fclean build
