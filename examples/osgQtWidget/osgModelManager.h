/* -*- c++ -*-
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
#ifndef diModelManager_h
#define diModelManager_h

#include "diCommonFieldTypes.h"
#include "osgModelManager.h"

#include <puTools/miTime.h>

#include <boost/shared_array.hpp>

#include <vector>
#include <map>
#include <set>

/**
 \brief Managing fields and "models"

 Parse setup
 Prepare field data and information (Field) for FieldPlot
 Initiate field reading and computations.
 */
class ModelManager {

public:
  ModelManager();

  ~ModelManager();

  /// read Field sections in setup
  bool parseSetup();

  void setFieldNames(const std::vector<std::string>& vfieldname);

  /// parse <MODEL_FILES> from setup, or updates
  bool updateFileSetup(const std::vector<std::string>& lines,
      std::vector<std::string>& errors, bool clearSources = false, bool top = false);

  /// add new model to filesetup
  bool addModels(const std::vector<std::string>& configInfo);

  /// return info about model/file groups (to FieldDialog)
  const FieldModelGroupInfo_v& getFieldModelGroups() const { return fieldModelGroups; }

  /// check if model exists
  bool modelOK(const std::string& modelName);

  void updateSources();

  std::vector<std::string> getFileNames(const std::string& modelName);

private:
  bool parseSetup(const std::vector<std::string>& lines, const std::string& token, std::vector<std::string>& errors);
  std::vector<std::string> subsections();

private:
  std::map<std::string, std::string> defaultConfig;
  std::map<std::string, std::string> defaultFile;
  FieldModelGroupInfo_v fieldModelGroups;
};

#endif
