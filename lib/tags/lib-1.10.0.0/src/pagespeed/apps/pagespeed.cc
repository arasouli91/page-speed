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

// Command line utility that runs lint rules on the provided input set.

#include <stdio.h>

#include <fstream>

#include "base/at_exit.h"
#include "base/logging.h"
#include "base/scoped_ptr.h"
#include "base/stl_util-inl.h"
#include "base/string_util.h"
#include "google/protobuf/io/zero_copy_stream_impl_lite.h"
#include "google/protobuf/stubs/common.h"
#include "pagespeed/core/engine.h"
#include "pagespeed/core/pagespeed_init.h"
#include "pagespeed/core/pagespeed_input.h"
#include "pagespeed/core/resource.h"
#include "pagespeed/formatters/json_formatter.h"
#include "pagespeed/formatters/proto_formatter.h"
#include "pagespeed/formatters/text_formatter.h"
#include "pagespeed/har/http_archive.h"
#include "pagespeed/image_compression/image_attributes_factory.h"
#include "pagespeed/l10n/localizer.h"
#include "pagespeed/proto/pagespeed_input.pb.h"
#include "pagespeed/proto/pagespeed_output.pb.h"
#include "pagespeed/proto/proto_resource_utils.h"
#include "pagespeed/rules/rule_provider.h"

namespace {

// UTF-8 byte order mark.
const char* kUtf8Bom = "\xEF\xBB\xBF";
const size_t kUtf8BomSize = strlen(kUtf8Bom);

bool ReadFileToString(const std::string &file_name, std::string *dest) {
  std::ifstream file_stream;
  file_stream.open(
      file_name.c_str(), std::ifstream::in | std::ifstream::binary);
  if (file_stream.fail()) {
    return false;
  }
  dest->assign(std::istreambuf_iterator<char>(file_stream),
               std::istreambuf_iterator<char>());
  file_stream.close();
  return true;
}

pagespeed::PagespeedInput* ParseProtoInput(const std::string& file_contents) {
  pagespeed::ProtoInput input_proto;
  ::google::protobuf::io::ArrayInputStream input_stream(
      file_contents.data(), file_contents.size());
  bool success = input_proto.ParseFromZeroCopyStream(&input_stream);
  CHECK(success);

  pagespeed::PagespeedInput *input = new pagespeed::PagespeedInput;
  pagespeed::proto::PopulatePagespeedInput(input_proto, input);
  if (!input_proto.identifier().empty()) {
    input->SetPrimaryResourceUrl(input_proto.identifier());
  }
  return input;
}

void PrintUsage() {
  fprintf(stderr,
          "Usage: pagespeed <output_format> <input_format> <file>\n"
          "       <output_format> can be one of 'text', 'json', or 'proto'\n"
          "       <input_format> can be one of 'har', or 'proto'\n"
          "       if <file> is '-', input will be read from stdin.\n"
          "       Otherwise input will be read from the specified filename.\n");
}

bool RunPagespeed(const std::string& out_format,
                  const std::string& in_format,
                  const std::string& filename) {
  std::string file_contents;
  if (filename == "-") {
    // Special case: if user specifies input file as '-', read the
    // input from stdin.
    file_contents.assign(std::istreambuf_iterator<char>(std::cin),
                         std::istreambuf_iterator<char>());
  } else if (!ReadFileToString(filename, &file_contents)) {
    fprintf(stderr, "Could not read input from %s\n", filename.c_str());
    PrintUsage();
    return false;
  }

  // TODO(lsong): Add support for byte order mark.
  // For now, strip byte order mark of the content if exists.
  if (file_contents.compare(0, kUtf8BomSize, kUtf8Bom) == 0) {
    file_contents.erase(0, kUtf8BomSize);
    LOG(INFO) << "Byte order mark ignored.";
  }

  scoped_ptr<pagespeed::RuleFormatter> formatter;
  if (out_format == "json") {
    formatter.reset(new pagespeed::formatters::JsonFormatter(&std::cout,
                                                             NULL));
  } else if (out_format == "proto") {
    // We don't need a formatter, since we're printing the raw results
  } else if (out_format == "text") {
    formatter.reset(new pagespeed::formatters::TextFormatter(&std::cout));
  } else {
    fprintf(stderr, "Invalid output format %s\n", out_format.c_str());
    PrintUsage();
    return false;
  }

  scoped_ptr<pagespeed::PagespeedInput> input;
  if (in_format == "har") {
    input.reset(pagespeed::ParseHttpArchive(file_contents));
  } else if (in_format == "proto") {
    input.reset(ParseProtoInput(file_contents));
  } else {
    fprintf(stderr, "Invalid input format %s\n", in_format.c_str());
    PrintUsage();
    return false;
  }
  if (input == NULL) {
    fprintf(stderr, "Failed to parse input.\n");
    return false;
  }
  if (input->primary_resource_url().empty() && input->num_resources() > 0) {
    // If no primary resource URL was specified, assume the first
    // resource is the primary resource.
    input->SetPrimaryResourceUrl(input->GetResource(0).GetRequestUrl());
  }

  input->AcquireImageAttributesFactory(
      new pagespeed::image_compression::ImageAttributesFactory());

  input->Freeze();

  std::vector<pagespeed::Rule*> rules;

  // In environments where exceptions can be thrown, use
  // STLElementDeleter to make sure we free the rules in the event
  // that they are not transferred to the Engine.
  STLElementDeleter<std::vector<pagespeed::Rule*> > rule_deleter(&rules);

  bool save_optimized_content = true;
  std::vector<std::string> incompatible_rule_names;
  pagespeed::rule_provider::AppendCompatibleRules(
      save_optimized_content,
      &rules,
      &incompatible_rule_names,
      input->EstimateCapabilities());
  if (!incompatible_rule_names.empty()) {
    std::string incompatible_rule_list =
        JoinString(incompatible_rule_names, ' ');
    LOG(INFO) << "Removing incompatible rules: " << incompatible_rule_list;
  }

  // Ownership of rules is transferred to the Engine instance.
  pagespeed::Engine engine(&rules);
  engine.Init();

  // If we have a formatter, use it; otherwise, print the raw Results proto
  if (formatter.get()) {
    engine.ComputeAndFormatResults(*input.get(), formatter.get());
  } else {
    pagespeed::Results results;
    engine.ComputeResults(*input.get(), &results);

    std::string out;
    ::google::protobuf::io::StringOutputStream out_stream(&out);
    results.SerializeToZeroCopyStream(&out_stream);
    std::cout << out;
  }

  return true;
}

}  // namespace

int main(int argc, char** argv) {
  if (argc != 4) {
    PrintUsage();
    return 1;
  }

  // Some of our code uses Singleton<>s, which require an
  // AtExitManager to schedule their destruction.
  base::AtExitManager at_exit_manager;

  pagespeed::Init();
  bool result = RunPagespeed(argv[1], argv[2], argv[3]);
  pagespeed::ShutDown();
  google::protobuf::ShutdownProtobufLibrary();

  return result ? EXIT_SUCCESS : EXIT_FAILURE;
}
