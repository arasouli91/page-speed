// Copyright 2009 Google Inc. All Rights Reserved.
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

#include "pagespeed/rules/minimize_request_size.h"

#include <string>
#include <vector>

#include "base/logging.h"
#include "pagespeed/core/formatter.h"
#include "pagespeed/core/pagespeed_input.h"
#include "pagespeed/core/resource.h"
#include "pagespeed/core/resource_util.h"
#include "pagespeed/core/result_provider.h"
#include "pagespeed/core/rule_input.h"
#include "pagespeed/l10n/l10n.h"
#include "pagespeed/proto/pagespeed_output.pb.h"

namespace {

// Maximum size of around 1 packet.  There is no guarantee that 1500 bytes will
// actually fit in the first packet so the exact value of this constant might
// need some tweaking.  What is important is that the whole request fit in
// a single burst while the TCP window size is still small.
const int kMaximumRequestSize = 1500;

}

namespace pagespeed {

namespace rules {

MinimizeRequestSize::MinimizeRequestSize()
    : pagespeed::Rule(pagespeed::InputCapabilities(
        pagespeed::InputCapabilities::REQUEST_HEADERS)) {
}

const char* MinimizeRequestSize::name() const {
  return "MinimizeRequestSize";
}

LocalizableString MinimizeRequestSize::header() const {
  // TRANSLATOR: TODO: add a translator comment describing this string
  return _("Minimize request size");
}

const char* MinimizeRequestSize::documentation_url() const {
  return "request.html#MinimizeRequestSize";
}

bool MinimizeRequestSize::AppendResults(const RuleInput& rule_input,
                                        ResultProvider* provider) {
  const PagespeedInput& input = rule_input.pagespeed_input();
  for (int idx = 0, num = input.num_resources(); idx < num; ++idx) {
    const Resource& resource = input.GetResource(idx);

    int request_bytes = resource_util::EstimateRequestBytes(resource);
    if (request_bytes > kMaximumRequestSize) {
      Result* result = provider->NewResult();
      result->set_original_request_bytes(request_bytes);
      result->add_resource_urls(resource.GetRequestUrl());

      Savings* savings = result->mutable_savings();
      savings->set_request_bytes_saved(request_bytes - kMaximumRequestSize);

      pagespeed::ResultDetails* details_container = result->mutable_details();
      pagespeed::RequestDetails* details =
          details_container->MutableExtension(
              pagespeed::RequestDetails::message_set_extension);
      details->set_url_length(resource.GetRequestUrl().size());
      details->set_cookie_length(resource.GetCookies().size());
      details->set_referer_length(resource.GetRequestHeader("referer").size());
      details->set_is_static(resource_util::IsLikelyStaticResource(resource));
    }
  }

  return true;
}

void MinimizeRequestSize::FormatResults(const ResultVector& results,
                                       Formatter* formatter) {
  if (results.empty()) {
    return;
  }

  Argument size_threshold(Argument::BYTES, kMaximumRequestSize);
  Formatter* body = formatter->AddChild(
      // TRANSLATOR: TODO: add a translator comment describing this string
      _("The requests for the following URLs don't fit in a single packet.  "
        "Reducing the size of these requests could reduce latency."),
      size_threshold);

  for (ResultVector::const_iterator iter = results.begin(),
           end = results.end();
       iter != end;
       ++iter) {

    const Result& result = **iter;
    if (result.resource_urls_size() != 1) {
      LOG(DFATAL) << "Unexpected number of resource URLs.  Expected 1, Got "
                  << result.resource_urls_size() << ".";
      continue;
    }

    Argument url(Argument::URL, result.resource_urls(0));
    Argument size(Argument::BYTES, result.original_request_bytes());
    // TRANSLATOR: TODO: add a translator comment describing this string
    Formatter* entry = body->AddChild(_("$1 has a request size of $2"),
                                      url, size);

    const ResultDetails& details_container = result.details();
    if (details_container.HasExtension(RequestDetails::message_set_extension)) {
      const RequestDetails& details = details_container.GetExtension(
          RequestDetails::message_set_extension);

      Argument url_size(Argument::BYTES, details.url_length());
      // TRANSLATOR: TODO: add a translator comment describing this string
      entry->AddChild(_("Request URL: $1"), url_size);

      Argument cookie_size(Argument::BYTES, details.cookie_length());
      if (details.is_static() && details.cookie_length() > 0) {
        // TRANSLATOR: TODO: add a translator comment describing this string
        entry->AddChild(_("Cookies: $1 (note that this is a static resource, "
                          "and should be served from a cookieless domain)"),
                        cookie_size);
      } else {
        // TRANSLATOR: TODO: add a translator comment describing this string
        entry->AddChild(_("Cookies: $1"), cookie_size);
      }

      Argument referer_size(Argument::BYTES, details.referer_length());
      // TRANSLATOR: TODO: add a translator comment describing this string
      entry->AddChild(_("Referer Url: $1"), referer_size);

      Argument other_size(Argument::BYTES,
                          result.original_request_bytes() -
                          details.url_length() -
                          details.cookie_length() -
                          details.referer_length());
      // TRANSLATOR: TODO: add a translator comment describing this string
      entry->AddChild(_("Other: $1"), other_size);
    }
  }
}

}  // namespace rules

}  // namespace pagespeed
