#ifndef HEMELB_COLLOIDS_PARTICLESET_H
#define HEMELB_COLLOIDS_PARTICLESET_H

#include <vector>
#include "geometry/LatticeData.h"
#include "io/xml/XmlAbstractionLayer.h"
#include "lb/MacroscopicPropertyCache.h"
#include "mpiInclude.h"
#include "colloids/Particle.h"
#include "topology/NetworkTopology.h"

namespace hemelb
{
  namespace colloids
  {
    /** represents the set of all particles known to the local process */
    class ParticleSet
    {
      public:
        /** constructor - gets local particle information from xml config file */
        ParticleSet(const geometry::LatticeData& latDatLBM,
                    io::xml::XmlAbstractionLayer& xml,
                    lb::MacroscopicPropertyCache& propertyCache,
                    std::vector<proc_t>& neighbourProcessors);

        /** destructor - de-allocates all Particle objects created by this Set */
        ~ParticleSet();

        /** updates the position of each particle using body forces and fluid velocity */
        const void UpdatePositions();

        /** calculates the effect of all body forces on each particle */
        const void CalculateBodyForces();

        /** calculates the effects of all particles on each lattice site */
        const void CalculateFeedbackForces();

        /** interpolates the fluid velocity to the location of each particle */
        const void InterpolateFluidVelocity();

        /** communicates the positions of all particles to&from all neighbours */
        const void CommunicateParticlePositions();

        /** communicates the partial fluid interpolations to&from all neighbours */
        const void CommunicateFluidVelocities();

        const void OutputInformation() const;

      private:
        const proc_t localRank;

        std::vector<Particle> particles;

        // map neighbourRank -> {numberOfParticlesFromThere, numberOfVelocitiesFromThere}
        typedef std::pair<unsigned int, unsigned int> scanMapElementType;
        std::map<proc_t, scanMapElementType> scanMap;
        typedef std::map<proc_t, scanMapElementType>::const_iterator scanMapConstIterType;
        typedef std::map<proc_t, scanMapElementType>::iterator scanMapIterType;
        typedef std::pair<proc_t, scanMapElementType> scanMapContentType;
        
        // a contiguous buffer into which MPI can write all the velocities from neighbours
        std::vector<std::pair<unsigned long, util::Vector3D<double> > > velocityBuffer;

        // map particleId -> sumOfvelocityContributionsFromNeighbours
        std::map<unsigned long, util::Vector3D<double> > velocityMap;

        /** contains useful geometry manipulation functions */
        const geometry::LatticeData& latDatLBM;

        /**
         * primary mechanism for interacting with the LB simulation
         * - the velocity cache: is used for velocity interpolation
         * - the forces cache  : stores the colloid feedback forces
         */
        lb::MacroscopicPropertyCache& propertyCache;

        /**
         * a vector of the processors that might be interested in
         * particles near the edge of this processor's sub-domain
         */
        //const std::vector<proc_t>& neighbourProcessors;

        net::Net net;
    };
  }
}

#endif /* HEMELB_COLLOIDS_PARTICLESET_H */
