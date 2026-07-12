#ifndef NON_INHERITING_PROCESS_HPP__
#define NON_INHERITING_PROCESS_HPP__

#include <QProcess>
#include "pimpl_h.hpp"

class QObject;

//
// class NonInheritingProcess - Manage a process without it inheriting
//                              all inheritable handles
//
//   On MS  Windows QProcess  creates sub-processes which  inherit all
// inheritable  handles, and  handles  on Windows  are inheritable  by
// default. This can cause the  lifetime of objects to be unexpectedly
// extended, which in turn can cause unexpected errors. The motivation
// for this class  was implementing log file rotation  using the Boost
// log library.  The  current log file's handle gets  inherited by any
// long  running sub-process  started by  QProcess and  that causes  a
// sharing  violation  when  attempting  to rename  the  log  file  on
// rotation, even though  the log library closes the  current log file
// before trying to rename it.
//
class NonInheritingProcess
  : public QProcess
{
public:
  NonInheritingProcess (QObject * parent = nullptr);
  ~NonInheritingProcess ();

protected:
#ifdef Q_OS_LINUX
  // Runs in the child, right after fork() and before exec() on Linux.
  // Sets PR_SET_PDEATHSIG so the kernel kills this child automatically
  // if wsjtx dies without a chance to clean up (crash, kill -9, power
  // loss) -- the Linux equivalent of the Windows Job Object below, so
  // jt9 doesn't outlive an abnormal exit. PR_SET_PDEATHSIG is Linux-
  // specific (not available via macOS/BSD's more general Q_OS_UNIX).
  void setupChildProcess () override;
#endif

private:
  class impl;
  pimpl<impl> m_;
};
#endif
