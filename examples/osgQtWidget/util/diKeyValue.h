/*
  Diana - A Free Meteorological Visualisation Tool

  Copyright (C) 2018 met.no

  Contact information:
  Norwegian Meteorological Institute
  Box 43 Blindern
  0313 OSLO
  NORWAY
  email: diana@met.no

  This file is part of Diana

  Diana is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  Diana is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Diana; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef DIKEYVALUE_H
#define DIKEYVALUE_H

#include <iosfwd>
#include <string>
#include <vector>

namespace miutil {

class KeyValue {
public:
  KeyValue();
  explicit KeyValue(const std::string& k);
  explicit KeyValue(const std::string& k, const std::string& v, bool keptQuotes=false);

  const std::string& key() const
    { return mKey; }
  const std::string& value() const
    { return mValue; }

  bool hasValue() const
    { return mHasValue; }
  bool keptQuotes() const
    { return mKeptQuotes; }

  int toInt(bool& ok, int def=0) const;
  int toInt(int def=0) const
    { bool ok; return toInt(ok, def); }

  double toDouble(bool& ok, double def=0) const;
  double toDouble(double def=0) const
    { bool ok; return toDouble(ok, def); }

  float toFloat(bool& ok, float def=0) const;
  float toFloat(float def=0) const
    { bool ok; return toFloat(ok, def); }

  bool toBool(bool& ok, bool def=false) const;
  bool toBool(bool def=false) const
    { bool ok; return toBool(ok, def); }

  bool operator==(const KeyValue& other) const
    { return mKey == other.mKey && mValue == other.mValue && mHasValue == other.mHasValue; }

  std::vector<int> toInts() const;
  std::vector<float> toFloats() const;

private:
  std::string mKey, mValue;
  bool mHasValue, mKeptQuotes;
};

typedef std::vector<KeyValue> KeyValue_v;

KeyValue kv(const std::string& key, bool value);
KeyValue kv(const std::string& key, int value);
KeyValue kv(const std::string& key, float value);
KeyValue kv(const std::string& key, const std::string& value);
KeyValue kv(const std::string& key, const char* value);
KeyValue kv(const std::string& key, const std::vector<int>& values);
KeyValue kv(const std::string& key, const std::vector<float>& values);
KeyValue kv(const std::string& key, const std::vector<std::string>& values);


inline KeyValue_v& operator<<(KeyValue_v& ostr, const KeyValue& kv)
{ ostr.push_back(kv); return ostr; }

inline KeyValue_v operator<<(KeyValue_v&& ostr, const KeyValue& kv)
{
  ostr.push_back(kv);
  return ostr;
}

inline KeyValue_v& operator<<(KeyValue_v& ostr, const KeyValue_v& kvs)
{ ostr.insert(ostr.end(), kvs.begin(), kvs.end()); return ostr; }

inline void add(KeyValue_v& ostr, const std::string& key, bool value)
{ ostr.push_back(kv(key, value)); }

inline void add(KeyValue_v& ostr, const std::string& key, int value)
{ ostr.push_back(kv(key, value)); }

inline void add(KeyValue_v& ostr, const std::string& key, float value)
{ ostr.push_back(kv(key, value)); }

inline void add(KeyValue_v& ostr, const std::string& key, const std::string& value)
{ ostr.push_back(kv(key, value)); }

inline void add(KeyValue_v& ostr, const std::string& key, const char* value)
{ ostr.push_back(kv(key, value)); }

inline void add(KeyValue_v& ostr, const std::string& key, const std::vector<float>& values)
{ ostr.push_back(kv(key, values)); }

inline void add(KeyValue_v& ostr, const std::string& key, const std::vector<std::string>& values)
{ ostr.push_back(kv(key, values)); }


size_t find(const KeyValue_v& kvs, const std::string& key, size_t start=0);
size_t rfind(const KeyValue_v& kvs, const std::string& key);
size_t rfind(const KeyValue_v& kvs, const std::string& key, size_t start);

std::string mergeKeyValue(const miutil::KeyValue_v& kvs);
KeyValue_v splitKeyValue(const std::string& kvtext, bool keepQuotes=false);

std::ostream& operator<<(std::ostream& out, const miutil::KeyValue& kv);
std::ostream& operator<<(std::ostream& out, const miutil::KeyValue_v& kvs);

} // namespace miutil

#endif // DIKEYVALUE_H
