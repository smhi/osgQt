/*
 Diana - A Free Meteorological Visualisation Tool

 Copyright (C) 2013-2018 met.no

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

//#define DEBUGPRINT
//#define DEBUGFDIFF

#include "osgModelManager.h"

#include "miSetupParser.h"

#include "diUtilities.h"
#include "util/misc_util.h"

#include <puTools/miStringFunctions.h>

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/split.hpp>

#include <cmath>
#include <iomanip>
#include <iterator>
#include <set>
#include <sstream>
#include <strstream>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>

#define MILOGGER_CATEGORY "osgqt.ModelManager"
#include "miLogger/miLogging.h"
/*
using namespace std;
using namespace miutil;
using namespace miutil::constants;
*/
namespace {

const std::string MODEL_FILES = "MODEL_FILES";

} // namespace

ModelManager::ModelManager()
{
  METLIBS_LOG_SCOPE();
}

ModelManager::~ModelManager()
{
  METLIBS_LOG_SCOPE();
}

std::vector<std::string> ModelManager::subsections()
{
  std::vector<std::string> subs;
  subs.push_back(MODEL_FILES);
  return subs;
}

bool ModelManager::parseSetup()
{
  // Parse field sections
  std::vector<std::string> errors;
  std::vector<std::string> lines;
  for (const std::string& suse : subsections()) {
    miutil::SetupParser::getSection(suse, lines);
    parseSetup(lines, suse, errors);
  }
  // Write error messages
  int nerror = errors.size();
  for (int i = 0; i < nerror; i++) {
    std::vector<std::string> token = miutil::split(errors[i], "|");
    miutil::SetupParser::errorMsg(token[0], atoi(token[1].c_str()), token[2]);
  }

  return true; // FIXME this ignores errors
}

bool ModelManager::parseSetup(const std::vector<std::string>& lines,
    const std::string& token, std::vector<std::string>& errors)
{
  if (lines.empty())
    return true;

  METLIBS_LOG_SCOPE(LOGVAL(token));

  if (token == MODEL_FILES)
    return updateFileSetup(lines, errors, true);

  return true;
}

