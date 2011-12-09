#ifndef HEMELB_UNITTESTS_SIMULATIONMASTERTESTS_H
#define HEMELB_UNITTESTS_SIMULATIONMASTERTESTS_H

#include <cppunit/TestFixture.h>
#include "SimulationMaster.h"
#include "unittests/helpers/FolderTestFixture.h"

namespace hemelb
{
  namespace unittests
  {
    /**
     * Class to test the simulation master.
     */
    using namespace helpers;
    class SimulationMasterTests : public FolderTestFixture
    {
        CPPUNIT_TEST_SUITE(SimulationMasterTests);
        CPPUNIT_TEST(TestRun);
        CPPUNIT_TEST_SUITE_END();
      public:
        void setUp()
        {
          argc = 9;
          argv[0] = "hemelb";
          argv[2] = "four_cube.xml";
          argv[1] = "-in";
          argv[3] = "-i";
          argv[4] = "1";
          argv[5] = "-s";
          argv[6] = "1";
          argv[7] = "-ss";
          argv[8] = "1111";
          FolderTestFixture::setUp();
          CopyResourceToTempdir("four_cube.xml");
          CopyResourceToTempdir("four_cube.dat");
          options = new hemelb::configuration::CommandLine(argc, argv);
          master = new SimulationMaster(*options);
        }

        void tearDown()
        {
          FolderTestFixture::tearDown();
          delete master;
          delete options;
        }

        void TestRun()
        {
          master->RunSimulation();
          AssertPresent("results/timings1.asc");
        }

      private:
        int argc;
        hemelb::configuration::CommandLine *options;
        SimulationMaster *master;
        const char* argv[9];

    };

    CPPUNIT_TEST_SUITE_REGISTRATION(SimulationMasterTests);
  }
}

#endif /* HEMELB_UNITTESTS_SIMULATIONMASTERTESTS_H_ */