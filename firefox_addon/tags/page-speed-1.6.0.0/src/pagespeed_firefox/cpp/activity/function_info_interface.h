/**
 * Copyright 2008-2009 Google Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// Author: Bryan McQuade
//
// FunctionInfoInterface is an that allows for querying for
// information about a JavaScript function. This interface exists to
// decouple the Firefox JavaScript debugger internals from the
// CallGraphProfile.

#ifndef FUNCTION_INFO_INTERFACE_H_
#define FUNCTION_INFO_INTERFACE_H_

#include "base/basictypes.h"

namespace activity {

// See comment at top of file for a complete description
class FunctionInfoInterface {
 public:
  virtual ~FunctionInfoInterface() {}
  virtual int32 GetFunctionTag() = 0;
  virtual const char *GetFileName() = 0;
  virtual const char *GetFunctionName() = 0;
  virtual const char *GetFunctionSourceUtf8() = 0;
};

}  // namespace activity

#endif  // FUNCTION_INFO_INTERFACE_H_