bool ModelManager::updateFileSetup(const std::vector<std::string>& lines,
    std::vector<std::string>& errors, bool clearSources, bool top)
{
  METLIBS_LOG_SCOPE();

  if (clearSources) {
    fieldModelGroups.clear();
  }

  std::string groupName = "First Group"; // default
  std::string groupType = "fileGroup";

  const int nlines = lines.size();
  for (int l = 0; l < nlines; l++) {
    const std::vector<std::string> tokens = miutil::split_protected(lines[l], '"', '"');

    std::string modelName;
    std::string fieldFileType;
    std::vector<std::string> fileNames;
    std::vector<std::string> options;
    std::vector<std::string> format;
    std::vector<std::string> config;
    std::string guiOptions;
    std::string gridioType = "fimex";
    bool validTimeFromFilename = false;
    bool clearFileGroup = false;
    bool clearFiles = false;

    const int m = tokens.size();
    for (int j = 0; j < m; j++) {
      //delete "Model group" and all models (FieldSource-objects) in the group
      const std::string tu = miutil::to_upper(tokens[j]);
      if (tu == "CLEAR_FILEGROUP") {
        clearFiles = true;
        clearFileGroup = true;
        break;
      }
      //delete all models (FieldSource-objects) in the model group
      if (tu == "CLEAR_FILES") {
        clearFiles = true;
        break;
      }

      if (tu == "OK") { // OK/DELETE: used to skip some models
        continue;
      }

      std::vector<std::string> stokens = miutil::split_protected(tokens[j], '"', '"', "=", true);
      if (stokens.size() < 2) {
        std::string error = MODEL_FILES + "|" + miutil::from_number(l) + "|Missing argument to keyword: " + tokens[j];
        errors.push_back(error);
        continue;
      }
      std::string key = miutil::to_lower(stokens[0]);
      miutil::remove(key, '"');
      miutil::remove(stokens[1], '"');
      if (key == "default_file") {
        const std::vector<std::string> sstoken = miutil::split(stokens[1], ":");
        if (sstoken.size() == 2) {
          defaultFile[sstoken[0]] = sstoken[1];
        }
      } else if (key == "default_config") {
        const std::vector<std::string> sstoken = miutil::split(stokens[1], ":");
        if (sstoken.size() == 2)
          defaultConfig[sstoken[0]] = sstoken[1];
      } else if (key == "filegroup" || key == "archivefilegroup"
          || key == "profetfilegroup")
      {
        // group name (only used in dialog)
        groupName = stokens[1];
        groupType = key;
      } else if (key == "m" && modelName.empty()) {
        modelName = stokens[1];
      } else if (key == "t") {
        if ( stokens[1] != "fimex")
          format.push_back(stokens[1]);
      } else if (key == "f") {
        fileNames.push_back(stokens[1]);
      } else if (key == "o") {
        guiOptions = stokens[1];
      } else if (key == "format") {
        format.push_back(stokens[1]);
      } else if (key == "config" || key == "c") {
        config.push_back(stokens[1]);
      } else if (key == "gridiotype") {
        gridioType = stokens[1];
      } else if (key == "time") {
        validTimeFromFilename = (miutil::to_lower(stokens[1]) == "validtime");
      } else {
        options.push_back(tokens[j]);
      }
    }

    //delete models from current model group
    if (clearFiles) {
      FieldModelGroupInfo_v::iterator p = fieldModelGroups.begin();
      while (p != fieldModelGroups.end() and p->groupName != groupName)
        p++;
      if (p != fieldModelGroups.end()) {
        if (clearFileGroup) {
          fieldModelGroups.erase(p); // remove group and models
        } else {
          p->models.clear(); // remove models
        }
      }
    }

    if (!modelName.empty() && (not fileNames.empty())) {

      std::vector<std::string> vModelNames;
      std::vector<std::vector<std::string> > vFileNames;

      if (miutil::contains(modelName, "*")) { // the * is replaced by the filename (without path)

        std::string mpart1, mpart2;
        unsigned int nstar = modelName.find_first_of('*');
        if (nstar > 0)
          mpart1 = modelName.substr(0, nstar);
        if (nstar < modelName.length() - 1)
          mpart2 = modelName.substr(nstar + 1);

        for (size_t j = 0; j < fileNames.size(); j++) {
          const diutil::string_v matches = diutil::glob(fileNames[j], 0);
          for (size_t k = 0; k < matches.size(); k++) {
            const std::string& fname = matches[k];
            size_t pb = fname.rfind('/');
            if (pb == std::string::npos)
              pb = 0;
            else
              pb++;
            modelName = mpart1 + fname.substr(pb) + mpart2;
            vModelNames.push_back(modelName);
            std::vector<std::string> vf(1, fname);
            vFileNames.push_back(vf);
          }
        }

      } else {

        vModelNames.push_back(modelName);
        vFileNames.push_back(fileNames);

      }

      unsigned int groupIndex = 0;
      while (groupIndex < fieldModelGroups.size() && fieldModelGroups[groupIndex].groupName != groupName)
        groupIndex++;
      if (groupIndex == fieldModelGroups.size()) {
        FieldModelGroupInfo fdi;
        fdi.groupName = groupName;
        if (groupType == "archivefilegroup")
          fdi.groupType = FieldModelGroupInfo::ARCHIVE_GROUP;
        else {
          if (groupType != "filegroup")
            METLIBS_LOG_WARN("group type '" << groupType << "' unknown, using standard group");
          fdi.groupType = FieldModelGroupInfo::STANDARD_GROUP;
        }
        if (top) {
          fieldModelGroups.insert(fieldModelGroups.begin(), fdi);
          groupIndex = 0;
        } else {
          fieldModelGroups.push_back(fdi);
        }
      }

      for (unsigned int n = 0; n < vModelNames.size(); n++) {
        const std::string& mn = vModelNames[n];
        if (!miutil::contains(miutil::to_lower(guiOptions), "notingui")) {
            fieldModelGroups[groupIndex].models.push_back(FieldModelInfo(mn, lines[l]));
        }
      }
    }
  }
  return true;
}

