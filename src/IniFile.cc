// *************************************************************************************************************
// *   IniFile
// *   Alexis Fagot
// *   29/01/2015
// *************************************************************************************************************

#include <iostream>
#include <cstdlib>
#include <fstream>
#include <sstream>

#include "../include/IniFile.h"
#include "../include/MsgSvc.h"

using namespace std;

// *************************************************************************************************************

IniFile::IniFile(){

}

// *************************************************************************************************************

IniFile::IniFile(string filename){
    SetFileName(filename);
}

// *************************************************************************************************************

IniFile::~IniFile(){

}

// *************************************************************************************************************

bool IniFile::CheckIfComment(string line){
    return ( line[0] == '#' );
}

// *************************************************************************************************************

bool IniFile::CheckIfGroup(string line,string& group){
    if( line[0] == '[' ){
        if( line[line.length()-1] == ']' ){ // The format is right
            group = line.substr(1,line.length()-2);
        } else {
            Error = INI_ERROR_WRONG_GROUP_FORMAT;
            MSG_ERROR("[Inifile]: Group error %i",Error);
        }
        return true;
    }
    return false;
}

// *************************************************************************************************************

bool IniFile::CheckIfToken(string line,string& key,string& value){
    size_t p0 = 0;

    size_t p1 = string::npos;
    p1 = line.find_first_of('=',p0);

    if(p1 != p0){
        key = line.substr(p0,(p1-p0));
        p0 = line.find_first_not_of('=',p1);
        if(p0 != string::npos){
            value = line.substr(p0,(line.size()-p0));
        } else {
            Error = INI_ERROR_MISSING_VALUE;
            MSG_ERROR("[Inifile]: Token error %i",Error);
        }
        return true;
    } else {
        Error = INI_ERROR_WRONG_FORMAT;
        MSG_ERROR("[Inifile]: Token error %i",Error);
        return false;
    }
}

// *************************************************************************************************************

void IniFile::SetFileName(const string filename){
    FileName = filename;
}

// *************************************************************************************************************

int IniFile::Read(){
    ifstream ini(FileName.c_str());
    stringstream parser;
    string token, value, line, group;

    Error = INI_OK;

    // Loading the file into the parser
    if(ini){
        parser << ini.rdbuf();
        ini.close();
    } else {
        Error = INI_ERROR_CANNOT_OPEN_READ_FILE;
        MSG_ERROR("[Inifile]: Read error %i",Error);
        return Error;
    }

    group = "";

    while(getline(parser,line) && (Error == INI_OK)){
        // Check if the line is comment
        if(!CheckIfComment(line) ){
            // Check for group
            if(!CheckIfGroup(line,group)){
                // Check for token
                if(CheckIfToken(line,token,value)){
                    // Make the key in format group.key if the group is not empty
                    if(group.size() > 1)
                        token = group + "." + token;
                    FileData[token] = value;
                } else {
                    Error = INI_ERROR_WRONG_FORMAT;
                    return Error;
                }
            }
        }
    }

//    for(IniFileDataIter Iter = FileData.begin(); Iter != FileData.end(); Iter++)
//        cout << "[Inifile]: " << Iter->first << " = " << Iter->second << endl;

    return Error;
}

// *************************************************************************************************************

float IniFile::GetValue(string groupname, const string keyname, const float defaultvalue ){
    string key;
    float floatValue = defaultvalue;

    IniFileDataIter Iter;

    if(groupname.size() > 0)
        key = groupname + "." + keyname;

    Iter = FileData.find(key);

    if(Iter != FileData.end())
        floatValue = strtof(Iter->second.c_str(),NULL);

    return floatValue;
}
