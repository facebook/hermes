/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

package com.hermes;

import com.squareup.javapoet.*;
import java.io.*;
import java.nio.file.Path;
import java.util.*;
import java.util.regex.Matcher;
import java.util.regex.Pattern;
import javax.lang.model.element.Modifier;
import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;
import javax.xml.xpath.XPath;
import javax.xml.xpath.XPathConstants;
import javax.xml.xpath.XPathExpressionException;
import javax.xml.xpath.XPathFactory;
import org.w3c.dom.Document;
import org.w3c.dom.NamedNodeMap;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;
import org.xml.sax.EntityResolver;
import org.xml.sax.InputSource;
import org.xml.sax.SAXException;

public class Main {

  private static Set<String> s_grandfatheredTagsRegular = new HashSet<>();

  // The fixed list of BCP 47 grandfathered language tags as in
  // https://www.rfc-editor.org/rfc/bcp/bcp47.txt
  private static Set<String> s_grandfatheredTagsRaw =
      new HashSet<>(
          Arrays.asList(
              "art-lojban",
              "cel-gaulish",
              "en-GB-oed",
              "i-ami",
              "i-bnn",
              "i-default",
              "i-enochian",
              "i-hak",
              "i-klingon",
              "i-lux",
              "i-mingo",
              "i-navajo",
              "i-pwn",
              "i-tao",
              "i-tay",
              "i-tsu",
              "no-bok",
              "no-nyn",
              "sgn-BE-FR",
              "sgn-BE-NL",
              "sgn-CH-DE",
              "zh-guoyu",
              "zh-hakka",
              "zh-min",
              "zh-min-nan",
              "zh-xiang"));

  // Unzipped CLDR
  private static String s_cldrPath = "E:\\cldr\\cldr-release-37";

  private static String readFileAsString(String filePath) throws IOException {
    StringBuffer fileData = new StringBuffer();
    BufferedReader reader = new BufferedReader(new FileReader(filePath));
    char[] buf = new char[1024];
    int numRead = 0;
    while ((numRead = reader.read(buf)) != -1) {
      String readData = String.valueOf(buf, 0, numRead);
      fileData.append(readData);
    }
    reader.close();
    return fileData.toString();
  }

  private static String resolveFilePath(String base, String relative) {
    String fullpath = Path.of(base, relative).toAbsolutePath().toString();
    return fullpath;
  }

  public static Document geCLDRSupplementalMetadataDOM()
      throws IOException, ParserConfigurationException, SAXException {
    DocumentBuilderFactory domfactory = DocumentBuilderFactory.newInstance();
    DocumentBuilder domBuilder = domfactory.newDocumentBuilder();

    domBuilder.setEntityResolver(
        new EntityResolver() {
          @Override
          public InputSource resolveEntity(String publicId, String systemId)
              throws SAXException, IOException {
            if (systemId.contains("ldmlSupplemental.dtd")) {
              return new InputSource(new StringReader(""));
            } else {
              return null;
            }
          }
        });

    return domBuilder.parse(
        new FileInputStream(
            resolveFilePath(s_cldrPath, "common/supplemental/supplementalMetadata.xml")));
  }

  public static NodeList getRegionAliasesDomNode(Document supplementalMetadataDOM)
      throws XPathExpressionException {
    XPath xPath = XPathFactory.newInstance().newXPath();
    String expression = "/supplementalData/metadata/alias/territoryAlias";
    NodeList nodeList =
        (NodeList)
            xPath.compile(expression).evaluate(supplementalMetadataDOM, XPathConstants.NODESET);

    return nodeList;
  }

  public static NodeList getLanguageAliasesDomNode(Document supplementalMetadataDOM)
      throws XPathExpressionException {
    XPath xPath = XPathFactory.newInstance().newXPath();
    String expression = "/supplementalData/metadata/alias/languageAlias";
    NodeList nodeList =
        (NodeList)
            xPath.compile(expression).evaluate(supplementalMetadataDOM, XPathConstants.NODESET);

    return nodeList;
  }

