#include "llvh/Support/Locale.h"
#include "llvh/ADT/StringRef.h"
#include "llvh/Support/Unicode.h"

namespace llvh {
namespace sys {
namespace locale {

int columnWidth(StringRef Text) {
  return llvh::sys::unicode::columnWidthUTF8(Text);
}

bool isPrint(int UCS) {
  return llvh::sys::unicode::isPrintable(UCS);
}

} // namespace locale
} // namespace sys
} // namespace llvh
