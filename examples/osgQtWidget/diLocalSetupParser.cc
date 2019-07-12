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

#include "diLocalSetupParser.h"

#include "miSetupParser.h"

#include <puTools/miStringFunctions.h>
#include <puCtools/mkdir.h>
#include <puCtools/stat.h>

#include <fstream>
#include <list>

#include <sys/types.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#define MILOGGER_CATEGORY "osgqt.LocalSetupParser"
#include <miLogger/miLogging.h>

using namespace std;

/// PVERSION is defined due to debian packing using the metno-debuild tool
#ifndef PVERSION
#define PVERSION
#endif
#ifndef DATAROOTDIR
#define DATAROOTDIR
#endif

const std::string SectBasics=      "BASIC";

// static members
std::string LocalSetupParser::setupFilename;
map<std::string,std::string>     LocalSetupParser::basic_values;
vector<std::string>           LocalSetupParser::langPaths;


bool LocalSetupParser::makeDirectory(const std::string& filename, std::string & error)
{
  pu_struct_stat buff;
  if (pu_stat(filename.c_str(), &buff) != -1){
    if ( S_ISDIR(buff.st_mode) ){
      return true;
    }
  }

  if (pu_mkdir(filename.c_str(), 0755) != 0) {
    error = strerror(errno);
    return false;
  }
  return true;
}

bool LocalSetupParser::parse(std::string& mainfilename)
{
  METLIBS_LOG_INFO("LocalSetupParser::parse:" << mainfilename);

  //find $HOME, and make homedir
  std::string homedir=miutil::from_c_str(getenv("HOME"));
  homedir += "/.osgqt";
  std::string error;
  if (makeDirectory(homedir,error)) {
    std::string workdir = homedir + "/work";
    if (makeDirectory(workdir,error))
      basic_values["workdir"]    = workdir;

    std::string cachedir = homedir + "/cache";
    if (makeDirectory(cachedir,error))
      basic_values["cachedir"] = cachedir;
  } else {
    homedir=".";
  }
  basic_values["homedir"]    = homedir;

  //if no setupfile specified, use previus setupfile
  if (not mainfilename.empty())
    setupFilename = mainfilename;

  //if no setupfile, use default
  if (setupFilename.empty()) {
    setupFilename = "diana.setup-SMHI-3D";
    std::string filename_str = setupFilename;
    METLIBS_LOG_INFO("filename:" << setupFilename);
    ifstream file(setupFilename.c_str());
    if (!file) {
      setupFilename = homedir + "/diana.setup-SMHI-3D";
      filename_str += " or ";
      filename_str += setupFilename;
      METLIBS_LOG_INFO("filename:" << setupFilename);
      ifstream file2(setupFilename.c_str());
      if (!file2) {
        setupFilename = "/etc/diana/diana.setup-SMHI-3D";
        filename_str += " or ";
        filename_str += setupFilename;
        METLIBS_LOG_INFO("filename:" << setupFilename);
        ifstream file3(setupFilename.c_str());
        if (!file3) {
          METLIBS_LOG_ERROR("LocalSetupParser::readSetup. cannot open default setupfile "
          << filename_str);
          METLIBS_LOG_ERROR("Try osgQtWidget -s setupfile");
          return false;
        }
      }
    }
  }

  if (! miutil::SetupParser::parse( setupFilename ) )
    return false;

  if (!parseBasics(SectBasics)) return false;

  // return the setupFilename
  mainfilename=setupFilename;

  return true;
}



// parse basics-section
bool LocalSetupParser::parseBasics(const std::string& sectname){

  const std::string key_fontpath= "fontpath";
  const std::string key_docpath=  "docpath";
  const std::string key_obspath=  "obsplotfilepath";
  const std::string key_qserver=  "qserver";
  const std::string key_imagepath="imagepath";
  const std::string key_langpaths="languagepaths";
  const std::string key_language= "language";
  const std::string key_setenv= "setenv";

  // default values
  const std::string SHAREDIR = DATAROOTDIR "/diana/" PVERSION;
  std::string langpaths = SHAREDIR + "/lang:${QTDIR}/translations";
  std::string language = "en";
  basic_values[key_fontpath] = SHAREDIR + "/fonts";
  basic_values[key_docpath] = "share/doc/diana-" PVERSION;
  basic_values[key_obspath] = SHAREDIR;
  basic_values[key_qserver] = "/usr/bin/coserver";
  basic_values[key_imagepath] = SHAREDIR + "/images";
  basic_values[key_language] = language;

  vector<std::string> list,tokens;
  std::string key,value;
  int i,n;

  if (!miutil::SetupParser::getSection(sectname,list))
    return true;

  n= list.size();
  for (i=0; i<n; i++){
    miutil::SetupParser::splitKeyValue(list[i],key,value);

    // everything into basic_values map
    basic_values[key] = value;

    if (key==key_langpaths){
      langpaths= value;
    } else if (key==key_setenv){
      vector<std::string> part = miutil::split(value, ",");
      if(part.size()==3){
#ifdef __WIN32__
        //TODO: This is broken, disregards third argument (replace option)
	std::string envst = part[0] + "=" + part[1];
	putenv(envst.c_str());
#else
        setenv(part[0].c_str(), part[1].c_str(), miutil::to_int(part[2]));
#endif
      }
    }
  }

  // fix language paths
  //   checkEnvironment(langpaths);
  langPaths = miutil::split(langpaths, ":");

  return true;
}


