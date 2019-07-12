/*
  Diana - A Free Meteorological Visualisation Tool

  Copyright (C) 2006-2018 met.no

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
#ifndef diLocalSetupParser_h
#define diLocalSetupParser_h

#include <vector>
#include <map>

/**
   \brief the setup file parser

   Reads a Diana setup file
   - organized in sections
   - environment/shell variable expansions
   - local variable expansions
   - inclusion of other files
   - rejoin lines ending with /
*/

class LocalSetupParser {
private:
  static std::string setupFilename;
  static std::map<std::string, std::string> basic_values;
  static std::vector<std::string>        langPaths;

  // parse basic info
  static bool parseBasics(const std::string&);
  // check if fielname exists, if not make directory
  static bool makeDirectory(const std::string& filename, std::string & error);

public:
  LocalSetupParser(){}

  /// recursively parse setupfiles - mainfilename can be changed in the process
  static bool parse(std::string& mainfilename);
  /// paths to check for language files
  static const std::vector<std::string>& languagePaths() {return langPaths;}
  /// Basic types
  static const std::string& basicValue(const std::string& key) { return basic_values[key];}
  /// Setup filename
  static const std::string& getSetupFileName() { return setupFilename;}
  static void setSetupFileName(const std::string& sf) { setupFilename=sf;}
};

#endif
