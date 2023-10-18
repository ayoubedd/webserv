#include "libcgi/Cgi.hpp"
#include "libhttp/Multipart.hpp"
#include "libhttp/MultipartFormData.hpp"
#include "libnet/SessionState.hpp"
#include <assert.h>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <signal.h>
#include <sstream>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <utility>

const std::string libcgi::Cgi::blueprint = "/tmp/webserv/cgi/";

ssize_t doesContainerHasBuff(const char *raw, size_t rLen, const char *ptr, size_t pLen) {
  for (size_t i = 0; rLen - i > pLen; i++) {
    if (!strncmp(&raw[i], ptr, strlen(ptr)))
      return i;
  }
  return -1;
}

libcgi::Cgi::~Cgi(void) { clean(); }

std::string asStr(int fd) {
  std::stringstream ss;

  ss << fd;
  return ss.str();
};

std::pair<libcgi::Cgi::Error, libcgi::Cgi::State> libcgi::Cgi::handleCgiBuff(char  *ptr,
                                                                             size_t len) {
  ssize_t        idx;
  const char    *del = "\r\n\r\n";
  Respons::error err;

  if (this->state == READING_HEADERS) {
    idx = doesContainerHasBuff(ptr, len, del, strlen(del));
    if (idx == -1) {
      this->res.cgiHeader.insert(this->res.cgiHeader.end(), ptr, ptr + len);
      return std::make_pair(OK, this->state);
    }
    this->state = READING_BODY;
    this->res.cgiHeader.insert(this->res.cgiHeader.end(), ptr, ptr + idx + 2); // plus the \r\n
    err = this->res.build();
    if (err != Respons::OK)
      return std::make_pair(MALFORMED, this->state);

    this->res.sockBuff->insert(this->res.sockBuff->end(), del, del + 2);
    this->res.write(ptr + idx + 4, len - idx - 4);
    return std::make_pair(OK, this->state);
  }
  this->res.write(ptr, len);
  return std::make_pair(OK, this->state);
}

char *keyAndValAsStr(const std::string &key, const std::string &val) {
  char *keyAndVal;

  keyAndVal = new char[key.size() + val.size() + 2];
  std::memcpy(keyAndVal, key.c_str(), key.size());
  keyAndVal[key.size()] = '=';
  std::memcpy(keyAndVal + key.size() + 1, val.c_str(), val.size());
  keyAndVal[key.size() + val.size() + 1] = 0;
  return keyAndVal;
}

char **headersAsEnv(std::map<std::string, std::string> &headers) {
  char                                             **env;
  std::map<std::string, std::string>::const_iterator it;
  std::map<std::string, std::string>::const_iterator end;
  std::map<std::string, std::string>::size_type      i;

  env = new char *[headers.size() + 1];
  i = 0;
  it = headers.begin();
  end = headers.end();
  for (; i < headers.size(); i++, it++) {
    env[i] = keyAndValAsStr(it->first, it->second);
  }
  env[i] = NULL;
  return env;
}

char **getScriptArgs(std::string &scriptPath) {
  char **argv;

  argv = new char *[2];

  argv[0] = ::strdup(scriptPath.c_str());
  argv[1] = NULL;
  return argv;
}

char **getScriptArgs(const std::string &interpreter, const std::string &scriptPath) {
  char **argv;

  argv = new char *[3];

  argv[0] = ::strdup(interpreter.c_str());
  argv[1] = ::strdup(scriptPath.c_str());
  argv[2] = NULL;
  return argv;
}

void delete2d(char **env) {
  unsigned int i;

  for (i = 0; env[i]; i++)
    delete[] env[i];
  delete[] env;
}

libcgi::Cgi::Cgi(sockaddr_in *clientAddr, size_t bufferSize)
    : clientAddr(clientAddr)
    , req()
    , state(INIT)
    , pid(-1)
    , bodySize(0)
    , bufferSize(bufferSize)
    , cgiInput(-1) {
  fd[0] = -1;
  fd[1] = -1;
}

