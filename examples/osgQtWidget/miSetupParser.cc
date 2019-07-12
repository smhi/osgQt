/*
  Diana - A Free Meteorological Visualisation Tool

  Copyright (C) 2017 met.no

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

#include "miSetupParser.h"

#include "diUtilities.h"
#include "util/charsets.h"
#include "util/diLineMerger.h"
#include "util/string_util.h"

#include <puTools/miStringFunctions.h>

#include <cstdlib>
#include <fstream>
#include <list>
#include <sstream>

#define MILOGGER_CATEGORY "diana.SetupParser"
#include <miLogger/miLogging.h>

namespace miutil {

void SetupSection::clear()
{
  strlist.clear();
  linenum.clear();
  filenum.clear();
}

// ========================================================================

SetupParser* SetupParser::self = 0;

// static function
SetupParser* SetupParser::instance()
{
  if (!self)
    self = new SetupParser();
  return self;
}

// static function
void SetupParser::destroy()
{
  delete self;
  self = 0;
}

SetupParser::SetupParser()
{
}

// static function
void SetupParser::setUserVariables(const string_string_m& user_var)
{
  instance()->user_variables = user_var;
}

// static function
const std::map<std::string, std::string>& SetupParser::getUserVariables()
{
  return instance()->substitutions;
}

// static function
void SetupParser::replaceUserVariables(const std::string& key, const std::string& value)
{
  instance()->user_variables[key] = value;
}

bool SetupParser::checkSubstitutions(std::string& t) const
{
  return substitute(t, false);
}

// static function
bool SetupParser::checkEnvironment(std::string& t)
{
  return instance()->substitute(t, true);
}

bool SetupParser::substitute(std::string& t, bool environment) const
{
  std::string::size_type start = 0, stop = 0;

  const char* const s_begin = (environment) ? "${" : "$(";
  const char* const s_end   = (environment) ? "}"  : ")";

  while ((start = t.find(s_begin, stop)) != std::string::npos) {
    if ((stop = t.find(s_end, start)) == std::string::npos)
      // unterminated
      return false;

    std::string n;
    const std::string s = miutil::to_upper(t.substr(start + 2, stop - start - 2));
    string_string_m::const_iterator it = substitutions.find(s);
    if (it != substitutions.end())
      n = it->second;
    else if (environment)
      n = miutil::from_c_str(getenv(s.c_str()));

    t.replace(start, stop+1 - start, n);
    stop = start + n.size(); // this does not allow recursive replacement
  }
  return true;
}

// static function
void SetupParser::cleanstr(std::string& s)
{
  instance()->substituteAll(s);
}

void SetupParser::substituteAll(std::string& s) const
{
  // substitute environment/shell variables
  checkEnvironment(s);

  // substitute local/setupfile variables
  checkSubstitutions(s);

  // prepare strings for easy split on '=' and ' '
  // : remove leading and trailing " " for each '='
  if (miutil::contains(s, "=") && miutil::contains(s, " ")) {
    std::string::size_type p = 0;
    while ((p = s.find_first_of("=", p)) != std::string::npos) {
      // check for "" - do not clean out blanks inside these
      std::vector<int> sf1, sf2;
      std::string::size_type f1 = 0, f2;
      while ((f1 = s.find_first_of("\"", f1)) != std::string::npos && (f2
          = s.find_first_of("\"", f1 + 1)) != std::string::npos) {
        sf1.push_back(f1);
        sf2.push_back(f2);
        f1 = f2 + 1;
      }
      bool dropit = false;
      for (size_t i = 0; i < sf1.size(); i++) {
        f1 = sf1[i];
        f2 = sf2[i];
        if (f1 > p) {
          // no chance of "" pairs around this..
          break;
        }
        if (f1 < p && f2 > p) {
          p = f2 + 1;
          // '=' is inside a "" pair, drop cleaning
          dropit = true;
          break;
        }
      }
      if (dropit)
        continue;

      while (p > 0 && s[p - 1] == ' ') {
        s.erase(p - 1, 1);
        p--;
      }
      while (p < s.length() - 1 && s[p + 1] == ' ') {
        s.erase(p + 1, 1);
      }
      p++;
    }
  }
}

// static function
KeyValue SetupParser::splitKeyValue(const std::string& s, bool keepCase)
{
  std::string k, v;
  const ssize_t eq = s.find("=");
  if (eq < 0) {
    k = s;
  } else {
    k = s.substr(0, eq);
    v = s.substr(eq+1);
    miutil::trim(v);

    // handle quotes and ||, this is a mess
    const int pos_or = v.find("||");
    const int vsize = v.size();
    bool quote_start = (not v.empty() and v[0] == '"');
    bool quote_end   = (not v.empty() and v[vsize-1] == '"');
    if (pos_or >= 0) {
      const int last_before_or = (pos_or > 0) ? v.find_last_not_of(" ", pos_or-1) : -1;
      const bool quoted_before_or = (quote_start and last_before_or > 0 and v[last_before_or]=='"');
      const int first_after_or = v.find_first_not_of(" ", pos_or+2);
      const bool quoted_after_or = (quote_end and first_after_or>=0 and first_after_or < vsize-1 and v[first_after_or]=='"');
      if (not ((quote_start and not quoted_before_or) or (quote_end and not quoted_after_or))) {
        const std::string before = (quoted_before_or)
            ? v.substr(1, last_before_or-1)
            : v.substr(0, last_before_or+1);
        if (not before.empty()) {
          v = before;
        } else {
          v = (quoted_after_or)
              ? v.substr(first_after_or+1, vsize-first_after_or-2)
              : v.substr(first_after_or, vsize-first_after_or);
        }
      } else if (quote_start and quote_end) {
        v = v.substr(1, vsize-2);
      }
    } else if (quote_start and quote_end) {
      v = v.substr(1, vsize-2);
    }
  }
  miutil::trim(k);
  if (not keepCase)
    k = miutil::to_lower(k);
  return KeyValue(k, v);
}

// static function
void SetupParser::splitKeyValue(const std::string& s, std::string& key,
    std::string& value, bool keepCase)
{
  const KeyValue kv = splitKeyValue(s, keepCase);
  key   = kv.key();
  value = kv.value();
}

// static function
void SetupParser::splitKeyValue(const std::string& s, std::string& key, string_v& value)
{
  value.clear();
  string_v vs = miutil::split(s, 1, "=", true);
  if (vs.size() == 2) {
    key = miutil::to_lower(vs[0]); // always converting keyword to lowercase !
    string_v vv = miutil::split(vs[1], 0, ",", true);
    int n = vv.size();
    for (int i = 0; i < n; i++) {
      value.push_back(diutil::quote_removed(vv[i]));
    }
  } else {
    key = miutil::to_lower(s); // assuming pure keyword (without value)
  }
}

// static function
std::vector<KeyValue> SetupParser::splitManyKeyValue(const std::string& line, bool keepCase)
{
  std::vector<KeyValue> kvs;
  const string_v tokens = miutil::split(line);
  for (const std::string& t : tokens)
      kvs.push_back(splitKeyValue(t, keepCase));
  return kvs;
}

bool SetupParser::parseFile(const std::string& filename, const std::string& section, const int level)
{
  METLIBS_LOG_INFO(" " << std::string(level + 1, '.') << " reading \t[" << filename << "]");

  // list of filenames, index to them
  sfilename.push_back(filename);
  int activefile = sfilename.size() - 1;

  const std::string undefsect = "_UNDEF_";
  std::string origsect = (not section.empty() ? section : undefsect);
  std::string sectname = origsect;
  std::list<std::string> sectstack;

  // open filestream
  std::ifstream file(filename.c_str());
  if (!file) {
    METLIBS_LOG_ERROR("cannot open setupfile '" << filename << "'");
    return false;
  }

  /*
   - skip blank lines,
   - strip lines for comments and left/right whitespace
   - merge lines ending with \
    - accumulate strings for each section
   */
  diutil::LineMerger lm;
  diutil::GetLineConverter convertline("#");
  std::string line;
  while (convertline(file, line)) {
    diutil::remove_comment_and_trim(line);
    if (!lm.push(line))
      // FIXME this discards the last line if it ends with '\'
      continue;

    if (lm.mergedline().empty())
      continue;

    /*
     Remove preceding and trailing blanks.
     Remove comments
     Remove blanks around '='
     Do variable substitutions
     */
    std::string str = lm.mergedline();
    cleanstr(str);
    if (str.empty())
      continue;

    const size_t n = str.length();

    /*
     Check each line..
     */
    if (n > 1 && str[0] == '<' && str[n - 1] == '>') {
      // start or end of section
      if (str[1] == '/') { // end of current section
        if (sectstack.size() > 0) {
          // retreat to previous section
          sectname = sectstack.back();
          sectstack.pop_back();
        } else
          sectname = undefsect;

      } else { // start of new section
        // push current section onto stack
        sectstack.push_back(sectname);
        sectname = str.substr(1, n - 2);
      }

    } else if (diutil::startswith(str, "%include")) {
      //  include other setupfiles
      if (n < 10) {
        internalErrorMsg(filename, lm.lineno(), "Missing filename for include");
        return false;
      }
      const std::string nextfile = miutil::trimmed(str.substr(8));
#if 1
      if (!parseFile(nextfile, sectname, level + 1))
        return false;
#else
      // support glob pattern in %include
      std::vector<std::string> nextfiles = diutil::glob(nextfile);
      std::sort(nextfiles.begin(), nextfiles.end());
      for (const std::string& nf : nextfiles) {
        if (!parseFile(nf, sectname, level + 1))
          return false;
      }
#endif

    } else if (miutil::to_upper(str) == "CLEAR") {
      /*
       Clear all strings for this section
       Only valid inside a section
       */
      if (sectname == undefsect) {
        internalErrorMsg(filename, lm.lineno(), "CLEAR only valid within a section");
        continue;
      }
      std::map<std::string,SetupSection>::iterator its = sectionm.find(sectname);
      if (its != sectionm.end())
        its->second.clear();
    } else {
      /*
       Add string to section.
       If undefined section, check instead for variable declaration
       */
      if (sectname == undefsect) {
        std::string key, value;
        splitKeyValue(str, key, value);
        if (not value.empty()) {
          // Redefinitions are ignored
          const std::string key_up = miutil::to_upper(key);
          substitutions.insert(std::make_pair(key_up, value)); // map.insert does not overwrite!
        } else {
          METLIBS_LOG_WARN("setupfile line " << lm.lineno() << " in file " << filename << " is no variabledefinition, and is outside all sections:");
        }
        continue;
      }
      // add strings to appropriate section
      sectionm[sectname].strlist.push_back(str);
      sectionm[sectname].linenum.push_back(lm.lineno());
      sectionm[sectname].filenum.push_back(activefile);
    }
  }

  file.close();

  // File should start and end in same section
  if (sectname != origsect) {
    internalErrorMsg(filename, lm.lineno(), "File started in section " + origsect + " and ended in section " + sectname);
    return false;
  }
  return true;
}

