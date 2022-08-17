/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/BCGen/HBC/ConsecutiveStringStorage.h"
#include "hermes/BCGen/HBC/UniquingStringLiteralTable.h"
#include "hermes/Support/RegExpSerialization.h"
#include "hermes/VM/Predefined.h"

#include "gtest/gtest.h"

#include <cctype>
#include <string>
#include <vector>

using namespace hermes;

using llvh::StringRef;

namespace {

TEST(StringStorageTest, UniquingRegExpTest) {
  // Helper to create nonsense "bytecode" for a string.
  auto makeRegExp = [](const char *pat, const char *flags) -> CompiledRegExp {
    auto re = CompiledRegExp::tryCompile(pat, flags);
    EXPECT_TRUE(re.hasValue());
    return std::move(*re);
  };
  UniquingRegExpTable URET;
  auto idx0 = URET.addRegExp(makeRegExp("abc", "m"));
  auto idx1 = URET.addRegExp(makeRegExp("def", "m"));
  auto idx2 = URET.addRegExp(makeRegExp("abc", ""));
  auto idx3 = URET.addRegExp(makeRegExp("abc", "m"));

  EXPECT_EQ(0u, idx0);
  EXPECT_EQ(1u, idx1);
  EXPECT_EQ(2u, idx2);
  EXPECT_EQ(0u, idx3);
}

TEST(StringStorageTest, GetStringFromEntryTest) {
  std::vector<llvh::StringRef> strings = {"alpha", "beta", "", "\xE2\x84\xAB"};
  hbc::ConsecutiveStringStorage storage{strings};
  std::string utf8;
  for (uint32_t i = 0; i < strings.size(); i++) {
    EXPECT_EQ(strings[i], storage.getStringAtIndex(i, utf8));
  }
}

TEST(StringStorageTest, ConsecutiveStringStorageTest) {
  hbc::UniquingStringLiteralAccumulator USLA;

  USLA.addString("hello", /* isIdentifier */ false);
  USLA.addString("hello", /* isIdentifier */ false);
  USLA.addString("world", /* isIdentifier */ false);
  USLA.addString("some string", /* isIdentifier */ false);
  USLA.addString("", /* isIdentifier */ false);
  USLA.addString("hello", /* isIdentifier */ false);

  hbc::StringLiteralTable SLT =
      hbc::UniquingStringLiteralAccumulator::toTable(std::move(USLA));

  EXPECT_EQ(SLT.count(), 4);
  EXPECT_EQ(SLT.getStringID(""), 0);
  EXPECT_EQ(SLT.getStringID("hello"), 1);
  EXPECT_EQ(SLT.getStringID("some string"), 2);
  EXPECT_EQ(SLT.getStringID("world"), 3);

  std::string result;

  const char *hex = "0123456789ABCDEF";

  for (char ch : SLT.acquireStringStorage()) {
    if (std::isalpha(ch)) {
      result += ch;
    } else {
      result += "\\x";
      result += hex[ch & 0xf];
      result += hex[ch >> 4];
    }
  }

  EXPECT_EQ(result, "hellosome\\x02stringworld");
}

TEST(StringStorageTest, PackingStringStorageTest) {
  std::vector<llvh::StringRef> strings{"phab", "alphabet", "soup", "ou"};
  hbc::ConsecutiveStringStorage storage(strings);
  const auto data = storage.acquireStringStorage();
  llvh::StringRef dataAsStr((const char *)data.data(), data.size());
  EXPECT_EQ(dataAsStr.str(), "phabalphabetsoupou");
}

// Optimized string packing helper.
// We test that our storage does indeed contain strings at the location they
// claim, and that it is shorter than naive packing would imply.
// \param baseStrings is not empty when testing delta optimizing mode; we
// first construct ConsecutiveStringStorage from baseStrings to simulate that
// from the base bytecode, then pass it to the new ConsecutiveStringStorage.
static void test1OptimizingStringStorage(
    llvh::ArrayRef<llvh::StringRef> strings,
    int line,
    bool expectSmaller = true) {
  std::string info = " from test on line " + std::to_string(line);
  std::unique_ptr<hbc::ConsecutiveStringStorage> baseConsecutiveStrStorage;
  hbc::ConsecutiveStringStorage storage(strings, true /* optimize */);
  auto index = storage.acquireStringTable();
  auto data = storage.acquireStringStorage();
  llvh::StringRef dataAsString((const char *)data.data(), data.size());
  size_t idx = 0;
  for (const auto &p : index) {
    uint32_t offset = p.getOffset();
    uint32_t length = p.getLength();
    EXPECT_TRUE(offset + length >= offset && offset + length <= data.size())
        << " idx " << idx << info;
    EXPECT_EQ(
        dataAsString.slice(offset, offset + length).str(), strings[idx].str())
        << " idx " << idx << info;
    idx++;
  }
  // We should have consumed all strings.
  EXPECT_EQ(idx, strings.size()) << info;

  size_t naiveLength = 0;
  for (auto str : strings)
    naiveLength += str.size();
  if (expectSmaller) {
    EXPECT_LT(data.size(), naiveLength) << info;
  } else {
    EXPECT_LE(data.size(), naiveLength) << info;
  }
}

#define TEST_1_OPTIMIZING_STRING_STORAGE(s1, ...) \
  test1OptimizingStringStorage({s1, __VA_ARGS__}, __LINE__)

TEST(StringStorageTest, Optimizing) {
  TEST_1_OPTIMIZING_STRING_STORAGE("phab", "alphabet", "soup", "ou");
  TEST_1_OPTIMIZING_STRING_STORAGE(
      "ellitlavehr",
      "ranmellit",
      "nationsecern",
      "octpifine",
      "damnation",
      "octoutrun",
      "recoct",
      "utrunsecpar",
      "apteran",
      "avehr",
      "ampe",
      "dampedslyish",
      "unsticky",
      "reconsider",
      "octhyloid",
      "inetauten",
      "nlytelugu",
      "derdamped",
      "nsi",
      "inepanaka",
      "dershivoo",
      "vooopenly",
      "ernatrypa",
      "secparflaith",
      "ecoc",
      "octpeseta",
      "nationachime",
      "ationremass");
  TEST_1_OPTIMIZING_STRING_STORAGE(
      "tinfibrin",
      "tinach",
      "ien",
      "urikeelie",
      "eeliebandog",
      "ssushydroa",
      "nonarcing",
      "eliekirmew",
      "dogorienthostie",
      "cerebrology",
      "ienzymeoxhead",
      "fibrinfueler",
      "ebandogorient",
      "xheadmegrel",
      "inerreason",
      "uriviolal",
      "gdrawerpeptic",
      "ealprancy",
      "mammilliform",
      "nerimmane",
      "centauri",
      "tinachill",
      "plugdrawer",
      "hartin",
      "rcingveiner",
      "rmew",
      "rmewupseal",
      "taurienzyme",
      "oryssus");
  TEST_1_OPTIMIZING_STRING_STORAGE(
      "torula",
      "hecalpardaobeeish",
      "trothecalpardao",
      "calbricky",
      "nellatecultch",
      "ialretold",
      "gastrothecal",
      "eitfacula",
      "opticist",
      "vernier",
      "cky",
      "ulauploop",
      "cultchprefab",
      "ulabeloam",
      "cistwetted",
      "crenellate",
      "ementrhexis",
      "cementrhexis",
      "embracement",
      "efa",
      "lbric",
      "ula",
      "custodial",
      "cistdeceit",
      "oldevince");
  TEST_1_OPTIMIZING_STRING_STORAGE(
      "dhistenog",
      "olithprotea",
      "concerningly",
      "userhallow",
      "rangeman",
      "ophilebackie",
      "roadingtopply",
      "paus",
      "eback",
      "shiner",
      "atokapauser",
      "emanadance",
      "discolith",
      "railroading",
      "linguneven",
      "dingunlock",
      "photophile",
      "rote",
      "keeling",
      "pauserlaxism",
      "ply",
      "hilebatoka",
      "manawadhi",
      "pau");
  TEST_1_OPTIMIZING_STRING_STORAGE(
      "tpopatwirl",
      "rataurheen",
      "ingupline",
      "unclimb",
      "separata",
      "yangoftest",
      "uanwigger",
      "appboyang",
      "dewanship",
      "appafraid",
      "erabrookrohuna",
      "paganistic",
      "wiggerslough",
      "raidoutpop",
      "moeritherian",
      "shiploudly",
      "rabrookrohun",
      "ookfaluns",
      "poparnaut",
      "loudlydawish",
      "ticgenapp",
      "unattaining",
      "ining",
      "unsunspun",
      "iggerabrook",
      "suluan",
      "pun",
      "moeri",
      "moer",
      "herianoodles");
  TEST_1_OPTIMIZING_STRING_STORAGE(
      "larickfisher",
      "laricksulcal",
      "sulcalpallor",
      "iwi",
      "nistretook",
      "forthgo",
      "etookhowlet",
      "iwibaroco",
      "lde",
      "nistlarick",
      "inacrutch",
      "kiwiangina",
      "tretookisolde",
      "rocobronzy",
      "redfin",
      "ollercantle",
      "kiwikiwi",
      "annexation",
      "normanist",
      "thgosialis",
      "ronzyogboni",
      "gin",
      "lor",
      "ookhagged",
      "sherkoller",
      "anginainjure",
      "anistfemora",
      "nginasicula");
  TEST_1_OPTIMIZING_STRING_STORAGE(
      "oist",
      "unswallowable",
      "uplaid",
      "ipposeroot",
      "ocerialkiaugh",
      "keratodermia",
      "rmiasmugly",
      "cerialmoisty",
      "llumcultic",
      "culticstarer",
      "scabellum",
      "nerfacial",
      "bus",
      "lai",
      "icpimplasoekoe",
      "recruity",
      "rhinocerial",
      "maybush",
      "xiidbejade",
      "plai",
      "styahimsa",
      "mainerdolium",
      "almoistybecram",
      "mortmainer",
      "ushsheeny",
      "dbejadeulidia",
      "acialcixiid",
      "eenysachet",
      "facialhallah",
      "ityshippo",
      "culticpimpla",
      "ulticbrevet");
  TEST_1_OPTIMIZING_STRING_STORAGE(
      "lilycrakow",
      "arklebutoxy",
      "echuca",
      "tleblotch",
      "icrol",
      "trotlet",
      "letjoiner",
      "tletwaggel",
      "rklethirty",
      "astli",
      "onapebb",
      "monapebbly",
      "ghastlily",
      "nerassise",
      "ybillerpicrol",
      "ucaachage",
      "ilyrunted",
      "rtytantle",
      "sslycardia",
      "irtybiller",
      "crakowholmic",
      "ycrakowcabana",
      "anashewel",
      "rakowholm",
      "omonarefuge",
      "helplessly",
      "letcorema",
      "toromona",
      "kowholmicdarkle");
  TEST_1_OPTIMIZING_STRING_STORAGE(
      "insal",
      "becpratey",
      "gingrilawa",
      "eeri",
      "arto",
      "sheering",
      "chyverdoy",
      "untinsaluki",
      "botchy",
      "chiralmuntin",
      "teymothed",
      "ingchebec",
      "hiralpollam",
      "pugging",
      "ralmuntinaltaid",
      "shelfroom",
      "gingrefuel",
      "lukiknolly",
      "allochiral",
      "ollythrust",
      "heeri",
      "chartometer",
      "eri");
  TEST_1_OPTIMIZING_STRING_STORAGE(
      "celizchak",
      "soakeritonia",
      "derwi",
      "thrailcomply",
      "thraileloign",
      "indleintort",
      "assonant",
      "soakerzombie",
      "celizc",
      "laviclawful",
      "ortthrail",
      "antflavic",
      "windbindle",
      "astoncodder",
      "baculi",
      "antboruca",
      "ance",
      "ncelblowzy",
      "windbodier",
      "blowzyastare",
      "underwind",
      "soakerflurry",
      "ndbodi",
      "spancel",
      "haksoaker",
      "oniamythus",
      "antbaston",
      "odde");
  TEST_1_OPTIMIZING_STRING_STORAGE(
      "pingbutyne",
      "idalrerope",
      "ephalusstriae",
      "dalre",
      "midoidaltrusty",
      "tocephalusthemer",
      "horseback",
      "gustus",
      "pyramidoidal",
      "bekinkinite",
      "alust",
      "alusst",
      "tipproof",
      "tynethakur",
      "methylosis",
      "leptocephalus",
      "siscachou",
      "ethylosis",
      "initeoculus",
      "oidaltrus",
      "hooping",
      "cachou",
      "ebackkegler",
      "sebackcavity",
      "ylosisunrobe");
  TEST_1_OPTIMIZING_STRING_STORAGE(
      "hedoleose",
      "antproof",
      "regenesis",
      "dreary",
      "swingedalraun",
      "esiscreepy",
      "enesiswinged",
      "talgratis",
      "marital",
      "hedstolae",
      "reedtunica",
      "esisbreezy",
      "untwitched",
      "edo",
      "ezy",
      "aryhaikal",
      "arydebate",
      "osejereed");
  TEST_1_OPTIMIZING_STRING_STORAGE(
      "eatresysteigh",
      "risdungol",
      "manshivey",
      "eryfrasco",
      "eamrodent",
      "morris",
      "ogeejai",
      "geemagnum",
      "ogeejailer",
      "lenvoihaffet",
      "gra",
      "resydebate",
      "rislenvoi",
      "haffe",
      "hypogee",
      "erygraped",
      "treamcitric",
      "apedreveal",
      "dentbowery",
      "ogeeatresy",
      "anatolian",
      "restream",
      "odentteaman",
      "resyrobing");
  TEST_1_OPTIMIZING_STRING_STORAGE(
      "eterron",
      "eerfulwisher",
      "pinacle",
      "illmacusi",
      "spenc",
      "illoraler",
      "handbill",
      "essdietal",
      "stmazucacytost",
      "lete",
      "rolistmazuca",
      "stmazucacuorin",
      "oralermirage",
      "ucacuori",
      "petrolist",
      "cleterron",
      "inbokarktettix",
      "acuorinbokark",
      "uncheerful",
      "linten",
      "fungological",
      "raler",
      "illspence",
      "billstibic",
      "rron",
      "slimishness");
  TEST_1_OPTIMIZING_STRING_STORAGE(
      "daceloninae",
      "mismkismet",
      "semiminor",
      "mismkis",
      "basicburrah",
      "blepharocera",
      "minorrinker",
      "alanoctroy",
      "ker",
      "sanguinity",
      "smetnargil",
      "ismabasic",
      "aeatulwar",
      "eogaea",
      "smkis",
      "uinitytwaddy",
      "asicgallon",
      "agecigala",
      "argilniello",
      "crustalogical",
      "alanviolet",
      "noctroyrannel",
      "inaehavage",
      "atomism",
      "phthalan",
      "kismetmalati",
      "pharoc");
  TEST_1_OPTIMIZING_STRING_STORAGE(
      "ina",
      "rchamomis",
      "fulurinal",
      "rustful",
      "descar",
      "unstripped",
      "liere",
      "oyoreback",
      "gerfaucet",
      "ruelstamoyo",
      "rippedcruels",
      "schlieren",
      "fulinarch",
      "ust",
      "yoreb",
      "ppedescarp",
      "pilger",
      "omisminium",
      "miniummoiley",
      "deepmost",
      "amomisminium",
      "ucetminyan",
      "lierendrafty");
}

// Ensure we don't hang on very long strings.
TEST(StringStorageTest, NoHang) {
  std::string s1(16 * 1024 * 1024, 'a');
  std::string s2(16 * 1024 * 1024, 'b');
  llvh::StringRef sr1 = s1;
  llvh::StringRef sr2 = s2;
  test1OptimizingStringStorage({sr1, sr2}, __LINE__, false);
}

/// \return The table resulting from adding the strings in \p strings into the
/// accumulator \p accum (defaults to empty), and converting it into a table
/// with optimizations enabled.
hbc::StringLiteralTable tableForStrings(
    llvh::ArrayRef<llvh::StringRef> strings,
    hbc::UniquingStringLiteralAccumulator accum = {}) {
  for (auto str : strings) {
    accum.addString(str, /* isIdentifier */ false);
  }

  return hbc::UniquingStringLiteralAccumulator::toTable(
      std::move(accum), /* optimize */ true);
}

TEST(StringStorageTest, DeltaOptimizingModeTest) {
  std::vector<llvh::StringRef> baseStrings = {
      "ellitlavehr", "ranmellit",  "nationsecern", "octpifine",
      "damnation",   "octoutrun",  "recoct",       "utrunsecpar",
      "apteran",     "avehr",      "ampe",         "dampedslyish",
      "unsticky",    "reconsider", "octhyloid",    "inetauten",
      "nlytelugu",   "derdamped",  "nsi",          "inepanaka",
      "dershivoo",   "vooopenly",  "ernatrypa",    "secparflaith",
      "ecoc",        "octpeseta",  "nationachime", "ationremass"};

  auto baseTable = tableForStrings(baseStrings);
  std::vector<unsigned char> baseBuffer = baseTable.acquireStringStorage();
  std::vector<StringTableEntry> baseEntries = baseTable.acquireStringTable();

  // Create a new table starting with the base storage.
  std::vector<llvh::StringRef> newStrings = {
      "ina",          "rchamomis",   "fulurinal",    "rustful",
      "descar",       "unstripped",  "liere",        "oyoreback",
      "gerfaucet",    "ruelstamoyo", "rippedcruels", "schlieren",
      "fulinarch",    "ust",         "yoreb",        "ppedescarp",
      "pilger",       "omisminium",  "miniummoiley", "deepmost",
      "amomisminium", "ucetminyan",  "lierendrafty"};

  hbc::UniquingStringLiteralAccumulator baseAccumulator{
      hbc::ConsecutiveStringStorage{
          std::vector<StringTableEntry>{baseEntries},
          std::vector<unsigned char>{baseBuffer}},
      std::vector<bool>(baseEntries.size(), false)};

  auto newTable = tableForStrings(newStrings, std::move(baseAccumulator));

  // Verify that the new storage buffer starts with the base storage buffer.
  auto newBuffer = newTable.acquireStringStorage();
  ASSERT_TRUE(newBuffer.size() > baseBuffer.size());
  EXPECT_TRUE(
      std::equal(baseBuffer.begin(), baseBuffer.end(), newBuffer.begin()));

  // Verify that all base IDs are the same.
  for (auto str : baseStrings) {
    EXPECT_EQ(baseTable.getStringID(str), newTable.getStringID(str));
  }
}

TEST(StringAccumulatorTest, Ordering) {
  // When outputing a string storage instance, the accumulator sorts its index
  // entries.  First strings get grouped into "frequency classes".  The first
  // group contains the top 2^8 strings by number of usages of that string as
  // an identifier, the second group is the next (2^16 - 2^8) hottest strings,
  // and the last group is all the remaining strings.  Then within each class,
  // strings are further subdivided into the following categories (emitted in
  // the given order):
  //
  // - Strings that are not identifiers.
  // - Strings that are identifiers.
  //
  // Finally, within each category, strings are sorted first by the offset of
  // their character buffer in the string storage, and then by their size.
  //
  // This test verified the grouping into categories.

  hbc::UniquingStringLiteralAccumulator USLA;

  USLA.addString("Object", /* isIdentifier */ true);
  USLA.addString("String", /* isIdentifier */ false);
  USLA.addString("Id0", /* isIdentifier */ true);
  USLA.addString("Str0", /* isIdentifier */ false);
  USLA.addString("Str1", /* isIdentifier */ false);
  USLA.addString("Id1", /* isIdentifier */ true);
  USLA.addString("Function", /* isIdentifier */ true);
  USLA.addString("Str2", /* isIdentifier */ false);
  USLA.addString("Id2", /* isIdentifier */ true);

  auto SLT = hbc::UniquingStringLiteralAccumulator::toTable(
      std::move(USLA), /* optimize */ false);

  std::vector<llvh::StringRef> expectedStrings{
      "Str0",
      "Str1",
      "Str2",
      "String",
      "Function",
      "Id0",
      "Id1",
      "Id2",
      "Object",
  };

  for (size_t i = 0; i < expectedStrings.size(); ++i) {
    EXPECT_EQ(i, SLT.getStringID(expectedStrings[i])) << "at index " << i;
  }

  std::vector<StringKind::Entry> expectedKinds{
      {StringKind::String, 4},
      {StringKind::Identifier, 5},
  };

  auto actualKinds = SLT.getStringKinds();
  EXPECT_EQ(actualKinds.size(), expectedKinds.size());
  for (size_t i = 0; i < expectedKinds.size(); ++i) {
    EXPECT_EQ(expectedKinds[i].kind(), actualKinds[i].kind())
        << "at index " << i;
    EXPECT_EQ(expectedKinds[i].count(), actualKinds[i].count())
        << "at index " << i;
  }
}

} // end anonymous namespace
