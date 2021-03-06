# Copyright 2009 Google Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

{
  'variables': {
    'libpagespeed_root': '<(DEPTH)/third_party/libpagespeed/src',
    'xulrunner_sdk_root': '<(DEPTH)/third_party/xulrunner-sdk',
    'xulrunner_sdk_os_root': '<(xulrunner_sdk_root)/arch/<(OS)',
    'xulrunner_sdk_arch_root': '<(xulrunner_sdk_os_root)/<(target_arch)',
  },
  'targets': [
    {
      'target_name': 'xulrunner_sdk',
      'type': 'none',
      'direct_dependent_settings': {
        'rules': [
          {
            'rule_name': 'xpidl',
            'extension': 'idl',
            'outputs': ['<(SHARED_INTERMEDIATE_DIR)/<(RULE_INPUT_ROOT).h'],
            'action': [
              '<(xulrunner_sdk_arch_root)/bin/xpidl',
              '-m', 'header',
              '-I', '<(xulrunner_sdk_root)/idl',
              '-e', '<(SHARED_INTERMEDIATE_DIR)/<(RULE_INPUT_ROOT).h',
              '<(RULE_INPUT_PATH)',
            ],
          },
        ],
        'include_dirs': [
          '<(SHARED_INTERMEDIATE_DIR)',  # For headers generated from idl files
          '<(xulrunner_sdk_root)/include',
          '<(xulrunner_sdk_os_root)/all/include',
        ],
        'conditions': [  # See https://developer.mozilla.org/en/XPCOM_Glue
          ['OS == "linux"', {
            'cflags': [
              '-fshort-wchar',
              '-include', '<(xulrunner_sdk_os_root)/all/include/xpcom-config.h',
            ],
            'ldflags': [
              '-L<(xulrunner_sdk_arch_root)/lib',
              '-L<(xulrunner_sdk_arch_root)/bin',
              '-Wl,-rpath-link,<(xulrunner_sdk_arch_root)/bin',
            ],
            'link_settings': {
              'libraries': [
                '-lxpcomglue_s',
                '-lxpcom',
                '-lnspr4',
            ]},
          }],
          ['OS == "mac"', {
            'link_settings': {
              'libraries': [
                'libxpcomglue_s.a',
                'libxpcom.dylib',
                'libnspr4.dylib',
              ],
            },
            'xcode_settings': {
              'LIBRARY_SEARCH_PATHS': [
                '<(xulrunner_sdk_arch_root)/lib',
                '<(xulrunner_sdk_arch_root)/bin',
              ],
              'OTHER_CFLAGS': [
                '-fshort-wchar',
                '-include <(xulrunner_sdk_os_root)/all/include/xpcom-config.h',
              ],
            },
          }],
          ['OS == "win"', {
            'cflags': [
              '/FI "xpcom-config.h"',
              '/Zc:wchar_t-',
            ],
            'defines': [
              'XP_WIN',
            ],
            'link_settings': {
              'libraries': [
                '<(xulrunner_sdk_arch_root)/lib/xpcomglue_s.lib',
                '<(xulrunner_sdk_arch_root)/lib/xpcom.lib',
                '<(xulrunner_sdk_arch_root)/lib/nspr4.lib',
              ],
            },
          }]
        ],
      },
    },
    {
      'target_name': 'pagespeed_firefox_profile_pb',
      'type': '<(library)',
      'hard_dependency': 1,
      'dependencies': [
        '<(DEPTH)/third_party/protobuf2/protobuf.gyp:protobuf_lite',
        '<(DEPTH)/third_party/protobuf2/protobuf.gyp:protoc',
      ],
      'actions': [
        {
          'action_name': 'generate_pagespeed_firefox_profile_pb',
          'variables': {
            'proto_path': 'protobuf',
          },
          'inputs': [
            '<(PRODUCT_DIR)/<(EXECUTABLE_PREFIX)protoc<(EXECUTABLE_SUFFIX)',
            '<(proto_path)/activity/profile.proto',
          ],
          'outputs': [
            '<(SHARED_INTERMEDIATE_DIR)/activity/profile.pb.cc',
            '<(SHARED_INTERMEDIATE_DIR)/activity/profile.pb.h',
          ],
          'dependencies': [
            '<(DEPTH)/third_party/protobuf2/protobuf.gyp:protoc',
          ],
          'action': [
            '<(PRODUCT_DIR)/<(EXECUTABLE_PREFIX)protoc<(EXECUTABLE_SUFFIX)',
            '<(proto_path)/activity/profile.proto',
            '--proto_path=<(proto_path)',
            '--cpp_out=<(SHARED_INTERMEDIATE_DIR)',
          ],
        },
      ],
      'sources': [
        '<(SHARED_INTERMEDIATE_DIR)/activity/profile.pb.cc',
      ],
      'include_dirs': [
        '<(SHARED_INTERMEDIATE_DIR)',
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          '<(SHARED_INTERMEDIATE_DIR)/activity',
        ],
      },
      'export_dependent_settings': [
        '<(DEPTH)/third_party/protobuf2/protobuf.gyp:protobuf_lite',
      ]
    },
    {
      'target_name': 'pagespeed_firefox_activity_common',
      'type': '<(library)',
      'variables': {
        'activity_root': 'cpp/activity',
      },
      'dependencies': [
        'pagespeed_firefox_profile_pb',
        '<(DEPTH)/base/base.gyp:base',
      ],
      'sources': [
        '<(activity_root)/call_graph.cc',
        '<(activity_root)/call_graph_metadata.cc',
        '<(activity_root)/call_graph_profile.cc',
        '<(activity_root)/call_graph_profile_snapshot.cc',
        '<(activity_root)/call_graph_timeline_event_set.cc',
        '<(activity_root)/call_graph_timeline_visitor.cc',
        '<(activity_root)/call_graph_util.cc',
        '<(activity_root)/call_graph_visit_filter_interface.cc',
        '<(activity_root)/call_graph_visitor_interface.cc',
        '<(activity_root)/clock.cc',
        '<(activity_root)/delayable_function_tree_view_delegate.cc',
        '<(activity_root)/find_first_invocations_visitor.cc',
        '<(activity_root)/timer.cc',
        '<(activity_root)/uncalled_function_tree_view_delegate.cc',
      ],
      'include_dirs': [
        '<(activity_root)',
      ],
    },
    {
      'target_name': 'pagespeed_firefox_activity_gecko',
      'type': '<(library)',
      'variables': {
        'activity_root': 'cpp/activity',
      },
      'dependencies': [
        'xulrunner_sdk',
        'pagespeed_firefox_activity_common',
        '<(DEPTH)/base/base.gyp:base',
      ],
      'sources': [
        'idl/IActivityProfiler.idl',
        '<(activity_root)/basic_tree_view.cc',
        '<(activity_root)/check_gecko.cc',
        '<(activity_root)/jsd_call_hook.cc',
        '<(activity_root)/jsd_function_info.cc',
        '<(activity_root)/jsd_script_hook.cc',
        '<(activity_root)/profiler.cc',
        '<(activity_root)/profiler_event.cc',
        '<(activity_root)/profiler_runnables.cc',
      ],
      'include_dirs': [
        '<(activity_root)',
      ],
    },
    {
      'target_name': 'pagespeed_firefox_js_min',
      'type': '<(library)',
      'variables': {
        'js_min_root': 'cpp/js_min',
      },
      'dependencies': [
        'xulrunner_sdk',
        '<(DEPTH)/base/base.gyp:base',
        '<(libpagespeed_root)/third_party/jsmin/jsmin.gyp:jsmin',
      ],
      'sources': [
        'idl/IJsMin.idl',
        '<(js_min_root)/js_minifier.cc',
      ],
      'include_dirs': [
        '<(libpagespeed_root)',
      ],
      'export_dependent_settings': [
        '<(DEPTH)/base/base.gyp:base',
      ],
    },
    {
      'target_name': 'pagespeed_firefox_file_util',
      'type': '<(library)',
      'dependencies': [
        '<(DEPTH)/base/base.gyp:base',
        '<(DEPTH)/build/temp_gyp/googleurl.gyp:googleurl',
      ],
      'sources': [
        'cpp/pagespeed/file_util.cc',
      ],
      'include_dirs': [
        'cpp/pagespeed',
      ],
      'export_dependent_settings': [
        '<(DEPTH)/base/base.gyp:base',
        '<(DEPTH)/build/temp_gyp/googleurl.gyp:googleurl',
      ],
    },
    {
      'target_name': 'pagespeed_firefox_json_input',
      'type': '<(library)',
      'dependencies': [
        '<(DEPTH)/base/base.gyp:base',
        '<(DEPTH)/third_party/cJSON/cJSON.gyp:cJSON',
        '<(libpagespeed_root)/pagespeed/pagespeed.gyp:pagespeed_core',
      ],
      'sources': [
        'cpp/pagespeed/pagespeed_json_input.cc',
      ],
      'include_dirs': [
        'cpp/pagespeed',
      ],
      'export_dependent_settings': [
        '<(DEPTH)/base/base.gyp:base',
        '<(libpagespeed_root)/pagespeed/pagespeed.gyp:pagespeed_core',
      ],
    },
    {
      'target_name': 'pagespeed_firefox_test',
      'type': 'executable',
      'dependencies': [
        'pagespeed_firefox_file_util',
        'pagespeed_firefox_json_input',
        '<(DEPTH)/testing/gtest.gyp:gtest',
        '<(DEPTH)/testing/gtest.gyp:gtestmain',
      ],
      'sources': [
        'cpp/pagespeed/file_util_test.cc',
        'cpp/pagespeed/pagespeed_json_input_test.cc',
      ],
      'include_dirs': [
        'cpp/pagespeed',
      ],
    },
    {
      'target_name': 'pagespeed_firefox_activity_test',
      'type': 'executable',
      'dependencies': [
        'pagespeed_firefox_activity_common',
        'pagespeed_firefox_profile_pb',
        '<(DEPTH)/base/base.gyp:base',
        '<(DEPTH)/testing/gtest.gyp:gtest',
        '<(DEPTH)/testing/gtest.gyp:gtestmain',
      ],
      'sources': [
        'cpp/activity/call_graph_metadata_test.cc',
        'cpp/activity/call_graph_profile_test.cc',
        'cpp/activity/call_graph_test.cc',
        'cpp/activity/call_graph_timeline_event_set_test.cc',
        'cpp/activity/call_graph_timeline_visitor_test.cc',
        'cpp/activity/call_graph_util_test.cc',
        'cpp/activity/check_abort.cc',
        'cpp/activity/delayable_function_tree_view_delegate_test.cc',
        'cpp/activity/find_first_invocations_visitor_test.cc',
        'cpp/activity/timer_test.cc',
        'cpp/activity/uncalled_function_tree_view_delegate_test.cc',
      ],
      'include_dirs': [
        '<(DEPTH)',
        'cpp/activity',
      ],
      'defines': [
        'TEST_DIR_PATH="<(DEPTH)/pagespeed_firefox/cpp/activity/testdata/"',
      ],
    },
    {
      'target_name': 'pagespeed_firefox_library_rules',
      'type': '<(library)',
      'dependencies': [
        'xulrunner_sdk',
        'pagespeed_firefox_file_util',
        'pagespeed_firefox_json_input',
        '<(DEPTH)/base/base.gyp:base',
        '<(DEPTH)/build/temp_gyp/googleurl.gyp:googleurl',
        '<(libpagespeed_root)/pagespeed/pagespeed.gyp:pagespeed',
        '<(libpagespeed_root)/pagespeed/pagespeed.gyp:pagespeed_filters',
        '<(libpagespeed_root)/pagespeed/pagespeed.gyp:pagespeed_formatters',
      ],
      'sources': [
        'idl/IPageSpeedRules.idl',
        'cpp/pagespeed/firefox_dom.cc',
        'cpp/pagespeed/pagespeed_rules.cc',
      ],
      'include_dirs': [
        'cpp/pagespeed',
      ],
      'export_dependent_settings': [
        '<(DEPTH)/base/base.gyp:base',
        '<(libpagespeed_root)/pagespeed/pagespeed.gyp:pagespeed',
        '<(libpagespeed_root)/pagespeed/pagespeed.gyp:pagespeed_filters',
        '<(libpagespeed_root)/pagespeed/pagespeed.gyp:pagespeed_formatters',
      ],
    },
    {
      'target_name': 'pagespeed_firefox_module',
      'type': 'loadable_module',
      'variables': {
        'src_root': 'cpp',
      },
      'dependencies': [
        'xulrunner_sdk',
        'pagespeed_firefox_activity_gecko',
        'pagespeed_firefox_js_min',
        'pagespeed_firefox_library_rules',
      ],
      'defines': [
        'PAGESPEED_INCLUDE_LIBRARY_RULES',

        # HAVE_VISIBILITY_ATTRIBUTE instructs the Firefox module headers
        # to mark NS_GetModule with gcc attribute visibility("default").
        # Without this, Firefox would not be able to find its entry point
        # into our shared library. See http://gcc.gnu.org/wiki/Visibility
        # for additional information on the gcc visibility attribute.
        'HAVE_VISIBILITY_ATTRIBUTE=1',
      ],
      'sources': [
        '<(src_root)/pagespeed/pagespeed_module.cc',
      ],
      'include_dirs': [
        '<(src_root)',
        '<(src_root)/pagespeed',
      ],
      'conditions': [['OS == "mac"', {
        'xcode_settings': {
          # We must null out these two variables when building this target,
          # because it is a loadable_module (-bundle).
          'DYLIB_COMPATIBILITY_VERSION':'',
          'DYLIB_CURRENT_VERSION':'',
        }
      }]],
    },
  ],
}