  public static void processLanguageAliasesDomNodestoHashMaps(
      NodeList languageAliasNodeList,
      Hashtable<String, String> grandfatheredTags,
      Hashtable<String, String> languageAliasWith2chars,
      Hashtable<String, String> languageAliasWith3chars,
      Hashtable<String, String[]> complexLanguageAliasMapWith2chars,
      Hashtable<String, String[]> complexLanguageAliasMapWith3chars) {

    for (int i = 0; i < languageAliasNodeList.getLength(); i++) {
      Node languageAliasNode = languageAliasNodeList.item(i);
      NamedNodeMap languageAliasAttributes = languageAliasNode.getAttributes();

      String type = languageAliasAttributes.getNamedItem("type").getNodeValue();
      String replacement = languageAliasAttributes.getNamedItem("replacement").getNodeValue();

      // Process grandfathered tags first.
      if (s_grandfatheredTagsRaw.contains(type)
          || s_grandfatheredTagsRegular.contains(type.replace('_', '-'))) {
        grandfatheredTags.put(type.replace('_', '-'), replacement);
      }

      // We are interesting in language-id replacement, not more complex mappings with other
      // subtags.
      if (!unicodeLanguageSubtagRegexCompiled.matcher(type).matches()) continue;

      if (unicodeLanguageSubtagRegexCompiled.matcher(replacement).matches()) {

        if (type.length() == 2) {
          languageAliasWith2chars.put(type, replacement);
        } else if (type.length() == 3) {
          languageAliasWith3chars.put(type, replacement);
        } else {
          // throw new RuntimeException("Invalid Language tag: " + type);
        }
      } else {
        System.out.println("Complex language mapping: " + type + " ::" + replacement);

        Matcher complexLanagueMatcher = unicodeLanguageIdRegexCompiled.matcher(replacement);

        if (complexLanagueMatcher.matches()) {

          String replacementLanguage = complexLanagueMatcher.group("language");
          String replacementScript = complexLanagueMatcher.group("script");
          String replacementRegion = complexLanagueMatcher.group("region");
          String replacementVariants = complexLanagueMatcher.group("variants");

          System.out.println(
              replacementLanguage
                  + ":"
                  + replacementScript
                  + ":"
                  + replacementRegion
                  + ":"
                  + replacementVariants);

          if (replacementVariants != null)
            throw new RuntimeException(
                "We don't support complex language replacements with variants");

          if (type.length() == 2) {
            complexLanguageAliasMapWith2chars.put(
                type, new String[] {replacementLanguage, replacementScript, replacementRegion});
          } else {
            complexLanguageAliasMapWith3chars.put(
                type, new String[] {replacementLanguage, replacementScript, replacementRegion});
          }
        }

        // Complex language mapping not yet implemented.
      }
    }
  }

  public static void processRegionAliasesDomNodestoHashMaps(
      NodeList regionAliasNodeList,
      Hashtable<String, String> regionAliasWith2chars,
      Hashtable<String, String> regionAliasWith3chars) {
    for (int i = 0; i < regionAliasNodeList.getLength(); i++) {
      Node regionAliasNode = regionAliasNodeList.item(i);
      NamedNodeMap regionAliasAttributes = regionAliasNode.getAttributes();

      String type = regionAliasAttributes.getNamedItem("type").getNodeValue();
      String replacement = regionAliasAttributes.getNamedItem("replacement").getNodeValue();

      if (unicodeRegionSubtagRegexCompiled.matcher(replacement).matches()) {

        if (type.length() == 2) {
          regionAliasWith2chars.put(type, replacement);
        } else if (type.length() == 3) {
          regionAliasWith3chars.put(type, replacement);
        } else {
          // throw new RuntimeException("Invalid region tag: " + type);
        }
      } else {
        // Complex region mapping not yet implemented.
      }
    }
  }

