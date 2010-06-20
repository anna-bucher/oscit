/*
  ==============================================================================

   This file is part of the OSCIT library (http://rubyk.org/liboscit)
   Copyright (c) 2007-2010 by Gaspard Bucher - Buma (http://teti.ch).

  ------------------------------------------------------------------------------

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
   THE SOFTWARE.

  ==============================================================================
*/

#include "oscit/object.h"

#include <string>
#include <list>

#include "oscit/root.h"
#include "oscit/alias.h"
#include "oscit/object_handle.h"

namespace oscit {

Object::~Object() {
  std::list<Alias*>::iterator it, end = aliases_.end();
  // notify parent and root
  set_parent(NULL);
  set_root(NULL);

  for (it = aliases_.begin(); it != end; ++it) {
    // to avoid notification to this dying object
    (*it)->unlink_original();
    delete *it;
  }

  clear();
}

void Object::from_hash(const Value &val, Value *result) {
  HashIterator it;
  HashIterator end = val.end();
  Value param;
  ObjectHandle handle;

  for (it = val.begin(); it != end; ++it) {
    if (get_child(*it, &handle) && val.get(*it, &param)) {
      result->set(*it, root_->call(handle, param, NULL));
    } else {
      result->set(*it, ErrorValue(NOT_FOUND_ERROR, *it));
    }
  }
}

bool Object::set_all_ok(const Value &val) {
  HashIterator it;
  HashIterator end = val.end();
  Value param;
  bool all_ok = true;
  ObjectHandle handle;

  for (it = val.begin(); it != end; ++it) {
    if (get_child(*it, &handle) && val.get(*it, &param)) {
      all_ok = !root_->call(handle, param, NULL).is_error() && all_ok;
    } else {
      all_ok = false;
    }
  }
  return all_ok;
}

/** Inform the object of an alias that depends on it. */
void Object::register_alias(Alias *alias) {
  aliases_.push_back(alias);
}

/** Inform the object that an alias no longer exists. */
void Object::unregister_alias(Alias *alias) {
  aliases_.remove(alias);
}

/** Free the child from the list of children. */
void Object::unregister_child(Object *object) {
  { ScopedWrite lock(children_);
    children_.remove_element(object);
  }

  { ScopedWrite lock(children_vector_);
    std::vector<Object*>::iterator it, end = children_vector_.end();
    for(it = children_vector_.begin(); it != end; ++it) {
      if (*it == object) {
        it = children_vector_.erase(it);
        break;
      }
    }
  }
}

void Object::moved() {
  // 1. get new name from parent, register as child
  if (parent_) {
    // rebuild fullpath
    url_ = std::string(parent_->url()).append("/").append(name_);
    set_root(parent_->root_);
    set_context(parent_->context_);
  } else if (root_ == this) {
    // root: url does not contain name
    url_ = "";
  } else {
    // no parent
    url_ = name_;
    set_root(NULL);
  }

  { ScopedRead lock(children_);
    StringIterator it;
    StringIterator end = children_.end();
    Object *child;

    // 3. update children
    for(it = children_.begin(); it != end; it++) {
      if (children_.get(*it, &child)) child->moved();
    }
  }
}

void Object::register_child(Object *object) {
  // 1. make sure it is not in dictionary
  unregister_child(object);

  Object *child;
  // 2. get valid name
  { ScopedWrite lock(children_);
    while (children_.get(object->name_, &child)) {
      object->find_next_name();
    }

    // 3. add to dictionary with new name
    children_.set(object->name_, object);
  }

  { ScopedWrite lock(children_vector_);
    size_t sz = children_vector_.size();
        // goes last anyway   // empty list   // no keep_last_ object in list
    if (object->keep_last_    || sz == 0      || !children_vector_[sz-1]->keep_last_) {
      // just append at the end
      children_vector_.push_back(object);
    } else {
      // insert before the first 'keep_last_' child
      std::vector<Object*>::iterator it, end = children_vector_.end();
      for(it = children_vector_.begin(); it != end; ++it) {
        if ((*it)->keep_last_) {
          children_vector_.insert(it, object);
          break;
        }
      }
    }
  }
}

void Object::set_root(Root *root) {
  if (root_) root_->unregister_object(this);
  root_ = root;
  if (root_) root_->register_object(this);
}

void Object::set_parent(Object *parent) {
  if (parent_) parent_->unregister_child(this);
  parent_ = parent;
  if (parent_) parent_->register_child(this);
  moved();
  adopted();
}

void Object::clear() {
  ScopedWrite lock(children_);
  StringIterator it;
  StringIterator end = children_.end();

  // destroy all children
  for(it = children_.begin(); it != end; it++) {
    Object * child;
    if (children_.get(*it, &child)) {
      // to avoid 'unregister_child' call (would alter children_)
      child->parent_ = NULL;
      if (root_) root_->unregister_object(child);
      child->release();
    }
  }
  children_.clear();
}


const Value Object::list() const {
  ScopedRead lock(children_vector_);
  ListValue list;

  std::vector<Object*>::const_iterator it, end = children_vector_.end();
  for(it = children_vector_.begin(); it != end; ++it) {
    const Object *obj = *it;
    if (obj->children_.empty()) {
      list.push_back(obj->name_);
    } else {
      list.push_back(std::string(obj->name_).append("/"));
    }
  }

  return list;
}


void Object::insert_in_hash(Value *result) {

  { ScopedRead lock(children_vector_);
    if (children_vector_.size() > 0) {
      std::vector<Object*>::iterator it, end = children_vector_.end();

      for(it = children_vector_.begin(); it != end; ++it) {
        Object *obj = *it;

        Value obj_hash;
        obj->insert_in_hash(&obj_hash);

        if (!obj_hash.is_nil()) {
          result->set(obj->name(), obj_hash);
        }
      }
      return;
    }
  }

  // no children get value by sending trigger.
  *result = trigger(gNilValue);
}

const Value Object::list_with_type() {
  ScopedRead lock(children_vector_);
  ListValue list;

  std::vector<Object*>::iterator it, end = children_vector_.end();
  for(it = children_vector_.begin(); it != end; ++it) {
    ListValue name_with_type;
    Object *obj = *it;
    if (obj->children_.empty()) {
      name_with_type.push_back(obj->name_);
    } else {
      name_with_type.push_back(std::string(obj->name_).append("/"));
    }
    name_with_type.push_back(obj->type_with_current_value());
    list.push_back(name_with_type);
  }

  return list;
}

const Value Object::type_with_current_value() {
  Value type = type_;

  if (type.is_string()) {
    // meta type is string = just information (not callable)
    return type;
  }

  if (!type.is_list()) {
    // make sure type is a ListValue
    // type can be gNilValue for object proxies when they are not yet initialized (haven't received type information).
    return ErrorValue(INTERNAL_SERVER_ERROR, "Invalid meta type. Should be a list (found '").append(type.type_tag()).append("').");
  }

  if (!type[0].is_any() && !type[0].is_nil()) {
    // get current value
    Value current = trigger(gNilValue);

    if (current.is_nil()) {
      // current type cannot be queried. Leave dummy value.
    } else if (current.type_id() != type[0].type_id()) {
      // make sure current value type is compatible with type
      return ErrorValue(INTERNAL_SERVER_ERROR, "Current value type not matching meta type (expected '").append(
          type[0].type_tag()).append(
          "' found '").append(current.type_tag()).append("').");
    } else {
      type.set_value_at(0, current);
    }

  }
  return type;
}

bool Object::get_child(const std::string &name, ObjectHandle *handle) {
  Object * child;
  ScopedRead lock(children_);
  if (children_.get(name, &child)) {
    handle->hold(child);
    return true;
  } else {
    return false;
  }
}

bool Object::get_child(size_t index, ObjectHandle *handle) {
  ScopedRead lock(children_vector_);
  if (index >= children_vector_.size()) return false;
  Object *object = children_vector_[index];
  handle->hold(object);
  return true;
}

// FIXME: 'tree' is bad (locks too much because of recursion) and we do not need it
// in OSCIT.
void Object::tree(size_t base_length, Value *tree) const {
  ConstStringIterator it, end = children_.end();
  ScopedRead lock(children_);
  for (it = children_.begin(); it != end; ++it) {
    Object * obj;
    if (children_.get(*it, &obj)) {
      // if (!obj->kind_of(Alias)) {
        // do not list alias (Alias are used as internal helpers and
        // do not need to be advertised) ?
        tree->push_back(obj->url().substr(base_length));
        obj->tree(base_length, tree);
      //}
    }
  }
}

} // oscit
