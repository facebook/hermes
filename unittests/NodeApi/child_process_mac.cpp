#include "child_process.h"

#include <cerrno>
#include <fcntl.h>
#include <spawn.h>
#include <sys/wait.h>
#include <unistd.h>
#include <cassert>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

// Verify the condition.
// - If true, resume execution.
// - If false, print a message to stderr and exit the app with exit code 1.
#ifndef VerifyElseExit
#define VerifyElseExit(condition)                                              \
  do {                                                                         \
    if (!(condition)) {                                                        \
      ExitOnError(#condition, nullptr);                                        \
    }                                                                          \
  } while (false)
#endif

// Verify the condition.
// - If true, resume execution.
// - If false, destroy the passed `posix_spawn_file_actions_t* actions`, then
// print a message to stderr and exit the app with exit code 1.
#ifndef VerifyElseExitWithCleanup
#define VerifyElseExitWithCleanup(condition, actions_ptr)                      \
  do {                                                                         \
    if (!(condition)) {                                                        \
      ExitOnError(#condition, actions_ptr);                                    \
    }                                                                          \
  } while (false)
#endif

extern char** environ;

namespace node_api_tests {

namespace {

std::string ReadFromFd(int fd);
void ExitOnError(const char* message, posix_spawn_file_actions_t* actions);

}  // namespace

ProcessResult SpawnSync(std::string_view command,
                        std::vector<std::string> args) {
  ProcessResult result{};

  // These int arrays each comprise two file descriptors: { readEnd, writeEnd }.
  int stdout_pipe[2], stderr_pipe[2];
  VerifyElseExit(pipe(stdout_pipe) == 0);
  VerifyElseExit(pipe(stderr_pipe) == 0);

  posix_spawn_file_actions_t actions;
  VerifyElseExit(posix_spawn_file_actions_init(&actions) == 0);

  VerifyElseExitWithCleanup(posix_spawn_file_actions_adddup2(
                                &actions, stdout_pipe[1], STDOUT_FILENO) == 0,
                            &actions);
  VerifyElseExitWithCleanup(posix_spawn_file_actions_adddup2(
                                &actions, stderr_pipe[1], STDERR_FILENO) == 0,
                            &actions);

  VerifyElseExitWithCleanup(
      posix_spawn_file_actions_addclose(&actions, stdout_pipe[0]) == 0,
      &actions);
  VerifyElseExitWithCleanup(
      posix_spawn_file_actions_addclose(&actions, stderr_pipe[0]) == 0,
      &actions);

  std::vector<char*> argv;
  argv.push_back(strdup(std::string(command).c_str()));
  for (const std::string& arg : args) {
    argv.push_back(strdup(arg.c_str()));
  }
  argv.push_back(nullptr);

  pid_t pid;
  VerifyElseExitWithCleanup(
      posix_spawnp(&pid, argv[0], &actions, nullptr, argv.data(), environ) == 0,
      &actions);

  posix_spawn_file_actions_destroy(&actions);

  // Close the write ends of the pipes.
  close(stdout_pipe[1]);
  close(stderr_pipe[1]);

  int wait_status;
  pid_t waited_pid;
  do {
    waited_pid = waitpid(pid, &wait_status, 0);
  } while (waited_pid == -1 && errno == EINTR);

  VerifyElseExit(waited_pid == pid);

  if (WIFEXITED(wait_status)) {
    result.status = WEXITSTATUS(wait_status);
  } else if (WIFSIGNALED(wait_status)) {
    result.status = 128 + WTERMSIG(wait_status);
  } else {
    result.status = 1;
  }
  result.std_output = ReadFromFd(stdout_pipe[0]);
  result.std_error = ReadFromFd(stderr_pipe[0]);

  // Close the read ends of the pipes.
  close(stdout_pipe[0]);
  close(stderr_pipe[0]);

  for (char* arg : argv) {
    free(arg);
  }

  return result;
}

namespace {

std::string ReadFromFd(int fd) {
  std::string result;
  constexpr size_t bufferSize = 4096;
  char buffer[bufferSize];
  ssize_t bytesRead;
  while (true) {
    bytesRead = read(fd, buffer, bufferSize);
    if (bytesRead > 0) {
      result.append(buffer, bytesRead);
      continue;
    }

    if (bytesRead == 0) {
      break;
    }

    if (errno == EINTR) {
      continue;
    }

    ExitOnError("read", nullptr);
  }
  return result;
}

// Format a readable error message, print it to console, and exit from the
// application.
void ExitOnError(const char* message, posix_spawn_file_actions_t* actions) {
  int err = errno;
  const char* err_msg = strerror(err);

  fprintf(stderr, "%s failed with error %d: %s\n", message, err, err_msg);

  if (actions != nullptr) {
    posix_spawn_file_actions_destroy(actions);
  }

  exit(1);
}

}  // namespace
}  // namespace node_api_tests