  public static void populateSimpleLangageAliasFileds(
      FieldSpec.Builder languageAliasKeysFieldBuilder,
      FieldSpec.Builder languageAliasReplacementsFieldBuilder,
      Hashtable<String, String> languageAliasMap) {
    ArrayList<String> languageAliasKeys = new ArrayList<String>(languageAliasMap.keySet());
    Collections.sort(languageAliasKeys);

    ArrayList<String> languageAliasReplacements = new ArrayList<String>();
    for (String key : languageAliasKeys) {
      languageAliasReplacements.add(languageAliasMap.get(key));
    }

    languageAliasKeysFieldBuilder
        .addModifiers(Modifier.FINAL, Modifier.PUBLIC, Modifier.STATIC)
        .initializer("{\"" + String.join("\",\"", languageAliasKeys) + "\"}")
        .build();

    languageAliasReplacementsFieldBuilder
        .addModifiers(Modifier.FINAL, Modifier.PUBLIC, Modifier.STATIC)
        .initializer("{\"" + String.join("\",\"", languageAliasReplacements) + "\"}")
        .build();
  }

  public static void populateGrandfatheredFileds(
      FieldSpec.Builder grandFatheredKeysFieldBuilder,
      FieldSpec.Builder grandfatheredReplacementsFieldBuilder,
      Hashtable<String, String> grandfatheredMap) {
    ArrayList<String> grandfatheredKeys = new ArrayList<String>(grandfatheredMap.keySet());
    Collections.sort(grandfatheredKeys);

    ArrayList<String> grandfatheredReplacements = new ArrayList<String>();
    for (String key : grandfatheredKeys) {
      grandfatheredReplacements.add(grandfatheredMap.get(key));
    }

    grandFatheredKeysFieldBuilder
        .addModifiers(Modifier.FINAL, Modifier.PUBLIC, Modifier.STATIC)
        .initializer("{\"" + String.join("\",\"", grandfatheredKeys) + "\"}")
        .build();

    grandfatheredReplacementsFieldBuilder
        .addModifiers(Modifier.FINAL, Modifier.PUBLIC, Modifier.STATIC)
        .initializer("{\"" + String.join("\",\"", grandfatheredReplacements) + "\"}")
        .build();
  }

  public static void populateComplexLangageAliasFileds(
      FieldSpec.Builder complexLanguageAliasKeysFieldBuilder,
      FieldSpec.Builder complexLanguageAliasReplacementsLanguageFieldBuilder,
      FieldSpec.Builder complexLanguageAliasReplacementsScriptFieldBuilder,
      FieldSpec.Builder complexLanguageAliasReplacementsRegionFieldBuilder,
      Hashtable<String, String[]> complexLanguageAliasMap) {
    ArrayList<String> languageAliasKeys = new ArrayList<String>(complexLanguageAliasMap.keySet());
    Collections.sort(languageAliasKeys);

    ArrayList<String> languageAliasReplacementsLanguage = new ArrayList<String>();
    ArrayList<String> languageAliasReplacementsScript = new ArrayList<String>();
    ArrayList<String> languageAliasReplacementsRegion = new ArrayList<String>();

    for (String key : languageAliasKeys) {
      String[] mappings = complexLanguageAliasMap.get(key);

      languageAliasReplacementsLanguage.add(mappings[0]);
      languageAliasReplacementsScript.add(mappings[1]);
      languageAliasReplacementsRegion.add(mappings[2]);
    }

    complexLanguageAliasKeysFieldBuilder
        .addModifiers(Modifier.FINAL, Modifier.PUBLIC, Modifier.STATIC)
        .initializer(
            "{\"" + String.join("\",\"", languageAliasKeys) + "\"}".replaceAll("\"null\"", "null"))
        .build();

    String languageReplacement =
        "{\"" + String.join("\",\"", languageAliasReplacementsLanguage) + "\"}";
    complexLanguageAliasReplacementsLanguageFieldBuilder
        .addModifiers(Modifier.FINAL, Modifier.PUBLIC, Modifier.STATIC)
        .initializer(languageReplacement.replaceAll("\"null\"", "null"))
        .build();

    String scriptReplacement =
        "{\"" + String.join("\",\"", languageAliasReplacementsScript) + "\"}";
    complexLanguageAliasReplacementsScriptFieldBuilder
        .addModifiers(Modifier.FINAL, Modifier.PUBLIC, Modifier.STATIC)
        .initializer(scriptReplacement.replace("\"null\"", "null"))
        .build();

    String regionReplacement =
        "{\"" + String.join("\",\"", languageAliasReplacementsRegion) + "\"}";
    complexLanguageAliasReplacementsRegionFieldBuilder
        .addModifiers(Modifier.FINAL, Modifier.PUBLIC, Modifier.STATIC)
        .initializer(regionReplacement.replaceAll("\"null\"", "null"))
        .build();
  }

