//***************************************************************
// *    GIF OFFLINE TOOL v6
// *
// *    Program developped to extract from the raw data files
// *    the rates, currents and DIP parameters.
// *
// *    Inifile.cc
// *
// *    Macro to read Dimensions.ini files and extract
// *    information from each RPC as number of partitions,
// *    gaps, strips, and all ID names.
// *    This file was originally developped for the GIF_DAQ.
// *
// *    Developped by : Alexis Fagot & Salvador Carillo
// *    07/06/2017
//***************************************************************

#include <sstream>
#include <fstream>

#include "../include/IniFile.h"
#include "../include/MsgSvc.h"
#include "../include/utils.h"

using namespace std;

// ****************************************************************************************************
// *    IniFile()
//
//  Default constructor
// ****************************************************************************************************

IniFile::IniFile(){

}

// ****************************************************************************************************
// *    IniFile(string baseName)
//
//  Constructor. It needs the information stored in the Dimensions.ini file
// ****************************************************************************************************

IniFile::IniFile(string filename){
    SetFileName(filename);
}

// ****************************************************************************************************
// *    ~IniFile()
//
//  Destructor
// ****************************************************************************************************

IniFile::~IniFile(){

}

// ****************************************************************************************************
// *    bool CheckIfComment(string line)
//
//  Private method used to parse the ini file. It looks for '#' characters at the beginning of the
//  line.
// ****************************************************************************************************

bool IniFile::CheckIfComment(string line){
    return ( line[0] == '#' );
}

// ****************************************************************************************************
// *    bool CheckIfGroup(string line,string& group)
//
//  Private method used to parse the mapping file. It looks for '[' and ']' characters at the
//  beginning and end of group title lines
// ****************************************************************************************************

bool IniFile::CheckIfGroup(string line,string& group){
    if( line[0] == '[' ){
        if( line[line.length()-1] == ']' ){ // The format is right
            group = line.substr(1,line.length()-2);
        } else {
            Error = INI_ERROR_WRONG_GROUP_FORMAT;
            MSG_ERROR("[Inifile-ERROR] Group error " + to_string(Error));
        }
        return true;
    }
    return false;
}

// ****************************************************************************************************
// *    bool CheckIfToken(string line,string& key,string& value)
//
//  Private method used to parse the mapping file. It looks for '=' characters in the middle of 2
//  string fields: key = value
// ****************************************************************************************************

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
            MSG_ERROR("[Inifile-ERROR] Token error " + to_string(Error));
        }
        return true;
    } else {
        Error = INI_ERROR_WRONG_FORMAT;
        MSG_ERROR("[Inifile-ERROR] Token error " + to_string(Error));
        return false;
    }
}

// ****************************************************************************************************
// *    void SetFileName(const string filename)
//
//  Set the name of private membre FileName. This is the name of the ini file.
// ****************************************************************************************************

void IniFile::SetFileName(const string filename){
    FileName = filename;
}

// ****************************************************************************************************
// *    int Read()
//
//  Open, read and save into the inifile buffer (private members FileData) the content of the ini
//  file.
// ****************************************************************************************************

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
        MSG_ERROR("[Inifile-ERROR] Read error " + to_string(Error));
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

    return Error;
}

// ****************************************************************************************************
// *    long intType(string groupname, string keyname, long defaultvalue )
//
//  Cast and return the value of a group.key parameter into a long.
// ****************************************************************************************************


long IniFile::intType(string groupname, string keyname, long defaultvalue ){
    string key;
    long intValue = defaultvalue;
    string fileValue;

    IniFileDataIter Iter;

    if(groupname.size() > 0)
        key = groupname + "." + keyname;

    Iter = FileData.find(key);

    if(Iter != FileData.end()){
        int base = 0;
        fileValue = Iter->second;
        if(fileValue[1] == 'x')
            base = 16;
        else if(fileValue[1] == 'b'){
            base = 2;                   //In the case of binary values, the function doesn't
            fileValue.erase(0,2);       //understand 0b as a integer literals -> erase it
        }

        intValue = strtol(fileValue.c_str(),NULL,base);
    }
    else {
        string defVal = longTostring(defaultvalue);
        MSG_WARNING("[IniFile-WARING] "+key+" could not be found : default key used instead ("+defVal+")");
    }

    return intValue;
}

// ****************************************************************************************************
// *    string stringType(string groupname, string keyname, string defaultvalue )
//
//  Cast and return the value of a group.key parameter into a string.
// ****************************************************************************************************

string IniFile::stringType( string groupname, string keyname, string defaultvalue ){
    string key;
    string stringChain = defaultvalue;

    IniFileDataIter Iter;

    if(groupname.size() > 0)
        key = groupname + "." + keyname;

    Iter = FileData.find(key);

    if(Iter != FileData.end())
        stringChain = Iter->second;
    else
        MSG_WARNING("[IniFile-WARING] "+key+" could not be found : default key used instead ("+defaultvalue+")");

    return stringChain;
}

// ****************************************************************************************************
// *    float floatType(string groupname, string keyname, float defaultvalue )
//
//  Cast and return the value of a group.key parameter into a float.
// ****************************************************************************************************

float IniFile::floatType( string groupname, string keyname, float defaultvalue ){
    string key;
    float floatValue = defaultvalue;

    IniFileDataIter Iter;

    if(groupname.size() > 0)
        key = groupname + "." + keyname;

    Iter = FileData.find(key);

    if(Iter != FileData.end())
        floatValue = strtof(Iter->second.c_str(),NULL);
    else {
        string defVal = floatTostring(defaultvalue);
        MSG_WARNING("[IniFile-WARING] "+key+" could not be found : default key used instead ("+defVal+")");
    }

    return floatValue;
}
