/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "llvh/ADT/StringSwitch.h"
#include "llvh/Support/raw_ostream.h"

#include <map>
#include <vector>

enum class FieldType {
  Error,
  NodeString,
  NodeLabel,
  Boolean,
  Number,
  NodePtr,
  NodeList
};
static const llvh::StringLiteral typeName_[][2] = {
    {"**ERROR**", "**ERROR**"},
    {"NodeString", "NodeStringOpt"},
    {"NodeLabel", "NodeLabelOpt"},
    {"bool", "bool"},
    {"f64", "f64"},
    {"NodePtr", "NodePtrOpt"},
    {"NodeListRef", "NodeListOptRef"},
};

struct Field {
  FieldType type;
  std::string name;
  bool optional;

  Field(FieldType type, std::string const &name, bool optional)
      : type(type), name(name), optional(optional) {}

  llvh::StringRef typeName() const {
    return typeName_[(int)type][optional];
  }

  std::string rustName() const {
    if (name == "async" || name == "await" || name == "static")
      return "is_" + name;

    bool fix = false;
    for (char c : name) {
      if (isupper(c)) {
        fix = true;
        break;
      }
    }
    if (!fix)
      return name;

    std::string res;
    for (char c : name) {
      if (isupper(c)) {
        res.push_back('_');
        res.push_back(tolower(c));
      } else {
        res.push_back(c);
      }
    }
    return res;
  }
};

enum class SentinelType {
  None,
  First,
  Last,
};

struct TreeClass {
  std::string name;
  std::string base;
  SentinelType sentinel;
  std::vector<Field> fields;

  TreeClass(
      std::string const &name,
      std::string const &base,
      SentinelType sentinel = SentinelType::None)
      : name(name), base(base), sentinel(sentinel) {}

  std::string enumName() const {
    if (sentinel == SentinelType::First)
      return "_" + name + "First";
    else if (sentinel == SentinelType::Last)
      return "_" + name + "Last";
    else
      return name;
  }

  std::string className() const {
    return name + "Node";
  }

  llvh::Optional<std::string> getDecoration() const {
    // FIXME: obviously this is just an ugly hardcoded hack.
    bool haveDecoration = false;
    if (sentinel == SentinelType::First) {
      haveDecoration = llvh::StringSwitch<bool>(name)
                           .Case("Statement", false)
                           .Case("CallExpressionLike", false)
                           .Case("MemberExpressionLike", false)
                           .Case("Pattern", false)
                           .Case("Cover", false)
                           .Default(true);
    } else if (sentinel == SentinelType::None) {
      haveDecoration = llvh::StringSwitch<bool>(name)
                           .Case("BlockStatement", true)
                           .Case("BreakStatement", true)
                           .Case("ContinueStatement", true)
                           .Case("SwitchStatement", true)
                           .Case("LabeledStatement", true)
                           .Case("Program", true)
                           .Default(false);
    }
    if (!haveDecoration)
      return llvh::None;
    return name + "Decoration";
  }
};

static std::vector<TreeClass> treeClasses_{};
static std::map<std::string, size_t> treeNames_{};

