/*
  ==============================================================================

   This file is part of the OSCIT library (http://rubyk.org/liboscit)
   Copyright (c) 2007-2009 by Gaspard Bucher - Buma (http://teti.ch).

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

#if 0
#ifndef OSCIT_INCLUDE_OSCIT_CALLBACK_OBSERVER_H_
#define OSCIT_INCLUDE_OSCIT_CALLBACK_OBSERVER_H_

#include "oscit/callback_list.h"

namespace oscit {

class Callback;

/** In order to create callbacks, an observer must inherit from Observer
 * so that it has a list of callbacks to disable on destruction.
 */
class Observer {
public:
  Observer() {}

  virtual ~Observer() {
    std::list<CallbackWithList*>::iterator it, end = produced_callbacks_.end();
    for (it = produced_callbacks_.begin(); it != end; ++it) {
      CallbackWithList *cl = *it;
      // we are dying, remove all callbacks that we have registered
      cl->callback_->observer_ = NULL;
      cl->list_->delete_callback(cl->callback_);
      delete cl;
    }
  }

protected:
  void delete_produced_callbacks_with_data(void *data) {
    std::list<CallbackWithList*>::iterator it = produced_callbacks_.begin(), end = produced_callbacks_.end();
    while (it != end) {
      CallbackWithList *cl = *it;
      if (cl->callback_->match_data(data)) {
        // found matching callback: remove
        cl->callback_->observer_ = NULL;
        cl->list_->delete_callback(cl->callback_);
        delete cl;
        it = produced_callbacks_.erase(it);
      } else {
        ++it;
      }
    }
  }

private:
  friend class Callback;

  struct CallbackWithList {
    CallbackWithList(CallbackList *list, Callback *callback) : list_(list), callback_(callback) {}
    CallbackList *list_;
    Callback *callback_;
  };

  void callback_produced(CallbackList *list, Callback *callback) {
    produced_callbacks_.push_back(new CallbackWithList(list, callback));
  }

  void callback_destroyed(CallbackList *list, Callback *callback) {
    observer_lock();
      std::list<CallbackWithList*>::iterator it = produced_callbacks_.begin(), end = produced_callbacks_.end();
      while (it != end) {
        CallbackWithList *cl = *it;
        if (cl->list_ == list && cl->callback_ == callback) {
          delete cl;
          it = produced_callbacks_.erase(it);
        } else {
          ++it;
        }
      }
    observer_unlock();
  }

  std::list<CallbackWithList*> produced_callbacks_;
};
} // oscit

#endif // OSCIT_INCLUDE_OSCIT_CALLBACK_OBSERVER_H_
#endif // 0