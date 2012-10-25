#ifndef __emu_step_Test_h__
#define __emu_step_Test_h__

#include "emu/step/TestParameters.h"

#include "emu/pc/XMLParser.h"
#include "emu/pc/XMLParser.h"
#include "emu/pc/EmuEndcap.h"
#include "emu/pc/DAQMB.h"

#include <vector>
#include <map>
#include <string>

using namespace std;

namespace emu{
  namespace step{
    class Test : public TestParameters {

    public:

      /// Ctor.
      ///
      /// @param id Test id.
      /// @param group VME group that this instance of emu::step::Test will handle.
      /// @param testParametersXML XML of the parameters of the test.
      /// @param generalSettingsXML XML of the PCrate and on-chamber electronics settings.
      /// @param specialSettingsXML XML of the PCrate and on-chamber electronics settings specific to this test.
      /// @param isFake If \em true, the test will just be simulated without VME communication.
      /// @param pLogger Pointer to the logger.
      ///
      /// @return The Test object.
      ///
      Test( const string& id, 
	    const string& group, 
	    const string& testParametersXML, 
	    const string& generalSettingsXML,
	    const string& specialSettingsXML,
	    const bool    isFake,
	    Logger*       pLogger              );

      /// Execute the test.
      ///
      ///
      void execute();

      /// Get the progress fo this test.
      ///
      ///
      /// @return Progress in percent.
      ///
      double getProgress();

      /// Interrupt the running test.
      ///
      ///
      void stop(){ isToStop_ = true; }

    private:
      string              group_; ///< Name of the VME crate group that this emu::step::Test instance handles.
      bool                isFake_; ///< If \em true, the test will just be simulated without VME communication.
      bool                isToStop_; ///< Set this to \em true to interrupt the test.
      emu::pc::XMLParser  parser_;
      uint64_t            iEvent_; ///< Progress counter.
      void ( emu::step::Test::* procedure_ )();

      void ( emu::step::Test::* getProcedure( const string& testId ) )(); 
      void createEndcap( const string& generalSettingsXML,
			 const string& specialSettingsXML  );
      void configureCrates();
      string withoutChars( const string& chars, const string& str );
      void enableTrigger();
      void disableTrigger();
      void setUpDMB( emu::pc::DAQMB *dmb );
      void _11();
      void _12();
      void _13();
      void _14();
      void _15();
      void _16();
      void _17();
      void _17b();
      void _18();
      void _19();
      void _21();
      void _25();
      void _27();
      void _fake();

    };
  }
}

#endif