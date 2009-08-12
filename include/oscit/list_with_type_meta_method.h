#ifndef OSCIT_INCLUDE_OSCIT_LIST_WITH_TYPE_META_METHOD_H_
#define OSCIT_INCLUDE_OSCIT_LIST_WITH_TYPE_META_METHOD_H_
#include "oscit/root.h"

namespace oscit {

class ListWithTypeMetaMethod : public Object
{
public:  
  /** Class signature. */
  TYPED("Object.ListTypesMetaMethod")
  
  ListWithTypeMetaMethod(const char *name) : Object(name, StringIO("path", "List all children under the given path with their current value and type.")) {}

  virtual const Value trigger (const Value &path) {
    if (!path.is_string()) return gNilValue;
    
    Value error;
    Object * target = root_->find_or_build_object_at(path.c_str(), &error);
    
    return target ? target->list_with_type() : error;
  }
};

} // oscit

#endif // OSCIT_INCLUDE_OSCIT_LIST_WITH_TYPE_META_METHOD_H_
