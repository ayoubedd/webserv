#include "libcgi/Cgi.hpp"
#include <cstring>
#include <sys/stat.h>
#include <unistd.h>

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

void delete2d(char **env) {
  char *i;

  for (i = *env; i; i++)
    delete[] i;
  delete[] env;
}

libcgi::Cgi::Cgi(std::string scriptPath, libhttp::Request *httpReq, sockaddr_in *clientAddr)
    : httpReq(httpReq)
    , scriptPath(scriptPath)
    , clientAddr(clientAddr) {}

libcgi::Cgi::error libcgi::Cgi::init(std::string serverName, std::string scriptName,
                                     std::string localReqPath, std::string serverPort,
                                     std::string protocol, std::string serverSoftware) {
  struct stat s;

  this->cgiReq.init(serverName, scriptName, localReqPath, serverPort, protocol, serverSoftware);
  if (::stat(this->scriptPath.c_str(), &s) != 0)
    return FAILED_OPEN_FILE;
  if (!(s.st_mode & S_IXUSR))
    return FAILED_OPEN_FILE;
  this->cgiReq.build(this->httpReq);
  return OK;
}

std::pair<libcgi::Cgi::error, libcgi::CgiResult> libcgi::Cgi::exec() {
  char **env, **argv;
  int    fd[2];
  int    pid;

  if (::pipe(fd) < 0)
    return std::make_pair(FAILED_OPEN_FILE, CgiResult{.pid = -1, .fd = -1});
  if (this->httpReq->body.size() > 0) {
    int err;

    err = ::write(fd[1], &this->httpReq->body.front(), this->httpReq->body.size());
    if (err < 0) {
      close(fd[0]);
      close(fd[1]);
      return std::make_pair(FAILED_WRITE, CgiResult{.pid = -1, .fd = -1});
    }
  }
  pid = fork();
  if (pid == 0) {
    dup2(0, fd[0]);
    dup2(1, fd[1]);
    argv = getScriptArgs(this->scriptPath);
    env = headersAsEnv(cgiReq.env);
    execve(this->scriptPath.c_str(), argv, env);
    delete2d(argv);
    delete2d(env);
    // write to the fd the error
    exit(1);
  } else if (pid > 0) {
    close(fd[1]);
    return std::make_pair(OK, CgiResult{.pid = pid, .fd = fd[0]});
  }
  close(fd[0]);
  close(fd[1]);
  return std::make_pair(FAILED_FORK, CgiResult{.pid = -1, .fd = -1});
}