libcgi::Cgi::Error libcgi::Cgi::init(libhttp::Request *httpReq, std::string scriptPath,
                                     std::string serverName, std::string localReqPath,
                                     std::string serverPort, std::string protocol,
                                     std::string serverSoftware) {
  std::string::size_type i;
  std::string            scriptName;

  if (::pipe(fd) < 0)
    return FAILED_OPEN_PIPE;
  this->cgiInputFileName = libhttp::generateFileName(this->blueprint + "input");
  cgiInput = open(this->cgiInputFileName.c_str(), O_RDWR | O_CREAT, 0644);
  if (cgiInput < 0)
    return FAILED_OPEN_FILE;
  i = scriptPath.rfind('/');
  if (i == std::string::npos)
    scriptName = scriptPath;
  else
    scriptName = scriptPath.substr(i + 1, scriptPath.size() - i);

  this->req.init(scriptPath, serverName, scriptName, localReqPath, serverPort, protocol,
                 serverSoftware);
  this->req.build(httpReq);
  this->res.init();
  return OK;
}

libcgi::Cgi::Error libcgi::Cgi::write(std::vector<char> &body) {
  ssize_t len;

  len = ::write(this->cgiInput, &body.front(), body.size());
  if (len < 0)
    return FAILED_WRITE;
  this->bodySize += len;
  return OK;
};

libcgi::Cgi::Error libcgi::Cgi::exec(const std::string &interpreter) {
  char **env, **argv;

  lseek(this->cgiInput, 0, SEEK_SET);
  pid = fork();
  if (pid == 0) {
    dup2(this->cgiInput, STDIN_FILENO);
    dup2(this->fd[1], STDOUT_FILENO);

    close(fd[0]);
    close(fd[1]);
    close(cgiInput);

    interpreter.empty() ? argv = getScriptArgs(req.scriptPath)
                        : argv = getScriptArgs(interpreter, req.scriptPath);
    env = headersAsEnv(req.env);
    ::execve(argv[0], argv, env);
    exit(1);
  } else if (pid > 0) {
    this->state = READING_HEADERS;

    close(fd[1]);
    fd[1] = -1;

    close(cgiInput);
    cgiInput = -1;
    return OK;
  }
  return FAILED_FORK;
}

libcgi::Cgi::Error libcgi::Cgi::read() {
  char                    buff[bufferSize];
  ssize_t                 len;
  std::pair<Error, State> newState;

  if (this->state == INIT)
    this->state = READING_HEADERS;
  len = ::read(this->fd[0], buff, sizeof buff);
  if (len == 0) {
    int         status = 0;
    std::string end = "0\r\n\r\n";
    this->res.sockBuff->insert(this->res.sockBuff->end(), end.begin(), end.end());
    this->state = FIN;
    if (waitpid(this->pid, &status, 0) <= 0)
      return FAILED_WAITPID;
    this->pid = -1;
    if (status != 0)
      return CHIIED_RETURN_ERR;
    return OK;
  }
  if (len < 0) {
    if (waitpid(this->pid, NULL, 0) > 0) {
      pid = -1;
    }
    return FAILED_READ;
  }

  newState = this->handleCgiBuff(buff, len);
  if (newState.first != OK)
    return MALFORMED;
  return OK;
}

void libcgi::Cgi::clean() {
  // Pipes cleanup
  if (fd[0] != -1) {
    close(fd[0]);
    fd[0] = -1;
  }
  if (fd[1]) {
    close(fd[1]);
    fd[1] = -1;
  }

  // Input file clanup
  if (cgiInput != -1) {
    close(cgiInput);
    cgiInput = -1;
  }
  std::remove(cgiInputFileName.c_str());
  cgiInputFileName.clear();

  // Orphant processes catch
  if (pid != -1) {
    kill(pid, SIGKILL);
    waitpid(pid, NULL, 0);
    pid = -1;
  }

  if (state != libcgi::Cgi::READING_BODY && state != libcgi::Cgi::FIN)
    delete res.sockBuff;
  res.clean();

  req.clean();
  bodySize = 0;
  state = INIT;
}