bool ModelManager::addModels(const std::vector<std::string>& configInfo)
{
  std::vector<std::string> lines;

  for (const std::string& ci : configInfo) {

    std::string datasource;
    std::string sourcetype;
    std::string dataset;
    std::string referencetime;
    std::string model;
    std::string guiOption;
    std::string config;
    std::string gridioType = "fimex";
    std::vector<std::string> options;

    std::string file;

    const std::vector<std::string> tokens = miutil::split_protected(ci, '"', '"');
    if (tokens.size() < 3) {
      lines.push_back(ci);
      continue;
    }

    for (const std::string& tok : tokens) {
      std::vector<std::string> stokens= miutil::split_protected(tok, '"', '"', "=", true);
      if (stokens.size()<2) {
        METLIBS_LOG_INFO("Missing argument to keyword: '" << tok << "', assuming it is an option");
        options.push_back(tok);
        continue;
      }
      std::string key = miutil::to_lower(stokens[0]);
      miutil::remove(key, '"');
      miutil::remove(stokens[1], '"');
      if (key == "datasource") {
        datasource = stokens[1];
        //        file = stokens[1];
      } else if (key == "sourcetype") {
        sourcetype = stokens[1];
      } else if (key == "dataset" ) {
        dataset = stokens[1];
      } else if (key == "referencetime" ) {
        referencetime = stokens[1];
        miutil::remove(referencetime, ':');
      } else if (key == "model" ) {
        model = stokens[1];
      } else if (key == "o" ) {
        guiOption = stokens[1];
      } else if (key == "gridioType" ) {
        gridioType = miutil::to_lower(stokens[1]);
      } else if (key == "config" || key == "c" ) {
        config = stokens[1];
      } else if (key == "file" ) {
        file = stokens[1];
      } else {
        options.push_back(tok);
      }

    }

    if (config.empty() && defaultConfig.count(sourcetype)>0 ) {
      config = defaultConfig[sourcetype];
    }

    if (file.empty() && defaultFile.count(sourcetype)>0 ) {
      file = defaultFile[sourcetype];
    }

    //make setup string
    if (sourcetype == "wdb" ) {
      std::ostringstream source;
      source <<"\"file="<<file<<";dataprovider="<<dataset<<";host="<<datasource<<";referencetime="<<referencetime<<"\"";
      file = source.str();
    }

    std::ostringstream ost;
    ost <<"m="<<model<<" t=" <<sourcetype<< " f="<<file;

    if (not config.empty()) {
      ost <<" config="<<config;
    }

    if (not guiOption.empty()) {
      ost <<" o="<<guiOption;
    }

    if (not options.empty()) {
      ost << ' ';
      std::copy(options.begin(), options.end(), std::ostream_iterator<std::string>(ost, " "));
    }

    lines.push_back(ost.str());
  }

  bool top = true;
  bool clearsources = false;
  std::vector<std::string> errors;
  return updateFileSetup(lines, errors, clearsources, top);
}

bool ModelManager::modelOK(const std::string& modelName)
{
  /*
  GridCollectionPtr pgc = getGridCollection(modelName, "");
  if (not pgc)
    return false;
  */
  return true;
}

void ModelManager::updateSources()
{
  /*
  for (GridSources_t::iterator it_gs = gridSources.begin();
      it_gs != gridSources.end(); ++it_gs)
    it_gs->second->updateSources();
    */
}

std::vector<std::string> ModelManager::getFileNames(const std::string& modelName)
{
  METLIBS_LOG_SCOPE();
  std::vector<std::string> filenames;
  /*
  GridCollectionPtr gridCollection = getGridCollection(modelName, "", true);
  if (gridCollection)
    filenames = gridCollection->getRawSources();
  */
  return filenames;
}