static void initClasses() {
  auto baseName = [](llvh::StringRef n) { return n == "Base" ? "" : n; };
  auto type = [](llvh::StringRef n) {
    return llvh::StringSwitch<FieldType>(n)
        .Case("NodeString", FieldType::NodeString)
        .Case("NodeLabel", FieldType::NodeLabel)
        .Case("NodeBoolean", FieldType::Boolean)
        .Case("NodeNumber", FieldType::Number)
        .Case("NodePtr", FieldType::NodePtr)
        .Case("NodeList", FieldType::NodeList)
        .Default(FieldType::Error);
  };

#define STRUCT(NAME, BASE)                           \
  treeClasses_.emplace_back(#NAME, baseName(#BASE)); \
  treeNames_.emplace(#NAME, treeClasses_.size() - 1)

#define ARG(TY, NM, OPT) \
  treeClasses_.back().fields.emplace_back(type(#TY), #NM, OPT)

#define ESTREE_FIRST(NAME, BASE)                                          \
  treeClasses_.emplace_back(#NAME, baseName(#BASE), SentinelType::First); \
  treeNames_.emplace(#NAME, treeClasses_.size() - 1);

#define ESTREE_LAST(NAME) \
  treeClasses_.emplace_back(#NAME, "", SentinelType::Last);

#define ESTREE_NODE_0_ARGS(NAME, BASE) STRUCT(NAME, BASE);

#define ESTREE_NODE_1_ARGS(NAME, BASE, ARG0TY, ARG0NM, ARG0OPT) \
  STRUCT(NAME, BASE);                                           \
  ARG(ARG0TY, ARG0NM, ARG0OPT);

#define ESTREE_NODE_2_ARGS(                                       \
    NAME, BASE, ARG0TY, ARG0NM, ARG0OPT, ARG1TY, ARG1NM, ARG1OPT) \
  STRUCT(NAME, BASE);                                             \
  ARG(ARG0TY, ARG0NM, ARG0OPT);                                   \
  ARG(ARG1TY, ARG1NM, ARG1OPT);

#define ESTREE_NODE_3_ARGS(     \
    NAME,                       \
    BASE,                       \
    ARG0TY,                     \
    ARG0NM,                     \
    ARG0OPT,                    \
    ARG1TY,                     \
    ARG1NM,                     \
    ARG1OPT,                    \
    ARG2TY,                     \
    ARG2NM,                     \
    ARG2OPT)                    \
  STRUCT(NAME, BASE);           \
  ARG(ARG0TY, ARG0NM, ARG0OPT); \
  ARG(ARG1TY, ARG1NM, ARG1OPT); \
  ARG(ARG2TY, ARG2NM, ARG2OPT);

#define ESTREE_NODE_4_ARGS(     \
    NAME,                       \
    BASE,                       \
    ARG0TY,                     \
    ARG0NM,                     \
    ARG0OPT,                    \
    ARG1TY,                     \
    ARG1NM,                     \
    ARG1OPT,                    \
    ARG2TY,                     \
    ARG2NM,                     \
    ARG2OPT,                    \
    ARG3TY,                     \
    ARG3NM,                     \
    ARG3OPT)                    \
  STRUCT(NAME, BASE);           \
  ARG(ARG0TY, ARG0NM, ARG0OPT); \
  ARG(ARG1TY, ARG1NM, ARG1OPT); \
  ARG(ARG2TY, ARG2NM, ARG2OPT); \
  ARG(ARG3TY, ARG3NM, ARG3OPT);

#define ESTREE_NODE_5_ARGS(     \
    NAME,                       \
    BASE,                       \
    ARG0TY,                     \
    ARG0NM,                     \
    ARG0OPT,                    \
    ARG1TY,                     \
    ARG1NM,                     \
    ARG1OPT,                    \
    ARG2TY,                     \
    ARG2NM,                     \
    ARG2OPT,                    \
    ARG3TY,                     \
    ARG3NM,                     \
    ARG3OPT,                    \
    ARG4TY,                     \
    ARG4NM,                     \
    ARG4OPT)                    \
  STRUCT(NAME, BASE);           \
  ARG(ARG0TY, ARG0NM, ARG0OPT); \
  ARG(ARG1TY, ARG1NM, ARG1OPT); \
  ARG(ARG2TY, ARG2NM, ARG2OPT); \
  ARG(ARG3TY, ARG3NM, ARG3OPT); \
  ARG(ARG4TY, ARG4NM, ARG4OPT);

#define ESTREE_NODE_6_ARGS(     \
    NAME,                       \
    BASE,                       \
    ARG0TY,                     \
    ARG0NM,                     \
    ARG0OPT,                    \
    ARG1TY,                     \
    ARG1NM,                     \
    ARG1OPT,                    \
    ARG2TY,                     \
    ARG2NM,                     \
    ARG2OPT,                    \
    ARG3TY,                     \
    ARG3NM,                     \
    ARG3OPT,                    \
    ARG4TY,                     \
    ARG4NM,                     \
    ARG4OPT,                    \
    ARG5TY,                     \
    ARG5NM,                     \
    ARG5OPT)                    \
  STRUCT(NAME, BASE);           \
  ARG(ARG0TY, ARG0NM, ARG0OPT); \
  ARG(ARG1TY, ARG1NM, ARG1OPT); \
  ARG(ARG2TY, ARG2NM, ARG2OPT); \
  ARG(ARG3TY, ARG3NM, ARG3OPT); \
  ARG(ARG4TY, ARG4NM, ARG4OPT); \
  ARG(ARG5TY, ARG5NM, ARG5OPT);

#define ESTREE_NODE_7_ARGS(     \
    NAME,                       \
    BASE,                       \
    ARG0TY,                     \
    ARG0NM,                     \
    ARG0OPT,                    \
    ARG1TY,                     \
    ARG1NM,                     \
    ARG1OPT,                    \
    ARG2TY,                     \
    ARG2NM,                     \
    ARG2OPT,                    \
    ARG3TY,                     \
    ARG3NM,                     \
    ARG3OPT,                    \
    ARG4TY,                     \
    ARG4NM,                     \
    ARG4OPT,                    \
    ARG5TY,                     \
    ARG5NM,                     \
    ARG5OPT,                    \
    ARG6TY,                     \
    ARG6NM,                     \
    ARG6OPT)                    \
  STRUCT(NAME, BASE);           \
  ARG(ARG0TY, ARG0NM, ARG0OPT); \
  ARG(ARG1TY, ARG1NM, ARG1OPT); \
  ARG(ARG2TY, ARG2NM, ARG2OPT); \
  ARG(ARG3TY, ARG3NM, ARG3OPT); \
  ARG(ARG4TY, ARG4NM, ARG4OPT); \
  ARG(ARG5TY, ARG5NM, ARG5OPT); \
  ARG(ARG6TY, ARG6NM, ARG6OPT);

#define ESTREE_NODE_8_ARGS(     \
    NAME,                       \
    BASE,                       \
    ARG0TY,                     \
    ARG0NM,                     \
    ARG0OPT,                    \
    ARG1TY,                     \
    ARG1NM,                     \
    ARG1OPT,                    \
    ARG2TY,                     \
    ARG2NM,                     \
    ARG2OPT,                    \
    ARG3TY,                     \
    ARG3NM,                     \
    ARG3OPT,                    \
    ARG4TY,                     \
    ARG4NM,                     \
    ARG4OPT,                    \
    ARG5TY,                     \
    ARG5NM,                     \
    ARG5OPT,                    \
    ARG6TY,                     \
    ARG6NM,                     \
    ARG6OPT,                    \
    ARG7TY,                     \
    ARG7NM,                     \
    ARG7OPT)                    \
  STRUCT(NAME, BASE);           \
  ARG(ARG0TY, ARG0NM, ARG0OPT); \
  ARG(ARG1TY, ARG1NM, ARG1OPT); \
  ARG(ARG2TY, ARG2NM, ARG2OPT); \
  ARG(ARG3TY, ARG3NM, ARG3OPT); \
  ARG(ARG4TY, ARG4NM, ARG4OPT); \
  ARG(ARG5TY, ARG5NM, ARG5OPT); \
  ARG(ARG6TY, ARG6NM, ARG6OPT); \
  ARG(ARG7TY, ARG7NM, ARG7OPT);

#include "hermes/AST/ESTree.def"
}

