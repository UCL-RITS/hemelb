// 
// Copyright (C) University College London, 2007-2012, all rights reserved.
// 
// This file is part of HemeLB and is CONFIDENTIAL. You may not work 
// with, install, use, duplicate, modify, redistribute or share this
// file, or any part thereof, other than as allowed by any agreement
// specifically made by you with University College London.
// 

#ifndef HEMELB_UNITTESTS_GEOMETRY_NEIGHBOURING_REQUIREDSITEINFORMATIONTESTS_H
#define HEMELB_UNITTESTS_GEOMETRY_NEIGHBOURING_REQUIREDSITEINFORMATIONTESTS_H

#include "geometry/neighbouring/RequiredSiteInformation.h"
namespace hemelb
{
  namespace unittests
  {
    namespace geometry
    {
      namespace neighbouring
      {
        using namespace hemelb::geometry::neighbouring;
        class RequiredSiteInformationTests : public CppUnit::TestFixture
        {
            CPPUNIT_TEST_SUITE (RequiredSiteInformationTests);
            CPPUNIT_TEST (TestConstruct);
            CPPUNIT_TEST (TestSpecifyRequirement);
            CPPUNIT_TEST (TestAny);
            CPPUNIT_TEST (TestAnyNonFieldDependent);
            CPPUNIT_TEST (TestAnyFieldDependent);
            CPPUNIT_TEST (TestAnyMacroscopic);

            CPPUNIT_TEST_SUITE_END();

          public:
            RequiredSiteInformationTests()
            {
            }

            void setUp()
            {
              requirements = new RequiredSiteInformation();
            }

            void tearDown()
            {
              delete requirements;
            }

            void TestConstruct()
            {
              CPPUNIT_ASSERT(!requirements->IsRequired(terms::Distribution));
            }

            void TestSpecifyRequirement()
            {
              requirements->Require(terms::Distribution);
              CPPUNIT_ASSERT(requirements->IsRequired(terms::Distribution));
            }

            void TestAny()
            {
              CPPUNIT_ASSERT(!requirements->RequiresAny());
              requirements->Require(terms::Distribution);
              CPPUNIT_ASSERT(requirements->RequiresAny());
            }

            void TestAnyNonFieldDependent()
            {
              CPPUNIT_ASSERT(!requirements->RequiresAnyNonFieldDependent());
              requirements->Require(terms::Distribution);
              CPPUNIT_ASSERT(!requirements->RequiresAnyNonFieldDependent());
              requirements->Require(terms::SiteData);
              CPPUNIT_ASSERT(requirements->RequiresAnyNonFieldDependent());
            }

            void TestAnyFieldDependent()
            {
              CPPUNIT_ASSERT(!requirements->RequiresAnyFieldDependent());
              requirements->Require(terms::SiteData);
              CPPUNIT_ASSERT(!requirements->RequiresAnyFieldDependent());
              requirements->Require(terms::Distribution);
              CPPUNIT_ASSERT(requirements->RequiresAnyFieldDependent());
            }

            void TestAnyMacroscopic()
            {
              CPPUNIT_ASSERT(!requirements->RequiresAnyMacroscopic());
              requirements->Require(terms::SiteData);
              CPPUNIT_ASSERT(!requirements->RequiresAnyMacroscopic());
              requirements->Require(terms::Distribution);
              CPPUNIT_ASSERT(!requirements->RequiresAnyMacroscopic());
              requirements->Require(terms::Velocity);
              CPPUNIT_ASSERT(requirements->RequiresAnyMacroscopic());
            }

          private:
            RequiredSiteInformation *requirements;
        };
        // CPPUNIT USES LINENUMBER TO REGISTER MACRO
        // EXTRA LINE
        // EXTRA LINE
        // EXTRA LINE
        CPPUNIT_TEST_SUITE_REGISTRATION (RequiredSiteInformationTests);
      }
    }
  }
}

#endif