// static function
void SetupParser::clearSect()
{
  instance()->sectionm.clear();
}

/*
 * Clears everything and parses a new setup file
 */
// static function
bool SetupParser::parse(const std::string& mainfilename)
{
  return instance()->parseFile(mainfilename);
}

bool SetupParser::parseFile(const std::string& mainfilename)
{
  sfilename.clear();
  sectionm.clear();
  substitutions.clear();

  // add user variables
  if (!user_variables.empty()) {
     METLIBS_LOG_INFO("adding user variables:");
     for (string_string_m::iterator itr = user_variables.begin(); itr != user_variables.end(); itr++) {
       const std::string key_up = miutil::to_upper(itr->first);
       METLIBS_LOG_INFO("'" << key_up << "' = '" << itr->second << "'");
       substitutions[key_up] = itr->second; // overrides user variable "Hei" with "heI"
     }
   }

   return parseFile(mainfilename, "", 0);
}

// report an error with filename and linenumber
// static function
void SetupParser::internalErrorMsg(const std::string& filename, int linenum,
    const std::string& error)
{
  METLIBS_LOG_ERROR("Error in setupfile '" << filename << "' line " << linenum << ": " << error);
}

// report an error with line# and sectionname
void SetupParser::errorMsg(const std::string& sectname, int linenum,
    const std::string& msg)
{
  instance()->writeMsg(sectname, linenum, msg, "Error");
}

