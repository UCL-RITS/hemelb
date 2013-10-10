// 
// Copyright (C) University College London, 2007-2012, all rights reserved.
// 
// This file is part of HemeLB and is CONFIDENTIAL. You may not work 
// with, install, use, duplicate, modify, redistribute or share this
// file, or any part thereof, other than as allowed by any agreement
// specifically made by you with University College London.
// 

#include "net/NetworkTopology.h"
#include "net/mpi.h"
/*
 * NOTE!
 *
 * Changes to this file may need to be made in concert with changes to
 * SingleMachineNetworkTopology.cc
 *
 * Only one of these two files will be compiled on any given build, and
 * they determine whether our topology will be over multiple machines or
 * not.
 */

namespace hemelb
{
  namespace net
  {
    /*!
     If one has more than one machine. The topology discovery mechanism is implemented in this function
     */
    void NetworkTopology::InitialiseMachineInfo()
    {
      int err;
      int *depth, **color;
      int machine_id, flag, is_found;
      int i, j, sum;

      depths = 0;
      net::MpiCommunicator& commWorld = net::MpiCommunicator::World();
      HEMELB_MPI_CALL(MPI_Comm_get_attr, (commWorld, MPICHX_TOPOLOGY_DEPTHS, &depth, &flag));

      if (flag == 0)
        throw Exception() << "MPICHX_TOPOLOGY_DEPTHS attribute missing";

      HEMELB_MPI_CALL(MPI_Comm_get_attr, (commWorld, MPICHX_TOPOLOGY_COLORS, &color, &flag));

      if (flag == 0)
        throw Exception() << "MPICHX_TOPOLOGY_COLORS attribute missing";

      machineCount = 0;

      MachineIdOfEachProc = new int[GetProcessorCount()];
      ProcCountOnEachMachine = new int[GetProcessorCount()];

      for (i = 0; i < GetProcessorCount(); i++)
      {
        ProcCountOnEachMachine[i] = 0;
      }
      for (i = 0; i < GetProcessorCount(); i++)
      {
        if (depth[i] != 4)
          continue;

        depths = max(depths, depth[i]);

        for (j = 0, is_found = 0; j < machineCount && is_found == 0; j++)
        {
          if (color[i][3] == MachineIdOfEachProc[j])
          {
            is_found = 1;
            ++ProcCountOnEachMachine[MachineIdOfEachProc[j]];
          }
        }
        if (is_found == 1)
          continue;

        MachineIdOfEachProc[machineCount] = color[i][3];
        ++ProcCountOnEachMachine[machineCount];
        ++machineCount;
      }
      machineCount = max(1, machineCount);

      if (machineCount == 1)
      {
        for (i = 0; i < GetProcessorCount(); i++)
        {
          MachineIdOfEachProc[i] = 0;
        }
        ProcCountOnEachMachine[0] = GetProcessorCount();
      }
      else
      {
        for (i = 0; i < GetProcessorCount(); i++)
        {
          sum = 0;
          machine_id = 0;

          is_found = 0;

          while (!is_found)
          {
            if (sum + ProcCountOnEachMachine[machine_id] > i)
            {
              is_found = 1;
              continue;
            }
            sum += ProcCountOnEachMachine[machine_id];
            ++machine_id;
          }
          MachineIdOfEachProc[i] = machine_id;
        }
      }
    }
  }
}
