// *************************************************************************************************************
// *   IniFile
// *   Alexis Fagot
// *   29/01/2015
// *************************************************************************************************************

#include <string>
#include <map>
#include "utils.h"

#ifndef __INIFILE_H_
#define __INIFILE_H_

using namespace std;

// *************************************************************************************************************

const int INI_OK                            = 0;

// File Errors
const int INI_ERROR_CANNOT_OPEN_READ_FILE   = 10;

// Format Errors
const int INI_ERROR_WRONG_FORMAT            = 20;
const int INI_ERROR_WRONG_GROUP_FORMAT      = 21;
const int INI_ERROR_MISSING_VALUE           = 22;

// *************************************************************************************************************

typedef  map< const string, string > IniFileData;
typedef  IniFileData::iterator IniFileDataIter;

// *************************************************************************************************************


class IniFile{
    private:
        bool            CheckIfComment(string line);
        bool            CheckIfGroup(string line,string& group);
        bool            CheckIfToken(string line,string& key,string& value);
        string          FileName;
        IniFileData 	FileData;
        int             Error;

    public:
        IniFile();
        IniFile(string filename);
        virtual         ~IniFile();

        // Basic file operations
        void            SetFileName(const string filename);
        int             Read();

        // Data readout methods
        long            intType     (string groupname, string keyname, long defaultvalue);
        string          stringType  (string groupname, string keyname, string defaultvalue );
        float           floatType   (string groupname, string keyname, float defaultvalue );
};

#endif
