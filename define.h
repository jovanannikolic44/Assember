// address modes
const int immediate = 0;
const int regdir = 1;
const int regdirpom = 5;
const int regind = 2;
const int regindpom = 3;
const int memdir = 4;
const int pcrel = 6;

// opcodes
const string iretCode = "20";
const string haltCode = "00";
const string retCode = "40";
const string intCode = "10";
const string callCode = "30";
const string jmpCode = "50";
const string jneCode = "52";
const string jeqCode = "51";
const string jgtCode = "53";
const string addCode = "70";
const string subCode = "71";
const string mulCode = "72";
const string divCode = "73";
const string cmpCode = "74";
const string notCode = "80";
const string andCode = "81";
const string orCode = "82";
const string xorCode = "83";
const string testCode = "84";
const string shlCode = "90";
const string shrCode = "91";
const string xchgCode = "60";
const string ldrCode = "A0";
const string strCode = "B0";