/*
/// Get all decorations for a class, recursively.
void getAllDecorations(
    const TreeClass &cls,
    std::vector<std::string> &decs) {
  if (!cls.base.empty())
    getAllDecorations(treeClasses_[treeNames_.find(cls.base)->second], decs);

  auto decoration = cls.getDecoration();
  if (decoration)
    decs.push_back(std::move(*decoration));
}
*/

static void genGetters() {
  auto genStruct = [](const TreeClass &cls) {
    if (cls.sentinel != SentinelType::None)
      return;
    if (cls.fields.empty())
      return;
    llvh::outs() << "    // " << cls.name << "\n";
    for (const auto &fld : cls.fields) {
      llvh::outs() << "    pub fn hermes_get_" << cls.name << "_" << fld.name
                   << "(node: NodePtr) -> ";
      llvh::outs() << fld.typeName() << ";\n";
    }
  };

  llvh::outs() << "extern \"C\" {\n";
  for (const auto &cls : treeClasses_) {
    if (cls.sentinel != SentinelType::None)
      continue;
    genStruct(cls);
  }
  llvh::outs() << "}\n";
}

static void genConvert() {
  llvh::outs() << "pub unsafe fn cvt_node_ptr<'parser, 'gc>(\n"
                  "  cvt: &mut Converter<'parser>, \n"
                  "  gc: &'gc ast::GCLock, \n"
                  "  n: NodePtr) -> &'gc ast::Node<'gc> {\n";
  llvh::outs() << "    let nr = n.as_ref();\n"
                  "    let range = ast::SourceRange {\n"
                  "        file: cvt.file_id,\n"
                  "        start: cvt.cvt_smloc(nr.source_range.start),\n"
                  "        end: ast::SourceLoc::invalid(),\n"
                  "    };\n"
                  "\n";

  llvh::outs() << "    let res = match nr.kind {\n";

  auto genStruct = [](const TreeClass &cls) {
    if (cls.sentinel != SentinelType::None)
      return;
    if (strncmp(cls.name.c_str(), "Cover", 5) == 0)
      return;

    llvh::outs() << "        NodeKind::" << cls.name << " => {\n";

    // Declare all the fields as local vars to avoid multiple borrows
    // of the context.
    for (const auto &fld : cls.fields) {
      llvh::outs() << "          let " << fld.rustName() << " = ";
      bool close = true;
      switch (fld.type) {
        case FieldType::NodeString:
          llvh::outs() << "cvt.cvt_string" << (fld.optional ? "_opt" : "")
                       << "(gc, ";
          break;
        case FieldType::NodeLabel:
          if ((cls.name == "UnaryExpression" && fld.name == "operator") ||
              (cls.name == "BinaryExpression" && fld.name == "operator") ||
              (cls.name == "LogicalExpression" && fld.name == "operator") ||
              (cls.name == "UpdateExpression" && fld.name == "operator") ||
              (cls.name == "VariableDeclaration" && fld.name == "kind") ||
              (cls.name == "Property" && fld.name == "kind") ||
              (cls.name == "MethodDefinition" && fld.name == "kind") ||
              (cls.name == "ImportDeclaration" && fld.name == "importKind") ||
              (cls.name == "ImportSpecifier" && fld.name == "importKind") ||
              (cls.name == "ExportNamedDeclaration" &&
               fld.name == "exportKind") ||
              (cls.name == "ExportAllDeclaration" &&
               fld.name == "exportKind") ||
              (cls.name == "AssignmentExpression" && fld.name == "operator"))
            llvh::outs() << "cvt_enum(";
          else
            llvh::outs() << "cvt.cvt_label" << (fld.optional ? "_opt" : "")
                         << "(gc, ";
          break;
        case FieldType::Boolean:
        case FieldType::Number:
        default:
          close = false;
          break;
        case FieldType::NodePtr:
          llvh::outs() << "cvt_node_ptr" << (fld.optional ? "_opt" : "")
                       << "(cvt, gc, ";
          break;
        case FieldType::NodeList:
          llvh::outs() << "cvt_node_list" << (fld.optional ? "_opt" : "")
                       << "(cvt, gc, ";
          break;
      }
      llvh::outs() << "hermes_get_" << cls.name << "_" << fld.name << "(n)";
      if (close)
        llvh::outs() << ")";
      llvh::outs() << ";\n";
    }

    llvh::outs()
        << "          let mut template = ast::template::" << cls.name << " {\n"
        << "              metadata: ast::TemplateMetadata {range, ..Default::default()},\n";

    for (const auto &fld : cls.fields) {
      // Shorthand initialization of each field.
      llvh::outs() << "                  " << fld.rustName() << ",\n";
    }

    llvh::outs()
        << "          };\n" // kind
           "          template.metadata.range.end = "
           // Empty source range would result in out of bounds on the left
           // side if we subtracted 1 from the end, so we just copy the start
           // location.
           "if nr.source_range.is_empty() { template.metadata.range.start } "
           "else { cvt.cvt_smloc(nr.source_range.end.pred()) };\n"
        << "          ast::builder::" << cls.name
        << "::build_template(gc, template)\n"
        << "        }\n"; // match block
  };

  for (const auto &cls : treeClasses_) {
    if (cls.sentinel != SentinelType::None)
      continue;
    genStruct(cls);
  }

  llvh::outs()
      << "        _ => {\n"
         "          cvt.report_invalid_node(gc, n, range);\n"
         "          let template = ast::template::Empty {\n"
         "            metadata: ast::TemplateMetadata {range, ..Default::default()}\n"
         "          };\n"
         "          ast::builder::Empty::build_template(gc, template)\n"
         "        }\n"

         "    };\n\n";
  llvh::outs() << "    res\n";
  llvh::outs() << "}\n";
}