// give a warning with line# and sectionname
void SetupParser::warningMsg(const std::string& sectname, const int linenum,
    const std::string& msg)
{
  instance()->writeMsg(sectname, linenum, msg, "Warning");
}

void SetupParser::writeMsg(const std::string& sectname, int linenum,
    const std::string& msg, const std::string& Error)
{
  const std::string& error = miutil::to_lower(Error);

  std::map<std::string, SetupSection>::iterator p;
  if ((p = sectionm.find(sectname)) != sectionm.end()) {
    int n = p->second.linenum.size();
    int lnum = (linenum >= 0 && linenum < n) ? p->second.linenum[linenum] : 9999;
    int m = p->second.filenum.size();
    int fnum = (linenum >= 0 && linenum < m) ? p->second.filenum[linenum] : 0;

    METLIBS_LOG_ERROR("Error in setupfile '" << sfilename[fnum] << "' section '" << sectname << "' line " << lnum
        << ": '" << p->second.strlist[linenum] << "', Message:" << msg);
  } else {
    METLIBS_LOG_ERROR("Internal SetupParser " << error << " in unknown section '"
        << sectname << "', Message:" << msg);
  }
}

bool SetupParser::getSection(const std::string& sectname, string_v& setuplines)
{
  std::map<std::string, SetupSection>::const_iterator p = instance()->sectionm.find(sectname);
  if (p != instance()->sectionm.end()) {
    setuplines = p->second.strlist;
    return true;
  }
#ifdef DEBUGPRINT1
  std::cerr << "Warning: ++SetupParser::getSection for unknown or missing (from setupfile) section: "
            << sectname << std::endl;
#endif
  setuplines.clear();
  return false;
}

} // namespace miutil
