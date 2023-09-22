NAME=webserv
CXX=g++
CXXFLAGS+=-Wall -Wextra -Wreorder -I .

CORE=core/main.cpp

LIBHTTP=libhttp/Headers.cpp libhttp/Request.cpp libhttp/Reader.cpp \
				 libhttp/Request-target.cpp libhttp/Chunk.cpp libhttp/MultipartFormData.cpp \
				 libhttp/Post_handler.cpp
LIBHTTP_TEST=libhttp/Headers_test.cpp libhttp/Reader_test.cpp

LIBPARSE=libparse/TestParser.cpp libparse/ReadFile.cpp \
				 libparse/utilities.cpp libparse/Lexer.cpp libparse/Parser.cpp libparse/matching-location.cpp libparse/Types.cpp
LIBNET=libnet/Net.cpp libnet/Session.cpp libnet/SessionState.cpp

CXXFILES=$(CORE) $(LIBHTTP) $(LIBHTTP_TEST) $(LIBPARSE) $(LIBNET)
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
