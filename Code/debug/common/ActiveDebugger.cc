// 
// Copyright (C) University College London, 2007-2012, all rights reserved.
// 
// This file is part of HemeLB and is CONFIDENTIAL. You may not work 
// with, install, use, duplicate, modify, redistribute or share this
// file, or any part thereof, other than as allowed by any agreement
// specifically made by you with University College London.
// 

#include <string>
#include <sstream>
#include <iostream>
#include <vector>

#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <cstdio>

#include <cstdarg>

#include <unistd.h>
#include <sys/wait.h>

#include "net/mpi.h"

#include "debug/common/ActiveDebugger.h"

namespace hemelb
{
  namespace debug
  {

    ActiveDebugger::ActiveDebugger(const char* const executable, const net::MpiCommunicator& comm) :
      Debugger(executable, comm), mAmAttached(false), mPIds()
    {
    }

    std::string ActiveDebugger::ConvertIntToString(int i)
    {
      // Convert an int to a string.
      // Remember you will have to delete it!
      std::stringstream ss;
      ss << i;
      return ss.str();
    }

    void ActiveDebugger::BreakHere(void)
    {
      return;
    }

    void ActiveDebugger::Print(const char* iFormat, ...)
    {
      std::va_list args;
      va_start(args, iFormat);
      std::vprintf(iFormat, args);
      va_end(args);
    }

    void ActiveDebugger::Attach(void)
    {
      // Start up a the debuggers, tell them to attach to the
      // processes and wait for them to attach. This function forks
      // another process on the rank 0 task and waits for it.
      if (mAmAttached)
        return;

      volatile int amWaiting = 1;

      int childPid = -1;
      // To rank 0
      GatherProcessIds();

      if (mCommunicator.Rank() == 0)
      {
        childPid = fork();
        // Fork gives the PID of the child to the parent and zero to the child
        if (childPid == 0)
        {
          // This function won't return.
          SpawnDebuggers();
        }
      } // if rank 0

      // Sit here waiting for GDB to attach
      while (amWaiting)
        sleep(5);

      if (mCommunicator.Rank() == 0)
      {
        // Reap the spawner
        int deadPid = waitpid(childPid, NULL, 0);
      }

      mAmAttached = true;
    }

    void ActiveDebugger::GatherProcessIds()
    {
      int rank = mCommunicator.Rank();
      int nProcs = mCommunicator.Size();

      int pId = getpid();

      mPIds = mCommunicator.Gather(pId, 0);
    }

    void ActiveDebugger::SpawnDebuggers(void)
    {
      // Run the script to Tell the OS to start appropriate
      // terminal(s), launch the debuggers and attach them to our
      // processes.  We're the child, so we are DOOMED.  (Either to
      // exec() or if that fails exit().)
      std::string srcFile(__FILE__);
      std::string debugCommonDir = srcFile.substr(0, srcFile.rfind('/'));

      std::string binaryPath(getcwd(NULL, 0)); // This will leak,
      // but don't care
      binaryPath += "/";
      binaryPath += mExecutable;

      VoS args;

      args.push_back(GetPlatformInterpreter());

      args.push_back(GetPlatformScript());

      // Get the GDB script to exec
      // This will either be the value of the environment variable
      // HEMELB_DEBUG_SCRIPT or resume.gdb
      args.push_back(GetPlatformGdbScript());

      //args.push_back(GetEnvironmentDebugScript());

      args.push_back(binaryPath);

      for (VoI::iterator i = mPIds.begin(); i < mPIds.end(); ++i)
      {
        // This leaks memory
        args.push_back(ConvertIntToString(*i));
      }

      // +1 to include required NULL pointer for execvp()
      char **argv = new char *[args.size() + 1];

      // convert to C array of char arrays.
      for (unsigned int i = 0; i < args.size(); ++i)
      {
        argv[i] = new char[args[i].length() + 1]; // for terminating null
        std::strcpy(argv[i], args[i].c_str());
      }

      // Terminating NULL
      argv[args.size()] = NULL;

      // Exec to replace hemelb with osascript
      int code = execvp(argv[0], argv);

      // OK- that didn't work if we get here, better die (since we're
      // the extra process). Print the error code too.
      std::cerr << "Couldn't exec() script to launch debuggers, error code " << errno << std::endl;
      // Now print the command we wanted to exec()
      for (VoS::iterator it = args.begin(); it < args.end(); it++)
      {
        std::cerr << *it << " ";
      }
      std::cerr.flush();
      // Die
      exit(1);
    }

  } // namespace debug
} // namespace hemelb
