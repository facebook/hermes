//===- WithColor.cpp ------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "llvh/Support/WithColor.h"
#include "llvh/Support/raw_ostream.h"
#include "llvh/Support/ManagedStatic.h"
#include "llvh/Support/DebugOptions.h"

using namespace llvh;

cl::OptionCategory &llvh::getColorCategory() {
  static cl::OptionCategory ColorCategory("Color Options");
  return ColorCategory;
}
namespace {
struct CreateUseColor {
  static void *call() {
    return new cl::opt<cl::boolOrDefault>(
        "color", cl::cat(getColorCategory()),
        cl::desc("Use colors in output (default=autodetect)"),
        cl::init(cl::BOU_UNSET));
  }
};
} // namespace
static ManagedStatic<cl::opt<cl::boolOrDefault>, CreateUseColor> UseColor;
void llvh::initWithColorOptions() { *UseColor; }

bool WithColor::colorsEnabled(raw_ostream &OS) {
  if (*UseColor == cl::BOU_UNSET)
    return OS.has_colors();
  return *UseColor == cl::BOU_TRUE;
}

WithColor::WithColor(raw_ostream &OS, HighlightColor Color) : OS(OS) {
  // Detect color from terminal type unless the user passed the --color option.
  if (colorsEnabled(OS)) {
    switch (Color) {
    case HighlightColor::Address:
      OS.changeColor(raw_ostream::YELLOW);
      break;
    case HighlightColor::String:
      OS.changeColor(raw_ostream::GREEN);
      break;
    case HighlightColor::Tag:
      OS.changeColor(raw_ostream::BLUE);
      break;
    case HighlightColor::Attribute:
      OS.changeColor(raw_ostream::CYAN);
      break;
    case HighlightColor::Enumerator:
      OS.changeColor(raw_ostream::MAGENTA);
      break;
    case HighlightColor::Macro:
      OS.changeColor(raw_ostream::RED);
      break;
    case HighlightColor::Error:
      OS.changeColor(raw_ostream::RED, true);
      break;
    case HighlightColor::Warning:
      OS.changeColor(raw_ostream::MAGENTA, true);
      break;
    case HighlightColor::Note:
      OS.changeColor(raw_ostream::BLACK, true);
      break;
    }
  }
}

raw_ostream &WithColor::error() { return error(errs()); }

raw_ostream &WithColor::warning() { return warning(errs()); }

raw_ostream &WithColor::note() { return note(errs()); }

raw_ostream &WithColor::error(raw_ostream &OS, StringRef Prefix) {
  if (!Prefix.empty())
    OS << Prefix << ": ";
  return WithColor(OS, HighlightColor::Error).get() << "error: ";
}

raw_ostream &WithColor::warning(raw_ostream &OS, StringRef Prefix) {
  if (!Prefix.empty())
    OS << Prefix << ": ";
  return WithColor(OS, HighlightColor::Warning).get() << "warning: ";
}

raw_ostream &WithColor::note(raw_ostream &OS, StringRef Prefix) {
  if (!Prefix.empty())
    OS << Prefix << ": ";
  return WithColor(OS, HighlightColor::Note).get() << "note: ";
}

WithColor::~WithColor() {
  if (colorsEnabled(OS))
    OS.resetColor();
}
