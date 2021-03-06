/**
 * Copyright 2009 Google Inc.
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
 *
 * @fileoverview Provides a hook into the Page Speed native library.
 *
 * @author Matthew Steele
 */

(function() {  // Begin closure

/**
 * Create an array representing a list of headers from an object representing
 * those headers.
 * @param {object} headers_object An object mapping header names to values.
 * @return {array} An array of [name,value] pairs.
 */
function translateHeaders(headers_object) {
  var headersList = [];
  for (var key in headers_object) {
    headersList.push([key, headers_object[key]]);
  }
  return headersList;
}

/**
 * Given a format array from the results of PageSpeedRules, produce a formatted
 * HTML string.
 * @param {array} formatArray The format array.
 * @return {string} The resulting raw HTML.
 */
function buildHtml(formatArray) {
  var stringParts = [];
  for (var i = 0, length = formatArray.length; i < length; ++i) {
    var item = formatArray[i];
    if (item.type === "url") {
      stringParts.push('<a href="', item.value,
                       '" onclick="document.openLink(this);return false;">',
                       PAGESPEED.Utils.htmlEscape(item.alt || item.value),
                       '</a>');
    } else if (item.type === "str") {
      stringParts.push(PAGESPEED.Utils.htmlEscape(item.value));
    }
  }
  return stringParts.join('');
}

/**
 * Given an array of children of a result object from PageSpeedRules, produce a
 * formatted HTML string.
 * @param {array} children The children objects to format.
 * @param {boolean} opt_grand If true, these are (great-)*grandchildren.
 * @return {string} The resulting raw HTML.
 */
function formatChildren(children, opt_grand) {
  if (!children) {
    return null;
  }
  var warnings = [];
  for (var i = 0; i < children.length; ++i) {
    var child = children[i];
    warnings.push(buildHtml(child.format));
    if (child.children) {
      warnings.push(formatChildren(child.children, true));
    }
  }
  if (opt_grand) {
    return PAGESPEED.Utils.formatWarnings(warnings, /*allow-raw-html*/true);
  } else {
    return warnings.join('\n');
  }
}

// See http://code.google.com/p/page-speed/wiki/BeaconDocs
var shortNameTranslationTable = {
  CombineExternalCSS: 'CombineCSS',
  CombineExternalJavaScript: 'CombineJS',
  EnableGzipCompression: 'Gzip',
  MinifyJavaScript: 'MinifyJS',
  MinimizeDnsLookups: 'MinDns',
  MinimizeRedirects: 'MinRedirect',
  OptimizeImages: 'OptImgs',
  ServeResourcesFromAConsistentUrl: 'DupeRsrc'
};

/**
 * Given a list of result objects from the PageSpeedRules, create an array of
 * objects that look like LintRules.
 * @param {array} results An array of results from PageSpeedRules.
 * @return {array} An array of LintRule-like objects.
 */
function buildLintRuleResults(results) {
  var lintRules = [];
  if (results) {
    for (var i = 0; i < results.length; ++i) {
      var result = results[i];
      lintRules.push({
        name: buildHtml(result.format),
        shortName: shortNameTranslationTable[result.name] || result.name,
        score: result.score,
        weight: 3,
        href: result.url || '',
        warnings: formatChildren(result.children),
        information: null,
        getStatistics: function () { return result.stats || {}; }
      });
    }
  }
  return lintRules;
}

PAGESPEED.NativeLibrary = {

/**
 * Invoke the native library rules and return an array of LintRule-like
 * objects, or return an empty array if the library is unavailable.
 * @return {array} An array of LintRule-like objects.
 */
  invokeNativeLibraryRules: function () {
    var pagespeedRules = PAGESPEED.Utils.CCIN(
      '@code.google.com/p/page-speed/PageSpeedRules;1', 'IPageSpeedRules');
    if (!pagespeedRules) {
      return [];
    }

    var resources = [];
    var bodyInputStreams = [];
    var resourceURLs = PAGESPEED.Utils.getResources();
    for (var i = 0; i < resourceURLs.length; ++i) {
      var url = resourceURLs[i];
      var res_body_index = bodyInputStreams.length;
      bodyInputStreams.push(PAGESPEED.Utils.getResourceInputStream(url));
      resources.push({
        // TODO Add req_method, req_protocol, req_body, and res_protocol.
        req_headers: translateHeaders(PAGESPEED.Utils.getRequestHeaders(url)),
        res_status: PAGESPEED.Utils.getResponseCode(url),
        res_headers: translateHeaders(PAGESPEED.Utils.getResponseHeaders(url)),
        res_body: res_body_index,
        req_url: url
      });
    }

    var inputJSON = JSON.stringify(resources);
    var resultJSON = pagespeedRules.computeAndFormatResults(
      inputJSON,
      PAGESPEED.Utils.newNsIArray(bodyInputStreams),
      PAGESPEED.Utils.getOutputDir('library-output'));
    var results = JSON.parse(resultJSON);
    return buildLintRuleResults(results);
  }

};

})();  // End closure
