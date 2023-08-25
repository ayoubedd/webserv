NAME=webserv
CXX=g++
CXXFLAGS+=-Wall -Wextra -Wreorder -I .

WEBSERV=webserv/main.cpp

LIBHTTP=libhttp/Headers.cpp libhttp/Request.cpp libhttp/Reader.cpp libhttp/Uri.cpp libhttp/MultipartFormData.cpp
LIBHTTP_TEST=libhttp/Headers_test.cpp libhttp/Reader_test.cpp

LIBPARSE=libparse/TestParser.cpp libparse/ReadFile.cpp \
				 libparse/utilities.cpp libparse/Lexer.cpp libparse/Parser.cpp

LIBNET=libnet/Net.cpp libnet/Session.cpp libnet/SessionState.cpp

CXXFILES=$(WEBSERV) $(LIBHTTP) $(LIBHTTP_TEST) $(LIBPARSE) $(LIBNET)
OBJFILES=$(patsubst %.cpp, %.o, $(CXXFILES))


all: $(NAME)

$(NAME): $(OBJFILES)
	@mkdir -p build
	$(CXX) $(CXXFLAGS) $(OBJFILES) -o build/$(NAME)

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
	rm -rf build/$(NAME)

build: all clean

re: clean all

.PHONY: all re clean fclean build
