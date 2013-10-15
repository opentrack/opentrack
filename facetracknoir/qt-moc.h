#include <QObject> 

// this file exists only such that cmake qt automoc is appeased

class AutomocMe {
  Q_OBJECT
private:
  virtual void foo() = 0;
  AutomocMe() {}
};