static void genEnum() {
  llvh::outs() << "#[repr(u32)]\n";
  llvh::outs() << "#[derive(Debug, PartialEq)]\n";
  llvh::outs() << "pub enum NodeKind {\n";
  for (const auto &cls : treeClasses_) {
    llvh::outs() << "    " << cls.enumName();
    llvh::outs() << ",\n";
  }
  llvh::outs() << "}\n";
}

int main(int argc, char **argv) {
  if (argc != 2) {
    llvh::errs() << "syntax: " << argv[0] << " ffi|cvt\n";
    return 1;
  }

  initClasses();

  llvh::outs()
      << "/*\n"
         " * Copyright (c) Meta Platforms, Inc. and affiliates.\n"
         " *\n"
         " * This source code is licensed under the MIT license found in the\n"
         " * LICENSE file in the root directory of this source tree.\n"
         " */\n\n";
  llvh::outs() << "// @"
                  "generated by Hermes rustgen\n";
  llvh::outs() << "// DO NOT EDIT\n\n";

  if (llvh::StringRef(argv[1]) == "ffi") {
    llvh::outs() << "use super::node::*;\n\n";

    genEnum();
    llvh::outs() << "\n";
    genGetters();
  } else if (llvh::StringRef(argv[1]) == "cvt") {
    llvh::outs() << "use hermes::parser::*;\n"
                    "use super::convert::*;\n"
                    "use crate::ast;\n\n";

    genConvert();
  } else {
    llvh::errs() << "Invalid command\n";
    return 1;
  }

  return 0;
}
