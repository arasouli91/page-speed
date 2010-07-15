// Copyright 2010 Google Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.


#ifndef MOD_PAGESPEED_MOD_PAGESPEED_H_
#define MOD_PAGESPEED_MOD_PAGESPEED_H_

#include "base/basictypes.h"

// Forward declaration.
struct server_rec;

namespace html_rewriter {

struct PageSpeedConfig;
class PageSpeedServerContext;

PageSpeedConfig* get_pagespeed_config(server_rec* server);
PageSpeedServerContext* mod_pagespeed_get_config_server_context(
    server_rec* server);
}  // namespace html_rewriter

#endif  // MOD_PAGESPEED_MOD_PAGESPEED_H_