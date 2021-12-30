/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "InternalBindings.h"
#include "hermes/Support/OSCompat.h"
#include "hermes/hermes.h"

#include <fcntl.h>
#include "uv.h"

using namespace facebook;

#define DEFINE_CONSTANT(target, runtime, name)     \
  do {                                             \
    target.setProperty(runtime, #name, (int)name); \
  } while (0)

/// Defines signal constants.
static jsi::Object signalConstants(jsi::Runtime &runtime) {
  jsi::Object signalsObject{runtime};
#ifdef SIGHUP
  DEFINE_CONSTANT(signalsObject, runtime, SIGHUP);
#endif

#ifdef SIGINT
  DEFINE_CONSTANT(signalsObject, runtime, SIGINT);
#endif

#ifdef SIGQUIT
  DEFINE_CONSTANT(signalsObject, runtime, SIGQUIT);
#endif

#ifdef SIGILL
  DEFINE_CONSTANT(signalsObject, runtime, SIGILL);
#endif

#ifdef SIGTRAP
  DEFINE_CONSTANT(signalsObject, runtime, SIGTRAP);
#endif

#ifdef SIGABRT
  DEFINE_CONSTANT(signalsObject, runtime, SIGABRT);
#endif

#ifdef SIGIOT
  DEFINE_CONSTANT(signalsObject, runtime, SIGIOT);
#endif

#ifdef SIGBUS
  DEFINE_CONSTANT(signalsObject, runtime, SIGBUS);
#endif

#ifdef SIGFPE
  DEFINE_CONSTANT(signalsObject, runtime, SIGFPE);
#endif

#ifdef SIGKILL
  DEFINE_CONSTANT(signalsObject, runtime, SIGKILL);
#endif

#ifdef SIGUSR1
  DEFINE_CONSTANT(signalsObject, runtime, SIGUSR1);
#endif

#ifdef SIGSEGV
  DEFINE_CONSTANT(signalsObject, runtime, SIGSEGV);
#endif

#ifdef SIGUSR2
  DEFINE_CONSTANT(signalsObject, runtime, SIGUSR2);
#endif

#ifdef SIGPIPE
  DEFINE_CONSTANT(signalsObject, runtime, SIGPIPE);
#endif

#ifdef SIGALRM
  DEFINE_CONSTANT(signalsObject, runtime, SIGALRM);
#endif

  DEFINE_CONSTANT(signalsObject, runtime, SIGTERM);

#ifdef SIGCHLD
  DEFINE_CONSTANT(signalsObject, runtime, SIGCHLD);
#endif

#ifdef SIGSTKFLT
  DEFINE_CONSTANT(signalsObject, runtime, SIGSTKFLT);
#endif

#ifdef SIGCONT
  DEFINE_CONSTANT(signalsObject, runtime, SIGCONT);
#endif

#ifdef SIGSTOP
  DEFINE_CONSTANT(signalsObject, runtime, SIGSTOP);
#endif

#ifdef SIGTSTP
  DEFINE_CONSTANT(signalsObject, runtime, SIGTSTP);
#endif

#ifdef SIGBREAK
  DEFINE_CONSTANT(signalsObject, runtime, SIGBREAK);
#endif

#ifdef SIGTTIN
  DEFINE_CONSTANT(signalsObject, runtime, SIGTTIN);
#endif

#ifdef SIGTTOU
  DEFINE_CONSTANT(signalsObject, runtime, SIGTTOU);
#endif

#ifdef SIGURG
  DEFINE_CONSTANT(signalsObject, runtime, SIGURG);
#endif

#ifdef SIGXCPU
  DEFINE_CONSTANT(signalsObject, runtime, SIGXCPU);
#endif

#ifdef SIGXFSZ
  DEFINE_CONSTANT(signalsObject, runtime, SIGXFSZ);
#endif

#ifdef SIGVTALRM
  DEFINE_CONSTANT(signalsObject, runtime, SIGVTALRM);
#endif

#ifdef SIGPROF
  DEFINE_CONSTANT(signalsObject, runtime, SIGPROF);
#endif

#ifdef SIGWINCH
  DEFINE_CONSTANT(signalsObject, runtime, SIGWINCH);
#endif

#ifdef SIGIO
  DEFINE_CONSTANT(signalsObject, runtime, SIGIO);
#endif

#ifdef SIGPOLL
  DEFINE_CONSTANT(signalsObject, runtime, SIGPOLL);
#endif

#ifdef SIGLOST
  DEFINE_CONSTANT(signalsObject, runtime, SIGLOST);
#endif

#ifdef SIGPWR
  DEFINE_CONSTANT(signalsObject, runtime, SIGPWR);
#endif

#ifdef SIGINFO
  DEFINE_CONSTANT(signalsObject, runtime, SIGINFO);
#endif

#ifdef SIGSYS
  DEFINE_CONSTANT(signalsObject, runtime, SIGSYS);
#endif

#ifdef SIGUNUSED
  DEFINE_CONSTANT(signalsObject, runtime, SIGUNUSED);
#endif
  return signalsObject;
}

/// Defines system constants.
static jsi::Object systemConstants(jsi::Runtime &runtime) {
  jsi::Object fsObject{runtime};

  DEFINE_CONSTANT(fsObject, runtime, UV_FS_SYMLINK_DIR);
  DEFINE_CONSTANT(fsObject, runtime, UV_FS_SYMLINK_JUNCTION);
  // file access modes
  DEFINE_CONSTANT(fsObject, runtime, O_RDONLY);
  DEFINE_CONSTANT(fsObject, runtime, O_WRONLY);
  DEFINE_CONSTANT(fsObject, runtime, O_RDWR);

  // file types from readdir
  DEFINE_CONSTANT(fsObject, runtime, UV_DIRENT_UNKNOWN);
  DEFINE_CONSTANT(fsObject, runtime, UV_DIRENT_FILE);
  DEFINE_CONSTANT(fsObject, runtime, UV_DIRENT_DIR);
  DEFINE_CONSTANT(fsObject, runtime, UV_DIRENT_LINK);
  DEFINE_CONSTANT(fsObject, runtime, UV_DIRENT_FIFO);
  DEFINE_CONSTANT(fsObject, runtime, UV_DIRENT_SOCKET);
  DEFINE_CONSTANT(fsObject, runtime, UV_DIRENT_CHAR);
  DEFINE_CONSTANT(fsObject, runtime, UV_DIRENT_BLOCK);

  DEFINE_CONSTANT(fsObject, runtime, S_IFMT);
  DEFINE_CONSTANT(fsObject, runtime, S_IFREG);
  DEFINE_CONSTANT(fsObject, runtime, S_IFDIR);
  DEFINE_CONSTANT(fsObject, runtime, S_IFCHR);
#ifdef S_IFBLK
  DEFINE_CONSTANT(fsObject, runtime, S_IFBLK);
#endif

#ifdef S_IFIFO
  DEFINE_CONSTANT(fsObject, runtime, S_IFIFO);
#endif

#ifdef S_IFLNK
  DEFINE_CONSTANT(fsObject, runtime, S_IFLNK);
#endif

#ifdef S_IFSOCK
  DEFINE_CONSTANT(fsObject, runtime, S_IFSOCK);
#endif

#ifdef O_CREAT
  DEFINE_CONSTANT(fsObject, runtime, O_CREAT);
#endif

#ifdef O_EXCL
  DEFINE_CONSTANT(fsObject, runtime, O_EXCL);
#endif

#ifdef O_NOCTTY
  DEFINE_CONSTANT(fsObject, runtime, O_NOCTTY);
#endif

#ifdef O_TRUNC
  DEFINE_CONSTANT(fsObject, runtime, O_TRUNC);
#endif

#ifdef O_APPEND
  DEFINE_CONSTANT(fsObject, runtime, O_APPEND);
#endif

#ifdef O_DIRECTORY
  DEFINE_CONSTANT(fsObject, runtime, O_DIRECTORY);
#endif

#ifdef O_EXCL
  DEFINE_CONSTANT(fsObject, runtime, O_EXCL);
#endif

#ifdef O_NOATIME
  DEFINE_CONSTANT(fsObject, runtime, O_NOATIME);
#endif

#ifdef O_NOFOLLOW
  DEFINE_CONSTANT(fsObject, runtime, O_NOFOLLOW);
#endif

#ifdef O_SYNC
  DEFINE_CONSTANT(fsObject, runtime, O_SYNC);
#endif

#ifdef O_DSYNC
  DEFINE_CONSTANT(fsObject, runtime, O_DSYNC);
#endif

#ifdef O_SYMLINK
  DEFINE_CONSTANT(fsObject, runtime, O_SYMLINK);
#endif

#ifdef O_DIRECT
  DEFINE_CONSTANT(fsObject, runtime, O_DIRECT);
#endif

#ifdef O_NONBLOCK
  DEFINE_CONSTANT(fsObject, runtime, O_NONBLOCK);
#endif

#ifdef S_IRWXU
  DEFINE_CONSTANT(fsObject, runtime, S_IRWXU);
#endif

#ifdef S_IRUSR
  DEFINE_CONSTANT(fsObject, runtime, S_IRUSR);
#endif

#ifdef S_IWUSR
  DEFINE_CONSTANT(fsObject, runtime, S_IWUSR);
#endif

#ifdef S_IXUSR
  DEFINE_CONSTANT(fsObject, runtime, S_IXUSR);
#endif

#ifdef S_IRWXG
  DEFINE_CONSTANT(fsObject, runtime, S_IRWXG);
#endif

#ifdef S_IRGRP
  DEFINE_CONSTANT(fsObject, runtime, S_IRGRP);
#endif

#ifdef S_IWGRP
  DEFINE_CONSTANT(fsObject, runtime, S_IWGRP);
#endif

#ifdef S_IXGRP
  DEFINE_CONSTANT(fsObject, runtime, S_IXGRP);
#endif

#ifdef S_IRWXO
  DEFINE_CONSTANT(fsObject, runtime, S_IRWXO);
#endif

#ifdef S_IROTH
  DEFINE_CONSTANT(fsObject, runtime, S_IROTH);
#endif

#ifdef S_IWOTH
  DEFINE_CONSTANT(fsObject, runtime, S_IWOTH);
#endif

#ifdef S_IXOTH
  DEFINE_CONSTANT(fsObject, runtime, S_IXOTH);
#endif

#ifdef F_OK
  DEFINE_CONSTANT(fsObject, runtime, F_OK);
#endif

#ifdef R_OK
  DEFINE_CONSTANT(fsObject, runtime, R_OK);
#endif

#ifdef W_OK
  DEFINE_CONSTANT(fsObject, runtime, W_OK);
#endif

#ifdef X_OK
  DEFINE_CONSTANT(fsObject, runtime, X_OK);
#endif

#ifdef UV_FS_COPYFILE_EXCL
#define COPYFILE_EXCL UV_FS_COPYFILE_EXCL
  DEFINE_CONSTANT(fsObject, runtime, UV_FS_COPYFILE_EXCL);
  DEFINE_CONSTANT(fsObject, runtime, COPYFILE_EXCL);
#undef COPYFILE_EXCL
#endif

#ifdef UV_FS_COPYFILE_FICLONE
#define COPYFILE_FICLONE UV_FS_COPYFILE_FICLONE
  DEFINE_CONSTANT(fsObject, runtime, UV_FS_COPYFILE_FICLONE);
  DEFINE_CONSTANT(fsObject, runtime, COPYFILE_FICLONE);
#undef COPYFILE_FICLONE
#endif

#ifdef UV_FS_COPYFILE_FICLONE_FORCE
#define COPYFILE_FICLONE_FORCE UV_FS_COPYFILE_FICLONE_FORCE
  DEFINE_CONSTANT(fsObject, runtime, UV_FS_COPYFILE_FICLONE_FORCE);
  DEFINE_CONSTANT(fsObject, runtime, COPYFILE_FICLONE_FORCE);
#undef COPYFILE_FICLONE_FORCE
#endif
  return fsObject;
}

/// Adds the 'constants' object as a property of internalBinding. Defines the
/// constants needed by fs.js and related js files.
jsi::Value facebook::constantsBinding(RuntimeState &rs) {
  jsi::Runtime &rt = rs.getRuntime();
  jsi::Object constants{rt};

  constants.setProperty(
      rt, jsi::String::createFromAscii(rt, "fs"), systemConstants(rt));

  jsi::Object os{rt};
  os.setProperty(
      rt, jsi::String::createFromAscii(rt, "signals"), signalConstants(rt));
  constants.setProperty(rt, jsi::String::createFromAscii(rt, "os"), os);

  rs.setInternalBindingProp("constants", std::move(constants));
  return rs.getInternalBindingProp("constants");
}
