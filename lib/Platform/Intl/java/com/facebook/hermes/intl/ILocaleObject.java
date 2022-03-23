/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

package com.facebook.hermes.intl;

import java.util.ArrayList;
import java.util.HashMap;

public interface ILocaleObject<T> {
  ArrayList<String> getUnicodeExtensions(String key) throws JSRangeErrorException;

  HashMap<String, String> getUnicodeExtensions() throws JSRangeErrorException;

  void setUnicodeExtensions(String key, ArrayList<String> type) throws JSRangeErrorException;

  T getLocale() throws JSRangeErrorException;

  T getLocaleWithoutExtensions() throws JSRangeErrorException;

  String toCanonicalTag() throws JSRangeErrorException;

  String toCanonicalTagWithoutExtensions() throws JSRangeErrorException;

  ILocaleObject<T> cloneObject() throws JSRangeErrorException;
}
