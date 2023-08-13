FROM gcc:latest

WORKDIR /webserv
EXPOSE 5550
ENV HOST=0.0.0.0

RUN apt update
RUN apt install gdb fish vim tmux python3 -y

RUN git clone https://github.com/rohanrhu/gdb-frontend.git /gdbf

CMD "fish"
