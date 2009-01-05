#include "oscit/root.h"

namespace oscit {
Root::~Root()
{
  std::list<OscSend*>::iterator it  = mControllers.begin();
  std::list<OscSend*>::iterator end = mControllers.end();

  while (it != end) delete *it++;
  
  if (mOscIn) delete mOscIn;
  clear();
  mRoot = NULL; // avoid call to unregister_object in ~Object
}
} // namespace oscit