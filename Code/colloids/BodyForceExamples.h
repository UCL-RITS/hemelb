// 
// Copyright (C) University College London, 2007-2012, all rights reserved.
// 
// This file is part of HemeLB and is CONFIDENTIAL. You may not work 
// with, install, use, duplicate, modify, redistribute or share this
// file, or any part thereof, other than as allowed by any agreement
// specifically made by you with University College London.
// 

#ifndef HEMELB_COLLOIDS_BODYFORCEEXAMPLES_H
#define HEMELB_COLLOIDS_BODYFORCEEXAMPLES_H

#include "colloids/BodyForces.h"

namespace hemelb
{
  namespace colloids
  {
    /** general class representing a linear body force, e.g. gravity over short distances */
    class ConstantBodyForce : public BodyForce
    {
      public:
        static BodyForce* ReadFromXml(io::xml::Element& xml)
        {
          LatticeForceVector field;
          io::xml::Element force = xml.GetChildOrThrow("force");
          // TODO: convert to lattice units
          xml.GetAttributeOrThrow("x", field.x);
          xml.GetAttributeOrThrow("y", field.y);
          xml.GetAttributeOrThrow("z", field.z);
          return new ConstantBodyForce(field);
        };

        virtual const LatticeForceVector GetForceForParticle(const Particle&) const
        {
          return constantForce;
        };

      protected:
        ConstantBodyForce(const LatticeForceVector constantForce) :
          BodyForce(), constantForce(constantForce) {};

        const LatticeForceVector constantForce;
    };

    class ConstantBodyForceFactory : public BodyForceFactory<ConstantBodyForce> { };


    /** general class representing a radial body force, i.e.
     *  inversely proportional to the square of the distance
     *  from the centre and in the direction from the centre
     *  force(r) = magnitude / (|r - centre| * |r - centre|)
     */
    class RadialBodyForce : public BodyForce
    {
      public:
        static BodyForce* ReadFromXml(io::xml::Element& xml)
        {
          LatticeForce magnitude;
          LatticePosition centrePoint;
          // TODO: convert to lattice units.
          xml.GetAttributeOrThrow("magnitude", magnitude);
          // TODO: convert to lattice units.
          io::xml::Element centreElem = xml.GetChildOrThrow("centrePoint");
          centreElem.GetAttributeOrThrow("x", centrePoint.x);
          centreElem.GetAttributeOrThrow("y", centrePoint.y);
          centreElem.GetAttributeOrThrow("z", centrePoint.z);

          return new RadialBodyForce(centrePoint, magnitude);
        };

        virtual const LatticeForceVector GetForceForParticle(const Particle& particle) const
        {
          const LatticePosition& direction = particle.GetGlobalPosition() - centrePoint;
/*
          printf("In RadialBF::GetForce - centre: {%g,%g,%g}, mag: %g, pos: {%g,%g,%g}, dir: {%g,%g,%g}\n",
            centrePoint.x, centrePoint.y, centrePoint.z,
            magnitude, particle.GetGlobalPosition().x,
            particle.GetGlobalPosition().y, particle.GetGlobalPosition().z,
            direction.x, direction.y, direction.z);
*/
          if (direction.GetMagnitudeSquared() < 0.0000001)
            return LatticeForceVector();
          return direction.GetNormalised() * (magnitude / direction.GetMagnitudeSquared());
        };

      protected:
        RadialBodyForce(const LatticePosition centrePoint, const LatticeForce magnitude) :
          BodyForce(), centrePoint(centrePoint), magnitude(magnitude) {};

        const LatticePosition centrePoint;
        const LatticeForce magnitude;
    };

    class RadialBodyForceFactory : public BodyForceFactory<RadialBodyForce> { };
  }
}
#endif /* HEMELB_COLLOIDS_BODYFORCEEXAMPLES_H */
