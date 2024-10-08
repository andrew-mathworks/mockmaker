/*****************************************************************************
 *                                                                           *
 * Parse.hpp:                                                                *
 *                                                                           *
 * Simple container classes for parse data, either of an in-progress parsing *
 * or the final mocks generated a completed parsing.                         *
 *                                                                           *
 *****************************************************************************/

#ifndef __PARSE_HPP
#define __PARSE_HPP

#include <set>
#include <map>

#include "Mocks.hpp"

using namespace std;

/**
 * Result of a successful parsing: Mocked classes
 * and their necessary includes (if applicable).
 */
struct ParseResult {
  bool failed = false;
  int n_methods = 0;
  set<string> includes;
  map<string, MockedClass> mocks;

  ParseResult() {}
  ParseResult(bool failed) : failed(failed) {}
};

/**
 * Variables used by an in-progress parsing, including
 * a pointer to the parse result used as output.
 */
struct ParseState {
  bool continue_descending = true;
  bool has_namespace = false;
  string class_name;
  ParseResult *out;
  vector<string> namespaces;
  vector<string> *valid_namespaces;

  ParseState() : out(nullptr) {}
  ParseState(ParseState &ps)
    : has_namespace(ps.has_namespace)
    , class_name(ps.class_name)
    , out(ps.out)
    , namespaces(ps.namespaces)
    , valid_namespaces(ps.valid_namespaces) {}
  ParseState(ParseResult *out, vector<string> *valid_namespaces)
      : out(out), valid_namespaces(valid_namespaces) {
    if (valid_namespaces->empty()) has_namespace = true;
  }
};

#endif /* __PARSE_HPP */
