#ifndef __INIFILE_H_
#define __INIFILE_H_

//***************************************************************
// *    GIF OFFLINE TOOL v3
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
// *    Developped by : Alexis Fagot
// *    29/01/2015
//***************************************************************

#include <string>
#include <map>

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
