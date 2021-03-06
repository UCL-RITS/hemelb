// 
// Copyright (C) University College London, 2007-2012, all rights reserved.
// 
// This file is part of HemeLB and is CONFIDENTIAL. You may not work 
// with, install, use, duplicate, modify, redistribute or share this
// file, or any part thereof, other than as allowed by any agreement
// specifically made by you with University College London.
// 

#ifndef HEMELB_GEOMETRY_BLOCK_H
#define HEMELB_GEOMETRY_BLOCK_H

#include "units.h"
#include <vector>

namespace hemelb
{
  namespace geometry
  {
    // Data about each global block in the lattice,
    // site_data[] is an array containing individual lattice site data
    // within a global block.
    class Block
    {
      public:
        Block();
        Block(site_t sitesPerBlock);

        ~Block();

        bool IsEmpty() const;

        proc_t GetProcessorRankForSite(site_t localSiteIndex) const;
        site_t GetLocalContiguousIndexForSite(site_t localSiteIndex) const;
        bool SiteIsSolid(site_t localSiteIndex) const;

        void SetProcessorRankForSite(site_t localSiteIndex, proc_t rank);
        void SetLocalContiguousIndexForSite(site_t localSiteIndex, site_t localContiguousIndex);

      private:
        // An array of the ranks on which each lattice site within the block resides.
        std::vector<proc_t> processorRankForEachBlockSite;

        // The local index for each site on the block in the LocalLatticeData.
        std::vector<site_t> localContiguousIndex;

        // Constant for the id assigned to any solid sites.
        static const site_t SOLID_SITE_ID;
    };
  }
}

#endif
