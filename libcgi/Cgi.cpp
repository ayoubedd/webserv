#include "libcgi/Cgi.hpp"
#include <assert.h>
#include <cstring>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#define assertm(ex, msg)                                                                           \
  (ex == false ? ({                                                                                \
    log(msg);                                                                                      \
    assert(ex);                                                                                    \
  })                                                                                               \
               : (assert(ex)))
#define log(msg) printf("%s\n", msg);

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
  unsigned int i;

  for (i = 0; env[i]; i++)
    delete[] env[i];
  delete[] env;
}

libcgi::Cgi::Cgi(libhttp::Request *httpReq, std::string scriptPath, sockaddr_in *clientAddr)
    : httpReq(httpReq)
    , scriptPath(scriptPath)
    , clientAddr(clientAddr)
    , req()
    , fd{-1, -1}
    , pid(-1)
    , bodySize(0)
    , state(libnet::CGI_INIT) {}

libcgi::Cgi::error libcgi::Cgi::init(std::string serverName, std::string scriptName,
                                     std::string localReqPath, std::string serverPort,
                                     std::string protocol, std::string serverSoftware) {
  struct stat s;

  this->req.init(serverName, scriptName, localReqPath, serverPort, protocol, serverSoftware);
  if (::stat(this->scriptPath.c_str(), &s) != 0)
    return FAILED_OPEN_FILE;
  if (!(s.st_mode & S_IXUSR))
    return FAILED_EXEC_PERM;
  this->req.build(this->httpReq);

  if (::pipe(fd) < 0)
    return FAILED_OPEN_PIPE;

  return OK;
}

libcgi::Cgi::error libcgi::Cgi::write(std::vector<char> &body) {
  int len;

  len = ::write(this->fd[1], &body.front(), body.size());
  if (len < 0)
    return FAILED_WRITE;
  this->bodySize += body.size();
  return OK;
};

libcgi::Cgi::error libcgi::Cgi::exec() {
  char **env, **argv;

  pid = fork();
  if (pid == 0) {
    dup2(0, fd[0]);
    dup2(1, fd[1]);
    argv = getScriptArgs(this->scriptPath);
    env = headersAsEnv(req.env);
    ::execve(this->scriptPath.c_str(), argv, env);
    delete2d(argv);
    delete2d(env);
    std::cerr << "child err" << std::endl;
    // write to the fd the error
    exit(1);
  } else if (pid > 0) {
    return OK;
  }
  return FAILED_FORK;
}

libcgi::Cgi::error libcgi::Cgi::read() {
  char    buff[4096 * 2];
  ssize_t len;

  len = ::read(this->fd[0], buff, sizeof buff);
  if (len < 0)
    return FAILED_READ;
  buff[len] = 0;

  std::cerr << "this is running" << std::endl;
  // std::cout << buff << std::endl;
  return OK;
}

void libcgi::Cgi::clean() {
  close(this->fd[0]);
  close(this->fd[1]);
  waitpid(this->pid, 0, 0); // this will block add WNOHANG
}