  public static void populateRegionAliasFileds(
      FieldSpec.Builder regionAliasKeysFieldBuilder,
      FieldSpec.Builder regionAliasReplacementsFieldBuilder,
      Hashtable<String, String> regionAliasMap) {
    ArrayList<String> regionAliasKeys = new ArrayList<String>(regionAliasMap.keySet());
    Collections.sort(regionAliasKeys);

    ArrayList<String> regionAliasReplacements = new ArrayList<String>();
    for (String key : regionAliasKeys) {
      regionAliasReplacements.add(regionAliasMap.get(key));
    }

    regionAliasKeysFieldBuilder
        .addModifiers(Modifier.FINAL, Modifier.PUBLIC, Modifier.STATIC)
        .initializer("{\"" + String.join("\",\"", regionAliasKeys) + "\"}")
        .build();

    regionAliasReplacementsFieldBuilder
        .addModifiers(Modifier.FINAL, Modifier.PUBLIC, Modifier.STATIC)
        .initializer("{\"" + String.join("\",\"", regionAliasReplacements) + "\"}")
        .build();
  }

  private static String unicodeLanguageIdRegex =
      "^"
          + "(?<language>[a-z]{2,3}|[a-z]{5,8})"
          + "(?:(-|_)(?<script>[a-z]{4}))?"
          + "(?:(-|_)(?<region>([a-z]{2}|[0-9]{3})))?"
          + "(?<variants>((-|_)([a-z0-9]{5,8}|[0-9][a-z0-9]{3}))+)?"
          + "$";

  private static String unicodeLanguageSubtagRegex = "^" + "([a-z]{2,3}|[a-z]{5,8})" + "$";

  private static String unicodeRegionSubtagRegex = "^" + "([a-z]{2}|[0-9]{3})" + "$";

  private static Pattern unicodeLanguageIdRegexCompiled =
      Pattern.compile(unicodeLanguageIdRegex, Pattern.CASE_INSENSITIVE);
  private static Pattern unicodeLanguageSubtagRegexCompiled =
      Pattern.compile(unicodeLanguageSubtagRegex, Pattern.CASE_INSENSITIVE);
  private static Pattern unicodeRegionSubtagRegexCompiled =
      Pattern.compile(unicodeRegionSubtagRegex, Pattern.CASE_INSENSITIVE);

