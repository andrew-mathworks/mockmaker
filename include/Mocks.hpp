/*****************************************************************************
 *                                                                           *
 * Mocks.hpp:                                                                *
 *                                                                           *
 * Classes for storing parsed data about a function or class to be mocked,   *
 * such as its name, return type, arguments, template parameters, etc.       *
 *                                                                           *
 *****************************************************************************/

#ifndef __MOCKS_HPP
#define __MOCKS_HPP

#include <algorithm>
#include <iostream>
#include <vector>
#include <format>
#include <string>
#include <clang-c/Index.h>

#include "Util.hpp"

using namespace std;

/**
 * A single mocked function within a mock class. Converts to a string of either
 * a constructor that calls the corresponding parent constructor, or a GMock
 * MOCK_METHODn macro call.
 */
struct MockedFunction {
  bool is_ctor = false;
  string type;
  string name;
  vector<string> arg_types;
  vector<string> typed_args;
  vector<string> untyped_args;

  MockedFunction() {}
  MockedFunction(CXCursor cursor, bool is_ctor) : is_ctor(is_ctor) {
    name = cxstring_to_string(clang_getCursorSpelling(cursor));

    auto cursor_type = clang_getCursorType(cursor);

    if (!is_ctor) {
      auto result_type = clang_getResultType(cursor_type);
      auto type_spelling = clang_getTypeSpelling(result_type);
      type = cxstring_to_string(type_spelling);
    }

    int num_args = max(0, clang_Cursor_getNumArguments(cursor));
    for (int i = 0; i < num_args; ++i) {
      auto arg_type = clang_getArgType(cursor_type, i);
      auto type_spelling = clang_getTypeSpelling(arg_type);
      string arg_data_type = cxstring_to_string(type_spelling);

      auto arg_cursor = clang_Cursor_getArgument(cursor, i);
      string arg_name = cxstring_to_string(
        clang_getCursorSpelling(arg_cursor)
      );

      arg_types.push_back(arg_data_type);
      typed_args.push_back(format(
        "{}{}{}", arg_data_type, arg_name.empty() ? "" : " ", arg_name
      ));
      untyped_args.push_back(arg_name);
    }
  }

  operator std::string() const {
    if (is_ctor) {
      return format(
        "Mock{0}({1}) : {0}({2}) {{}}",
        name,
        vec_join(typed_args, ", "),
        vec_join(untyped_args, ", ")
      );
    }

    // Ugly hack because overloaded operators cannot be mocked with GMock
    if (name.find("operator") != string::npos) {
      uint16_t n = rand();
      return format(
        "MOCK_METHOD{0}(Operator{1}, {2}({3}));\n        "
        "virtual {2} {4}({5}) {{ return Operator{1}({6}); }}",
        typed_args.size(),
        n,
        type,
        vec_join(arg_types, ", "),
        name,
        vec_join(typed_args, ", "),
        vec_join(untyped_args, ", ")
      );
    }

    return format(
      "MOCK_METHOD{}({}, {}({}));",
      typed_args.size(),
      name,
      type,
      vec_join(arg_types, ", ")
    );
  }
};
ostream &operator<<(ostream &lhs, const MockedFunction &rhs) {
  lhs << static_cast<string>(rhs);
  return lhs;
}

/**
 * A mocked class, storing by its name, qualified name (i.e. including
 * relevant namespaces), and mocked functions. Converts to a string of
 * a valid C++ class that can be used in test code.
 */
struct MockedClass {
  string name;
  string qualified_name;
  string templ;
  vector<MockedFunction> funcs;

  MockedClass() {}
  MockedClass(
    string name,
    vector<string> &namespaces,
    vector<string> &template_params
  ) : name(name) {
    string ns = vec_join(namespaces, "::");
    qualified_name = format("{}::{}", ns, name);

    if (template_params.size() == 0) templ = "";
    else {
      string t = vec_join(template_params, ", ");
      templ = format("template<{}>\n    ", t);
    }
  }

  void add_function(CXCursor cursor, bool is_ctor = false) {
    funcs.push_back(MockedFunction(cursor, is_ctor));
  }

  operator std::string() const {
    if (name.empty()) return "";

    vector<string> funcs_str;
    for (auto const& func : funcs) {
      funcs_str.push_back(func);
    }
    sort(funcs_str.begin(), funcs_str.end());
    funcs_str.erase(unique(funcs_str.begin(), funcs_str.end()), funcs_str.end());
    reverse(funcs_str.begin(), funcs_str.end());

    return format(
      "    {}class Mock{} : public {}\n"
      "    {{\n"
      "      public:\n"
      "        {}\n"
      "    }};\n",
      templ,
      name,
      qualified_name,
      vec_join(funcs_str, "\n        ")
    );
  }
};
ostream &operator<<(ostream &lhs, const MockedClass &rhs) {
  lhs << static_cast<string>(rhs);
  return lhs;
}

#endif /* __MOCKS_HPP */
