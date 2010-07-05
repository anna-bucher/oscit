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

#ifndef OSCIT_INCLUDE_OSCIT_ERROR_META_METHOD_H_
#define OSCIT_INCLUDE_OSCIT_ERROR_META_METHOD_H_
#include "oscit/root.h"

namespace oscit {

class ErrorMetaMethod : public Object
{
public:
  /** Class signature. */
  TYPED("Object.ErrorMetaMethod")

  ErrorMetaMethod(const char * name)       : Object(name, Attribute::any_io("Errors returned should end here.")) {}
  ErrorMetaMethod(const std::string &name) : Object(name, Attribute::any_io("Errors returned should end here.")) {}

  virtual const Value trigger(const Value &val) {
    std::cerr << "[" << url() << "] received " << val << std::endl;
    return gNilValue;
  }
};

} // oscit

#endif // OSCIT_INCLUDE_OSCIT_ERROR_META_METHOD_H_
