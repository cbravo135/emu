#ifndef __FACT_H__
#define __FACT_H__

#include <sstream>
#include <ostream>
#include <map>


namespace emu { namespace base {
  class Fact;
  using namespace std;
  ///
  /// 
  ///
  class Fact {
  public:

    enum ValueType_t   { stringType=0, numericType, booleanType, nValueTypes };
    enum ParameterId_t { HV_CURRENT=0, LV_CURRENT, DATA_EVENTS, nParameterIds };
    enum Unit_t        { Amper=0, Volt, meter, nUnits };
    enum Severity_t    { DEBUG=0, INFO, WARN, ERROR, FATAL, nSeverities };

    friend ostream& operator<<( ostream& os,       emu::base::Fact& f );
    friend ostream& operator<<( ostream& os, const emu::base::Fact& f );

    Fact();

    const string& getRun           () const { return run_;                  }
    const string& getTime          () const { return time_;                 }
    const string& getComponent     () const { return component_;            }
    const string& getParameterId   () const { return parameterId_;          }
    const string& getUnit          () const { return unit_;                 }
    const string& getSeverity      () const { return severity_;             }
    const string& getDescription   () const { return description_;          }
    const string& getValueTagName  () const { return valueTagName_;         }
    const map<string,string>& getParameters() const { return parameters_;   }


  protected:
    string defaultDateTime();
   
    static const char* parameterIds_[nParameterIds];
    static const char* units_[nUnits];
    static const char* severities_[nSeverities];
    string run_;
    string time_;
    string component_;
    string parameterId_;
    string unit_;
    string severity_;
    string description_;
    string valueTagName_;

    map<string,string> parameters_;
  };
}} // namespace emu::base

#endif  // ifndef __FACT_H__