  public static void main(String[] args)
      throws IOException, ParserConfigurationException, SAXException, XPathExpressionException {

    //        String x = "{\"null\",\"null\",\"Latn\",\"null\",\"null\",\"null\"}";
    //        String y = x.replace("\"null\"", "null");
    //        System.out.println(y);
    //        System.exit(0);

    //        StringBuffer str = new StringBuffer("  abc def ");
    //        System.out.println("|" + str.toString() + "|" + str.length());
    //
    //        int idx = 0;
    //        while (str.charAt(idx) == ' ') idx ++;
    //        str.delete(0, idx);
    //        System.out.println("|" + str.toString() + "|" + str.length());
    //
    //        idx=str.length()-1;
    //        while(str.charAt(idx) == ' ') idx --;
    //        str.delete(idx+1, str.length());
    //        System.out.println("|" + str.toString() + "|" + str.length());

    // Prepare regular grandfathered tags.
    for (String grandfathered : s_grandfatheredTagsRaw) {
      if (unicodeLanguageIdRegexCompiled.matcher(grandfathered).matches()) {
        s_grandfatheredTagsRegular.add(grandfathered);
      }
    }

    Document supplementalMetadataDOM = geCLDRSupplementalMetadataDOM();

    NodeList languageAliasNodeList = getLanguageAliasesDomNode(supplementalMetadataDOM);

    Hashtable<String, String> regularGrandfatheredTags = new Hashtable<>();

    Hashtable<String, String> languageAliasMapWith2chars = new Hashtable<>();
    Hashtable<String, String> languageAliasMapWith3chars = new Hashtable<>();

    Hashtable<String, String[]> complexLanguageAliasMapWith2chars = new Hashtable<>();
    Hashtable<String, String[]> complexLanguageAliasMapWith3chars = new Hashtable<>();

    processLanguageAliasesDomNodestoHashMaps(
        languageAliasNodeList,
        regularGrandfatheredTags,
        languageAliasMapWith2chars,
        languageAliasMapWith3chars,
        complexLanguageAliasMapWith2chars,
        complexLanguageAliasMapWith3chars);

    FieldSpec.Builder regularGrandfatheredKeysFieldBuilder =
        FieldSpec.builder(String[].class, "regularGrandfatheredKeys");
    FieldSpec.Builder regularGrandfatheredReplacementsFieldBuilder =
        FieldSpec.builder(String[].class, "regularGrandfatheredReplacements");

    populateGrandfatheredFileds(
        regularGrandfatheredKeysFieldBuilder,
        regularGrandfatheredReplacementsFieldBuilder,
        regularGrandfatheredTags);

    FieldSpec.Builder languageAliasKeys2FieldBuilder =
        FieldSpec.builder(String[].class, "languageAliasKeys2");
    FieldSpec.Builder languageAliasReplacements2FieldBuilder =
        FieldSpec.builder(String[].class, "languageAliasReplacements2");

    populateSimpleLangageAliasFileds(
        languageAliasKeys2FieldBuilder,
        languageAliasReplacements2FieldBuilder,
        languageAliasMapWith2chars);

    FieldSpec.Builder languageAliasKeys3FieldBuilder =
        FieldSpec.builder(String[].class, "languageAliasKeys3");
    FieldSpec.Builder languageAliasReplacements3FieldBuilder =
        FieldSpec.builder(String[].class, "languageAliasReplacements3");
    populateSimpleLangageAliasFileds(
        languageAliasKeys3FieldBuilder,
        languageAliasReplacements3FieldBuilder,
        languageAliasMapWith3chars);

    FieldSpec.Builder complexLanguageAliasKeysField2Builder =
        FieldSpec.builder(String[].class, "complexLanguageAliasKeys2");
    FieldSpec.Builder complexLanguageAliasReplacementsLanguage2FieldBuilder =
        FieldSpec.builder(String[].class, "complexLanguageAliasReplacementsLanguage2");
    FieldSpec.Builder complexLanguageAliasReplacementsScript2FieldBuilder =
        FieldSpec.builder(String[].class, "complexLanguageAliasReplacementsScript2");
    FieldSpec.Builder complexLanguageAliasReplacementsRegion2FieldBuilder =
        FieldSpec.builder(String[].class, "complexLanguageAliasReplacementsRegion2");

    populateComplexLangageAliasFileds(
        complexLanguageAliasKeysField2Builder,
        complexLanguageAliasReplacementsLanguage2FieldBuilder,
        complexLanguageAliasReplacementsScript2FieldBuilder,
        complexLanguageAliasReplacementsRegion2FieldBuilder,
        complexLanguageAliasMapWith2chars);

    FieldSpec.Builder complexLanguageAliasKeysField3Builder =
        FieldSpec.builder(String[].class, "complexLanguageAliasKeys3");
    FieldSpec.Builder complexLanguageAliasReplacementsLanguage3FieldBuilder =
        FieldSpec.builder(String[].class, "complexLanguageAliasReplacementsLanguage3");
    FieldSpec.Builder complexLanguageAliasReplacementsScript3FieldBuilder =
        FieldSpec.builder(String[].class, "complexLanguageAliasReplacementsScript3");
    FieldSpec.Builder complexLanguageAliasReplacementsRegion3FieldBuilder =
        FieldSpec.builder(String[].class, "complexLanguageAliasReplacementsRegion3");

    populateComplexLangageAliasFileds(
        complexLanguageAliasKeysField3Builder,
        complexLanguageAliasReplacementsLanguage3FieldBuilder,
        complexLanguageAliasReplacementsScript3FieldBuilder,
        complexLanguageAliasReplacementsRegion3FieldBuilder,
        complexLanguageAliasMapWith3chars);

    NodeList regionAliasNodeList = getRegionAliasesDomNode(supplementalMetadataDOM);

    Hashtable<String, String> regionAliasMapWith2chars = new Hashtable<>();
    Hashtable<String, String> regionAliasMapWith3chars = new Hashtable<>();
    processRegionAliasesDomNodestoHashMaps(
        regionAliasNodeList, regionAliasMapWith2chars, regionAliasMapWith3chars);

    FieldSpec.Builder regionAliasKeys2FieldBuilder =
        FieldSpec.builder(String[].class, "regionAliasKeys2");
    FieldSpec.Builder regionAliasReplacements2FieldBuilder =
        FieldSpec.builder(String[].class, "regionAliasReplacements2");
    populateRegionAliasFileds(
        regionAliasKeys2FieldBuilder,
        regionAliasReplacements2FieldBuilder,
        regionAliasMapWith2chars);

    FieldSpec.Builder regionAliasKeys3FieldBuilder =
        FieldSpec.builder(String[].class, "regionAliasKeys3");
    FieldSpec.Builder regionAliasReplacements3FieldBuilder =
        FieldSpec.builder(String[].class, "regionAliasReplacements3");
    populateRegionAliasFileds(
        regionAliasKeys3FieldBuilder,
        regionAliasReplacements3FieldBuilder,
        regionAliasMapWith3chars);

    TypeSpec helloWorld =
        TypeSpec.classBuilder("LanguageTagsGenerated")
            .addModifiers(Modifier.PUBLIC, Modifier.FINAL)
            .addField(regularGrandfatheredKeysFieldBuilder.build())
            .addField(regularGrandfatheredReplacementsFieldBuilder.build())
            .addField(languageAliasKeys2FieldBuilder.build())
            .addField(languageAliasReplacements2FieldBuilder.build())
            .addField(languageAliasKeys3FieldBuilder.build())
            .addField(languageAliasReplacements3FieldBuilder.build())
            .addField(complexLanguageAliasKeysField2Builder.build())
            .addField(complexLanguageAliasReplacementsLanguage2FieldBuilder.build())
            .addField(complexLanguageAliasReplacementsScript2FieldBuilder.build())
            .addField(complexLanguageAliasReplacementsRegion2FieldBuilder.build())
            .addField(complexLanguageAliasKeysField3Builder.build())
            .addField(complexLanguageAliasReplacementsLanguage3FieldBuilder.build())
            .addField(complexLanguageAliasReplacementsScript3FieldBuilder.build())
            .addField(complexLanguageAliasReplacementsRegion3FieldBuilder.build())
            .addField(regionAliasKeys2FieldBuilder.build())
            .addField(regionAliasReplacements2FieldBuilder.build())
            .addField(regionAliasKeys3FieldBuilder.build())
            .addField(regionAliasReplacements3FieldBuilder.build())
            .build();

    JavaFile javaFile = JavaFile.builder("com.company", helloWorld).build();

    javaFile.writeTo(System.out);
  }
}
