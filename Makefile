NAME=webserv
CXX=g++
CXXFLAGS+=-Wall -Wextra -Werror -Wreorder -std=c++98 -I .

CORE=core/main.cpp core/Sanitizer.cpp core/Multiplexer.cpp \
		 core/Initialization.cpp core/Logger.cpp \
		 core/Timer.cpp

LIBHTTP=libhttp/Headers.cpp libhttp/Request.cpp libhttp/Reader.cpp \
				 libhttp/Request-target.cpp libhttp/Chunk.cpp libhttp/MultipartFormData.cpp \
				 libhttp/Writer.cpp libhttp/Response.cpp libhttp/Post.cpp \
				 libhttp/Sized-Post.cpp libhttp/Methods.cpp libhttp/Redirect.cpp \
				 libhttp/Error-generate.cpp
LIBHTTP_TEST=libhttp/Headers_test.cpp libhttp/Reader_test.cpp


LIBPARSE=libparse/TestParser.cpp libparse/ReadFile.cpp \
				 libparse/utilities.cpp libparse/Lexer.cpp libparse/Parser.cpp libparse/match.cpp \
				 libparse/Types.cpp

LIBNET=libnet/Net.cpp libnet/Session.cpp libnet/Terminator.cpp

LIBCGI=libcgi/Cgi-req.cpp libcgi/Cgi.cpp libcgi/Cgi-res.cpp

CXXFILES=$(CORE) $(LIBHTTP) $(LIBHTTP_TEST) $(LIBPARSE) $(LIBNET) $(LIBCGI)
OBJFILES=$(patsubst %.cpp, %.o, $(CXXFILES))



all: $(NAME)

$(NAME): $(OBJFILES)
	@mkdir -p build
	$(CXX) $(CXXFLAGS) $(OBJFILES) -o $(NAME)

clean:
	rm -rf $(OBJFILES)

fclean: clean
	rm -rf $(NAME)

re: clean all

.PHONY: all re clean fclean build
