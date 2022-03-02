#include <fstream>
#include <iostream>
#include <list>
#include <cstring>
#include <iomanip>
#include <map>
#include <cctype>
#include "tableType.h"
#include "define.h"
#include "FlexLexer.h"
#include "relocationType.h"
using namespace std;

list <tableType> symbol_table;
yyFlexLexer* flexLexer = nullptr;

list <RelocationTableType> relocation_table;
list <RelocationCodeType> code_table;

string get_symbol_name = "";
int address_mode = -1;
bool isGlobal = false;

int Location_Counter = 0;
int visiting_number = 0;
int token_saved = 0;
int UND_one_time = 0;
string section_name = "";
string new_val_name = "";
string op_name = "";
string reg_dst = "";
string reg_src = "";
string instrDescr = "";
string addr_m = "";
string globalExtern_s = "";


// functions

void insert_back_in_table(string insert_symbol_name, string section, int val, char scope, bool isSection){
    tableType *insert_symbol_row;
    insert_symbol_row = new tableType(-1, insert_symbol_name, section, val, scope, isSection);
    symbol_table.push_back(*insert_symbol_row);
    delete insert_symbol_row;   
}

void insert_front_in_table(string insert_symbol_name, string section, int val, char scope, bool isSection){
    tableType *insert_symbol_row;
    insert_symbol_row = new tableType(-1, insert_symbol_name, section, val, scope, isSection);
    symbol_table.push_front(*insert_symbol_row);
    delete insert_symbol_row;   
}

int error_function(string message) {
    cout<<message<<endl;
    return -1;
}

int check_if_exist(string sym_name) {
    list<tableType>::iterator i = symbol_table.begin();
    int it;
    for(it = 0; it < symbol_table.size(); it++) {
        advance(i, it); 
        if(i->name == sym_name) {
            return 1;
        }
        i = symbol_table.begin();
    }
    return 0;
}

int firstPass_oneByte(string instructionName, int token) {
    if(token == 0 || token == 4) {
        Location_Counter += 1;
    }
    else {
        return error_function("Error in the instruction " + instructionName + ".");
    }
    return 0;
}

void assign_id_to_all() {
    int counter = 0;
    list<tableType>::iterator list_iterator = symbol_table.begin();
    int iterator = 0;
    for(; iterator < symbol_table.size(); iterator++) {
        advance(list_iterator, iterator);
        if(list_iterator->isSection == true) {
            counter++;
        }
        list_iterator = symbol_table.begin();
    }
    int id = 0;
    int old_counter = counter;
    list_iterator = symbol_table.begin();
    iterator = 0;
    for(; iterator < symbol_table.size(); iterator++) {
        advance(list_iterator, iterator);
        if(list_iterator->isSection == true) {
            counter--;
            list_iterator->id = counter;
            if(counter == 0) {
                id += old_counter;
            }
        }
        else {
            list_iterator->id = id;
            id++;
        }

        list_iterator = symbol_table.begin();
    }
    symbol_table.sort();
}

int hex_to_decimal(string hex) {
    int value = 0;
    int base = 1;
    int hex_length = hex.length();
    for(int i = hex_length--; i >= 0; i--) {
        if(hex[i] >= '0' && hex[i] <= '9') {
            value += (hex[i] - 48) * base;
            base *= 16;
        }
        else if(hex[i] >= 'A' && hex[i] <= 'F') {
            value += (hex[i] - 55) * base;
            base *= 16;
        }
        else if(hex[i] >= 'a' && hex[i] <= 'f') {
            value += (hex[i] - 87) * base;
            base *= 16;
        }
    }
    return value;
}

string dec_to_hex(int dec_num) {
    map<int, char> m;

    char number = '0';
    char character = 'a';
 
    for (int i = 0; i <= 15; i++) {
        if (i < 10) {
            m[i] = number++;
        }
        else {
            m[i] = character++;
        }
    }
    string res = "";

    if (!dec_num) {
        return "0";
    }
 
    if (dec_num > 0) {
        while (dec_num) {
            res = m[dec_num % 16] + res;
            dec_num /= 16;
        }
    }
    else {
        u_int n = dec_num;

        while (n) {
            res = m[n % 16] + res;
            n /= 16;
        }
    }

    // size of hexadecimal value is limited to 4B
    if(res.length() > 4) 
        res.erase(0, 4);
    
    // to uppercase
    string res_s;
    char res_ch;
    for(int i = 0; i < res.length(); i++) {
        res_ch = toupper(res[i]);
        res_s += res_ch;
    }
    return res_s;
}

void add_to_rel_code_exists(string item, string sect, int n) {
    list<RelocationCodeType>::iterator i = code_table.begin();
    int it;
    for(it = 0; it < symbol_table.size(); it++) {
        advance(i, it); 
        if(i->getSectionName() == section_name) {
            for(int k = 0; k < n; k++) {
                i->data.push_back(item);
            }
            break;
        }
        i = code_table.begin();
    }
}

void add_to_rel_code_n_exists(string item, string sect, int n) {
    RelocationCodeType *rct1 = new RelocationCodeType(section_name);
    rct1->setSectionName(section_name);
    for(int k = 0; k < n; k++) {
        rct1->data.push_back(item);
    }
    code_table.push_back(*rct1);
}

int check_if_section_exists(string nameS) {
    list<RelocationCodeType>::iterator i = code_table.begin();
    int it;
    for(it = 0; it < symbol_table.size(); it++) {
        advance(i, it); 
        if(i->getSectionName() == nameS) {
            return 1;
        }
        i = code_table.begin();
    }
    return 0;
}

void manage_input_hex_to_code(int chk, string ss_name) {
    string high;
    string low;
    if(ss_name.length() == 1 || ss_name.length() == 2) {
        if(ss_name.length() == 1) {
            ss_name = '0'+ss_name;
        }
        if(chk == 0) {
            add_to_rel_code_n_exists(ss_name, section_name, 1);
        }
        else {
            add_to_rel_code_exists(ss_name, section_name, 1);
        }
        add_to_rel_code_exists("00", section_name, 1);
    }
    else {
        if(ss_name.length() == 3) {
            ss_name = '0'+ss_name;
        }
        high = ss_name;
        low = ss_name;
        low.erase(0,2);
        high.erase(2,4);

        if(chk == 0) {
            add_to_rel_code_n_exists(low, section_name, 1);
        }
        else {
            add_to_rel_code_exists(low, section_name, 1);
        }
        add_to_rel_code_exists(high, section_name, 1);
    }
}

int global_local_id(string symname) {
    list<tableType>::iterator i = symbol_table.begin();
    int it;
    string secname;
    for(it = 0; it < symbol_table.size(); it++) {
        advance(i, it); 
        if(i->name == symname) {
            if(i->scope == 'g') {
                isGlobal = true;
                return i->id;
            }
            else if(i->scope == 'l' && i->section != "ABS") {
                secname = i->section;
                isGlobal = false;
                break;
            }
            else if(i->section == "ABS") {
                return -1;
            }
        }
        i = symbol_table.begin();
    }
    i = symbol_table.begin();
    
    for(it = 0; it < symbol_table.size(); it++) {
        advance(i, it); 
        if(secname == "-") return 2;
        if(i->name == "."+secname && i->isSection == true) {
            return i->id;
        }
        i = symbol_table.begin();
    }
    
    return -5;
}

void add_to_relocation_code(string toAdd, string secName, int iterNum) {
    int chk = 0;
    chk = check_if_section_exists(secName);
    if(chk == 1) {
        add_to_rel_code_exists(toAdd, secName, iterNum);
    }
    else {
        add_to_rel_code_n_exists(toAdd, secName, iterNum);
    }
}

string form_regs_descr(string reg_dst, string reg_src) {
    string code;
    if(reg_dst == "pc")         reg_dst = "r7";
    else if(reg_dst == "sp")    reg_dst = "r6";
    if(reg_src == "pc")         reg_src = "r7";
    else if(reg_src == "sp")    reg_src = "r6";
    if(reg_dst.length() > 1 )   reg_dst = reg_dst.erase(0,1);
    if(reg_src.length() > 1)    reg_src = reg_src.erase(0,1);
    code = reg_dst + reg_src;
    return code;
}

void registerAddressing_3B(string InstrDescr, string RegsDescr, string AddrMode, string sec_name) {
    add_to_relocation_code(InstrDescr, section_name, 1);
    add_to_rel_code_exists(RegsDescr, sec_name, 1);
    add_to_rel_code_exists(AddrMode, sec_name, 1);
}

void absoluteAddressing_5B(string InstrDescr, string RegsDescr, string AddrMode, string DataH, string DataL, string sec_name) {
    add_to_relocation_code(InstrDescr, section_name, 1);
    add_to_rel_code_exists(RegsDescr, sec_name, 1);
    add_to_rel_code_exists(AddrMode, sec_name, 1);
    add_to_rel_code_exists(DataL, sec_name, 1);
    add_to_rel_code_exists(DataH, sec_name, 1);
}

int getValFromSymTable(string sym_n) {
    list<tableType>::iterator i = symbol_table.begin();
    int it;
    for(it = 0; it < symbol_table.size(); it++) {
        advance(i, it); 
        if(i->name == sym_n) {
            return i->value;
        }
        i = symbol_table.begin();
    }
    return -1;
}

int checkIfGlobalSymbolIsDefined(string globSymName) {
    if(globSymName == "") return -1;

    list<tableType>::iterator i = symbol_table.begin();
    int it;

    for(it = 0; it < symbol_table.size(); it++) {
        advance(i, it); 
        if(i->name == globSymName) {
            if(i->isDefined == true){
                return 1;
            }
            else {
                return 0;
            }
        }
        i = symbol_table.begin();
    }
    return -1;
}

int instructions5B(int token_saved, string get_symbol_name, int address_mode, string reg_src, string reg_dst, string instrDescr) {
    if(address_mode == regindpom || address_mode == pcrel) {
        reg_src = reg_src.erase(0,1);
        if(address_mode == regindpom) addr_m = "03";        // U3
        else    addr_m = "F5";
    }
    else {
        if(address_mode == immediate) addr_m = "F0";
        else if (address_mode == memdir) addr_m = "F4";
    }
    if(token_saved == 7 || token_saved == 8 || token_saved == 55) {
        // literal
        string data_high, data_low;
        string dataValueHex;
        string dataValue_s = get_symbol_name;
        int dataValue;

        if(token_saved != 55) {
            dataValue = stoi(get_symbol_name, nullptr, 0);
            dataValueHex = dec_to_hex(dataValue);
        }
        else {
            dataValueHex = dataValue_s.erase(0,2);
            dataValue = hex_to_decimal(dataValueHex);
        }
        if(dataValue <= 255) {
            if(dataValueHex.length() == 1) {
                data_high = "00";
                data_low = "0" + dataValueHex;
            }
            else if(dataValueHex.length() == 2) {
                data_high = "00";
                data_low = dataValueHex;
            }
            else {
                return -1; 
            }
        }
        else {
            string temp_hex_value = dataValueHex;
            if(temp_hex_value.length() == 3) {
                data_low = dataValueHex.erase(0,1);
                data_high = temp_hex_value.erase(1,2);
                data_high = "0" + data_high;
            }
            else if(temp_hex_value.length() == 4) { 
                data_low = dataValueHex.erase(0,2);
                data_high = temp_hex_value.erase(2,3);
            }
            else {
                return -1;
            }
        }
        absoluteAddressing_5B(instrDescr, reg_dst + reg_src, addr_m, data_high, data_low, section_name); 
        Location_Counter += 5;
    }
    else {
        // symbol
        int r_id = global_local_id(get_symbol_name);
        if(r_id == -5) {
            return -2;
        }
        
        if(address_mode == pcrel) {
            
            RelocationTableType* rel_table_descr = new RelocationTableType(Location_Counter + 3, "R_386_16PC", r_id, section_name);
            relocation_table.push_back(*rel_table_descr);

            Location_Counter += 3;

            string dataH, dataL;
            if(isGlobal == true) {
                dataL = "FE";
                dataH = "FF";
            }   
            else {
                int dataValue = getValFromSymTable(get_symbol_name);
                if(dataValue == -1) {
                    return -2;
                }
                dataValue = dataValue - 2;
                string dataValueHex = dec_to_hex(dataValue);
                string temp_hex_value = dataValueHex;
                if(dataValueHex.length() == 1) {
                    dataH = "00";
                    dataL = "0" + dataValueHex;
                }
                else if(dataValueHex.length() == 2) {
                    dataH = "00";
                    dataL = dataValueHex;
                }
                else if(dataValueHex.length() == 3) {
                    dataL = dataValueHex.erase(0,1);
                    dataH = temp_hex_value.erase(1,2);
                    dataH = "0" + dataH;
                }
                else if(dataValueHex.length() == 4) {
                    dataL = dataValueHex.erase(0,2);
                    dataH = temp_hex_value.erase(2,3);
                }
                else {
                    return -1;
                }
            }  
            Location_Counter += 2; 
            absoluteAddressing_5B(instrDescr, reg_dst + reg_src, addr_m, dataH, dataL, section_name); 
        }
        else {
            if(r_id != -1) {         
                RelocationTableType* rel_table_descr = new RelocationTableType(Location_Counter + 3, "R_386_16", r_id, section_name);
                relocation_table.push_back(*rel_table_descr);
            }

            string dataH, dataL;
            if(isGlobal == true) {
                dataH = "00";
                dataL = "00";
            }
            else {
                int dataValue = getValFromSymTable(get_symbol_name);
                if(dataValue == -1) {
                    return -2;
                }
                
                string dataValueHex = dec_to_hex(dataValue);
                if(dataValue <= 255) {
                    if(dataValueHex.length() == 1) {
                        dataH = "00";
                        dataL = "0" + dataValueHex;
                    }
                    else if(dataValueHex.length() == 2) {
                        dataH = "00";
                        dataL = dataValueHex;
                    }
                    else {
                        return -1;
                    }
                }
                else {
                    string temp_hex_value = dataValueHex;
                    if(temp_hex_value.length() == 3) {
                        dataL = dataValueHex.erase(0,1);
                        dataH = temp_hex_value.erase(1,2);
                        dataH = "0" + dataH;
                    }
                    else if(temp_hex_value.length() == 4) {
                        dataL = dataValueHex.erase(0,2);
                        dataH = temp_hex_value.erase(2,3);
                    }
                    else {
                        return -1;
                    }
                }
            }
            absoluteAddressing_5B(instrDescr, reg_dst + reg_src, addr_m, dataH, dataL, section_name);
            Location_Counter += 5;
        }
    }

    return 0;
}

int main(int argc, char* argv[]) {
    char *outp = argv[2];
    char *inp = argv[3];
    char *inputFile = inp;
    char *outputFile = outp;
    filebuf fbIn;
    ofstream out;

    if (fbIn.open (inputFile,std::ios::in) == nullptr )
        return -1;

    istream is(&fbIn);

    flexLexer = new yyFlexLexer(is,cout);
    out.open(outputFile);	
    	
    int tokens = flexLexer->yylex();	
    int end_of_process = -1;

    while (tokens != 0){	
        if(tokens < 0) {
            return error_function("Error! Token has value lesser than zero.");
        }
        switch(tokens) {
            case 1: case 2: // .global a;   .extern a;    
                int globalExternToken;
                int check_existence;
                char* global_symbol_name;
                
                globalExternToken = tokens; 
                globalExtern_s = (string) flexLexer->YYText();   
                    
                tokens = flexLexer->yylex();

                if(tokens == 0 || tokens == 4) {         
                    return error_function("Error! The unexpected character in directive " + globalExtern_s + ".");
                }
                else if(tokens != 52 && tokens != 9 && tokens != 10 && tokens != 53 && tokens != 54) {
                    // argument is not symbol
                    return error_function("Error! Argument in the directive ."+globalExtern_s+" is not correct.");
                }

                if(visiting_number == 0) {
                    global_symbol_name = (char*) flexLexer->YYText();
                    check_existence = 0;
                    check_existence = check_if_exist(global_symbol_name);
                    
                    if(check_existence == 1) {
                        return error_function("Error! Symbol already exist in the symbol table.");
                    }

                    if(globalExternToken == 2) {
                        if(UND_one_time == 0) {
                            insert_front_in_table("UND", "UND", 0, 'l', true);
                            UND_one_time++;
                        }
                        insert_back_in_table(global_symbol_name, "UND", 0, 'g', false);
                    }
                    else {
                        insert_back_in_table(global_symbol_name, "-", 0, 'g', false);
                    }
                }
                else {
                    // in the second pass check if global symbol is defined 
                    if(globalExternToken == 1) {
                        int is_defined = 0;
                        string sym_name_s;

                        global_symbol_name = (char*) flexLexer->YYText();
                        is_defined = checkIfGlobalSymbolIsDefined(global_symbol_name);

                        sym_name_s = (string) global_symbol_name;

                        if(is_defined == 0) {
                            return error_function("Error! Global symbol " + sym_name_s + " must be defined.");
                        }
                        else if(is_defined == -1) {
                            return error_function("Error! Global symbol " + sym_name_s + " does not exist in the symbol table.");
                        }
                    }
                }

                tokens = flexLexer->yylex();

                // if there is more than one argument in the directive
                while(tokens == 5) {
                    tokens = flexLexer->yylex();
                    if(tokens == 0 || tokens == 4) { 
                        return error_function("Error! The unexpected character in directive " + globalExtern_s + ".");
                    }
                    else if(tokens != 52 && tokens != 9 && tokens != 10 && tokens != 53 && tokens != 54) {
                        return error_function("Error! Arguments in the directive ."+globalExtern_s+" are not correct.");
                    }

                    if(visiting_number == 0) {
                        global_symbol_name = (char*) flexLexer->YYText();
                        check_existence = check_if_exist(global_symbol_name);
                        if(check_existence == 1) {
                            return error_function("Error! Symbol already exist in the symbol table.");
                        }
                        
                        if(globalExternToken == 2) {
                            insert_back_in_table(global_symbol_name, "UND", 0, 'g', false);
                        }   
                        else {
                            insert_back_in_table(global_symbol_name, "-", 0, 'g', false);
                        }
                    }
                    else {
                    // in the second pass check if global symbol is defined 
                    if(globalExternToken == 1) {
                        int is_defined = 0;
                        string sym_name_s;

                        global_symbol_name = (char*) flexLexer->YYText();
                        is_defined = checkIfGlobalSymbolIsDefined(global_symbol_name);

                        sym_name_s = (string) global_symbol_name;

                        if(is_defined == 0) {
                            return error_function("Error! Global symbol must be defined.");
                        }
                        else if(is_defined == -1) {
                            return error_function("Error! Global symbol " + sym_name_s + " does not exist in the symbol table.");
                        }
                    }
                }
                    
                    tokens = flexLexer->yylex();    
                }
                
                if(tokens != 4 && tokens != 0) {
                    return error_function("Error! The unexpected character in directive .global.");
                }
            break;

            case 3: // .section name
                tokens = flexLexer->yylex();

                section_name = (string) flexLexer->YYText();

                if(tokens == 0 || tokens == 4) {
                    return error_function("Error! The unexpected character in directive .section.");
                } 
                else if(tokens != 52 && tokens != 9 && tokens != 10 && tokens != 53 && tokens != 54) {
                    // argument is not symbol
                    return error_function("Error! Argument in the directive .section is not correct.");
                }

                if(visiting_number == 0) {
                    // check if section with this name already exist in the symbol table
                    check_existence = 0;
                    check_existence = check_if_exist("." + section_name);
                
                    if(check_existence == 1) {
                        return error_function("Error! Section already exist in the symbol table.");
                    }
                }

                tokens = flexLexer->yylex();

                if(tokens == 0 || tokens == 4) {
                    if(visiting_number == 0) {
                        tableType *section_row = new tableType(-1, "."+section_name, section_name, 0, 'l', true);
                        list<tableType>::iterator it = symbol_table.begin();
                        symbol_table.insert(it, 1, *section_row);
                        delete section_row;
                    }
                    // reset Location Counter
                    Location_Counter = 0;
                }
                else {
                    return error_function("Error! The unexpected character in directive .section.");
                }  
            break;

            case 12: // skip <literal>
                int number;

                tokens = flexLexer->yylex();
                number = 0;

                if(section_name == "") {
                    return error_function("Error! Directive .skip has no section.");
                }

                if(tokens == 8) {
                    // only positive numbers are allowed
                    string num;
                    num = (string) flexLexer->YYText();
                    number = stoi(num, nullptr, 10);

                    tokens = flexLexer->yylex();

                    if(tokens == 0 || tokens == 4) {
                        Location_Counter += number;
                    }
                    else {
                        return error_function("Error! The unexpected literal in directive .skip.");
                    }
                }
                else if(tokens == 55) {
                    // hexadecimal number
                    string hex_value = (string) flexLexer->YYText();

                    if(hex_value.length() >= 2) {
                        // remove 0x from hex number
                        hex_value.erase(0, 2);
                    }

                    number = hex_to_decimal(hex_value);
                    Location_Counter += number;

                    tokens = flexLexer->yylex();

                    if(tokens != 0 && tokens != 4) {
                        return error_function("Error! Directive .skip is not correct.");
                    }
                }
                else {
                    return error_function("Error! The unexpected literal in directive .skip.");
                }
                if(visiting_number == 1) {
                    string skip_value = "00";
                    add_to_relocation_code(skip_value, section_name, number);
                }
            break;

            case 11:        // .word <lista_simbola ili literal>
                char* s_name;
                if(section_name == "") {
                    return error_function("Error! There is no section in the directive .word.");
                }

                tokens = flexLexer->yylex();

                if(tokens == 0 || tokens == 4) {
                    return error_function("Error! The unexpected character in directive .word.");
                }
                if(visiting_number == 0) {
                    Location_Counter += 2; 
                }
                else if(visiting_number == 1) {
                    string ss_name;
                    s_name = (char*) flexLexer->YYText();
                    
                    if(tokens == 55 || tokens == 7 || tokens == 8) {       
                        // .word allows negative numbers    
                        int chk;
                        chk = check_if_section_exists(section_name);
                        if(tokens != 55) {
                            // decimal literal
                            int dec_num;
                            dec_num = atoi(s_name);
                            ss_name = dec_to_hex(dec_num);
                        }
                        else {
                            // hexadecimal literal
                            ss_name = (string)s_name;
                            ss_name.erase(0, 2);
                        }
                        if(ss_name.length() > 4) {
                            return error_function("Error! Parametar of the directive .word is too large.");
                        }
                        manage_input_hex_to_code(chk, ss_name);
                    }
                    else {
                        // symbol
                        string ss_name;
                        ss_name = (string) s_name;

                        int chk_symbol = check_if_exist(ss_name);
                       
                        if(chk_symbol == 0) {
                            return error_function("Error! Symbol does not exists in the symbol table.");
                        }
                        
                        // find symbol in the symbol table and return id (symbol or section id)
                        int r_id = global_local_id(ss_name);
                        if(r_id == -5) {
                            return error_function("Error in the directive .word [II pass]");
                        }
                        
                        string word_value_first = "00";
                        string word_value_second = "00";

                        if(isGlobal == false) {
                            // local symbol
                            string hex_val;
                            string tmp;
                            int value = getValFromSymTable(ss_name);

                            hex_val = dec_to_hex(value);
                             
                            if(hex_val.length() == 1) {
                                word_value_first = "0" + hex_val;
                            }
                            else if(hex_val.length() == 2) {
                                word_value_first = hex_val;
                            }
                            else if(hex_val.length() == 3) {
                                tmp = hex_val;
                                word_value_first = hex_val.erase(0,1);
                                word_value_second = tmp.erase(1,2);
                                word_value_second = "0" + word_value_second;
                            }
                            else if(hex_val.length() == 4) { 
                                tmp = hex_val;
                                word_value_first = hex_val.erase(0,2);
                                word_value_second = tmp.erase(2,3);
           
                            }
                            else {
                                return error_function("Error! Number is larger than 2B in the directive .word.");
                            }
                        }
                        
                        int chk_sec = check_if_section_exists(section_name);
                        if(chk_sec == 1) {
                            add_to_rel_code_exists(word_value_first, section_name, 1);
                            add_to_rel_code_exists(word_value_second, section_name, 1);
                        }
                        else {
                            add_to_rel_code_n_exists(word_value_first, section_name, 1);
                            add_to_rel_code_exists(word_value_second, section_name, 1);
                        }
                      
                        RelocationTableType* rel_z1 = new RelocationTableType(Location_Counter, "R_386_16", r_id, section_name);
                        relocation_table.push_back(*rel_z1);
                    }

                    Location_Counter += 2;
                }

                tokens = flexLexer->yylex();

                while(tokens == 5) {
                    tokens = flexLexer->yylex();

                    if(tokens == 0 || tokens == 4) {
                        return error_function("Error! The unexpected character in directive .word.");
                    }
                    
                    if(visiting_number == 0) {
                        Location_Counter += 2;
                    }
                    else if(visiting_number == 1) {
                        string ss_name;
                        s_name = (char*) flexLexer->YYText();

                        if(tokens == 55 || tokens == 7 || tokens == 8) { 
                            // positive or negative literal           
                            int chk;
                            chk = check_if_section_exists(section_name);
                            if(tokens != 55) {
                                // decimal literal
                                int dec_num;
                                dec_num = atoi(s_name);
                                ss_name = dec_to_hex(dec_num);
                            }
                            else {
                                // hexadecimal literal
                                ss_name = (string)s_name;
                                ss_name.erase(0, 2);
                            }
                            if(ss_name.length() > 4) {
                                return error_function("Error! Parametar of the directive .word is too large.");
                            }
                            manage_input_hex_to_code(chk, ss_name);
                        }
                        else {
                            // symbol
                            string ss_name;
                            ss_name = (string) s_name;

                            int chk_symbol = check_if_exist(ss_name);

                            if(chk_symbol == 0) {
                                return error_function("Error! Symbol does not exists in the symbol table.");
                            }

                            int r_id = global_local_id(ss_name);
                            if(r_id == -5) {
                                return error_function("Error in the directive .word [II pass]");
                            }

                            string word_value_first = "00";
                            string word_value_second = "00";

                            if(isGlobal == false) {
                                // local symbol
                                string hex_val;
                                string tmp;
                                int value = getValFromSymTable(ss_name);
                                hex_val = dec_to_hex(value);

                                if(hex_val.length() == 1) {
                                word_value_first = "0" + hex_val;
                                }
                                else if(hex_val.length() == 2) {
                                    word_value_first = hex_val;
                                }
                                else if(hex_val.length() == 3) {
                                    tmp = hex_val;
                                    word_value_first = hex_val.erase(0,1);
                                    word_value_second = tmp.erase(1,2);
                                    word_value_second = "0" + word_value_second;
                                }
                                else if(hex_val.length() == 4) { 
                                    tmp = hex_val;
                                    word_value_first = hex_val.erase(0,2);
                                    word_value_second = tmp.erase(2,3);
            
                                }
                                else {
                                    return error_function("Error! Number is larger than 2B in the directive .word.");
                                }
                            }
                            // section already exist because this is not the first symbol
                            add_to_rel_code_exists(word_value_first, section_name, 1);
                            add_to_rel_code_exists(word_value_second, section_name, 1);

                            RelocationTableType* rel_z1 = new RelocationTableType(Location_Counter, "R_386_16", r_id, section_name);
                            relocation_table.push_back(*rel_z1);
                        }
                        Location_Counter += 2;
                    }
                    tokens = flexLexer->yylex();
                }
            break;

            case 9: case 10: case 52: case 53: case 54:       // a:
                get_symbol_name = (string) flexLexer->YYText();

                if(get_symbol_name == "end") {
                    end_of_process = 0;
                    break;
                }
                
                tokens = flexLexer->yylex();

                if(tokens != 13) {
                    return error_function("Error! The unexpected error in the definition of a label.");
                }
                if(visiting_number == 0) {      
                    // finds if symbol already exists in the symbol table   
                    bool symbol_exists = false;       
                    list<tableType>::iterator i = symbol_table.begin();
                    int it;
                    for(it = 0; it < symbol_table.size(); it++) {
                        advance(i, it);
                        // if symbol is local it will not be found in the symbol table, only global symbols
                        if(i->name == get_symbol_name) {
                            symbol_exists = true;
                            if(i->isDefined == true) {
                                return error_function("Error! Symbol " + i->name + " is already defined.");
                            }
                            if(i->scope == 'g') {
                                if(i->section == "UND") {
                                    return error_function("Error! Symbol " + i->name + " is extern symbol. Definition is not posssible.");
                                }
                                else {
                                    i->section = section_name;
                                    i->value = Location_Counter;
                                    i->isDefined = true;
                                }
                            }
                            else {
                                string message = "Error! The symbol " + i->name + " already exists.";
                                return error_function(message);
                            }
                            break; 
                        }
                        else {
                            symbol_exists = false;
                        }
                        i = symbol_table.begin();
                    }
                    if(symbol_exists == false) {
                        if(section_name == "") {
                            return error_function("Error! Local symbol " + get_symbol_name + " cannot be defined out of section.");
                        }
                        else {
                            insert_back_in_table(get_symbol_name, section_name, Location_Counter, 'l', false);
                        }
                    }
                }
            break;

            case 14:    // .equ<novi_simbol>, <literal>
                tokens = flexLexer->yylex();

                if(visiting_number == 0) {
                    if(tokens != 52 && tokens != 9 && tokens != 10 && tokens != 53 && tokens != 54) {
                        // argument is not symbol
                        return error_function("Error! Arguments in the directive .equ are not correct.");
                    }
                    new_val_name = (string) flexLexer->YYText();
                    check_existence = 0;
                    check_existence = check_if_exist(new_val_name);
                
                    if(check_existence == 1) {
                        return error_function("Error! Symbol " + new_val_name + " in the .equ directive already exists in the symbol table.");
                    }
                }

                tokens = flexLexer->yylex();

                if(tokens != 5) {
                    return error_function("Error! Unexpected character in the .equ directive.");
                }  

                tokens = flexLexer->yylex();
            
                if(tokens != 7 && tokens != 8 && tokens != 55) {
                    return error_function("Error! Unexpected character in the .equ directive.");
                }

                int symbol_value;

                if(visiting_number == 0) {
                    symbol_value = 0;
                    char* symbol_val;

                    symbol_val = (char*) flexLexer->YYText();
                    if(tokens == 55) {
                        // hexadecimal value
                        string hex_val = (string) symbol_val;

                        if(hex_val.length() >= 2) {
                            hex_val.erase(0, 2);
                        }

                        symbol_value = hex_to_decimal(hex_val);
                    }
                    else {     
                        symbol_value = atoi(symbol_val);
                    }
                }

                tokens = flexLexer->yylex();

                if(tokens != 0 && tokens != 4) {
                    return error_function("Error! Unexpected character in the .equ directive.");
                }
                if(visiting_number == 0) {
                    insert_back_in_table(new_val_name, "ABS", symbol_value, 'l', false);
                }
            break;

            // INSTRUCTION CASES
            case 15:    // instruction halt
                if(section_name == "")
                    return error_function("Error! Instruction is written out of section.");
                tokens = flexLexer->yylex();
                int rtv3;
                rtv3 = firstPass_oneByte("HALT", tokens);
                if(rtv3 == -1) return -1;
                if(visiting_number == 1) {
                    add_to_relocation_code(haltCode, section_name, 1);
                }
            break;

            case 16:    // instruction int regD
                if(section_name == "")
                    return error_function("Error! Instruction is written out of section.");
                tokens = flexLexer->yylex();
                reg_dst = (string) flexLexer->YYText();

                if(tokens != 17 && tokens != 44) {
                    return error_function("Error! Wrong register in the instruction INT.");
                }
                else {
                    tokens = flexLexer->yylex();

                    if(tokens == 0 || tokens == 4) {
                        Location_Counter += 2;

                        if(visiting_number == 1) {
                            reg_src = "F";
                            string regsDescr = form_regs_descr(reg_dst, reg_src);
                            add_to_relocation_code(intCode, section_name, 1);
                            add_to_rel_code_exists(regsDescr, section_name, 1);
                        }
                    }
                    else {
                        return error_function("Error in the instruction INT");
                    }
                }
            break;

            case 20:    // instruction iret
                if(section_name == "")
                    return error_function("Error! Instruction is written out of section.");
                tokens = flexLexer->yylex();
                int rtv1;
                rtv1 = firstPass_oneByte("IRET", tokens);
                if(rtv1 == -1) return -1;
                if(visiting_number == 1) {
                    add_to_relocation_code(iretCode, section_name, 1);
                }
            break;

            case 21: case 27: case 28: case 29: case 30:  
            // instructions: call operand; jmp operand; jeq operand; jne operand; jgt operand
                if(section_name == "")
                    return error_function("Error! Instruction is written out of section.");
                char* symbol_existence_name;
                op_name = (string) flexLexer->YYText();
                tokens = flexLexer->yylex();
                
                if(tokens != 23 && tokens != 6 && tokens != 7 && tokens != 8 && tokens != 9 && tokens != 10 && tokens != 52 && tokens != 55
                && tokens != 53 && tokens != 54) {
                    return error_function("Error in the jmp instructions.");
                }
                else if(tokens == 23) {
                    address_mode = pcrel;
                    if(visiting_number == 1) {
                        reg_dst = "F";
                        reg_src = "R7";
                    }
                }
                else if(tokens == 7 || tokens == 8 || tokens == 9 || tokens == 10 || tokens == 52 || tokens == 55 || tokens == 53 || tokens == 54) {
                    address_mode = immediate;
                    if(visiting_number == 1){
                        get_symbol_name = flexLexer->YYText();
                        token_saved = tokens;
                        reg_src = "F";
                        reg_dst = "F";
                    }
                }
                else if(tokens == 6) {
                    address_mode = memdir;
                }

                tokens = flexLexer->yylex();

                if(address_mode == pcrel && (tokens == 7 || tokens == 8 || tokens == 55)) {
                    return error_function("Error! PC rel addressing is not correct");
                }
                else if(address_mode == pcrel) {
                    if(visiting_number == 1)    {
                        token_saved = tokens;
                        get_symbol_name = flexLexer->YYText();
                    }
                    tokens = flexLexer->yylex();
                    if(tokens != 0 && tokens != 4) {
                        return error_function("Error! PC rel addressing is not correct.");
                    }
                }
                else if(address_mode == immediate) {
                    if(tokens != 0 && tokens !=4) {
                        return error_function("Error! Absolute addressing is not correct");
                    }
                }
                else {
                    if(tokens != 17 && tokens != 44 && tokens != 7 && tokens != 8 && 
                    tokens != 9 && tokens != 10 && tokens != 24 && tokens != 52 && tokens != 55
                    && tokens != 53 && tokens != 54) {
                        return error_function("Error in the jmp instructions.");
                    }

                    if(visiting_number == 1 && address_mode == memdir){    
                        get_symbol_name = flexLexer->YYText();
                        token_saved = tokens;
                        reg_src = "F";
                        reg_dst = "F";
                    }

                    if(tokens == 17 || tokens == 44) {
                        address_mode = regdir;
                        if(visiting_number == 1) {
                            string regInDir = "";
                            if(tokens == 44) {
                                get_symbol_name = flexLexer->YYText();
                                if(get_symbol_name == "pc")     regInDir = "r7";
                                else if(get_symbol_name == "sp")    regInDir = "r6";
                            }
                            else {
                                regInDir = (string) flexLexer->YYText();
                            }
                            reg_src = regInDir;
                            reg_dst = "F";
                        }
                    }
                    else if(tokens == 24) {
                        address_mode = regind;
                    }
                }

                if(address_mode != immediate && address_mode != pcrel) tokens = flexLexer->yylex();

                if(address_mode == regdir || address_mode == memdir) {
                    if(tokens != 0 && tokens != 4) {
                        return error_function("Error! Addressing is not correct");
                    }
                }
                else if(address_mode == regind) {
                    if(tokens == 17 || tokens == 44) {
                        if(visiting_number == 1) {
                            string regInDir = "";
                            if(tokens == 44) {
                                get_symbol_name = flexLexer->YYText();
                                if(get_symbol_name == "pc")     regInDir = "r7";
                                else if(get_symbol_name == "sp")    regInDir = "r6";
                            }
                            else {
                                regInDir = (string) flexLexer->YYText();
                            }
                            reg_src = regInDir;
                            reg_dst = "F";
                        }
                        tokens = flexLexer->yylex();
                        if(tokens != 25 && tokens != 26) {
                            return error_function("Error! Addressing is not correct");
                        } 
                        else if(tokens == 26) {
                            address_mode = regindpom;
                            tokens = flexLexer->yylex();
                            if(tokens != 7 && tokens != 8 && tokens != 9 && tokens != 10 && tokens != 52 && tokens != 55
                            && tokens != 53 && tokens != 54) {
                                return error_function("Error! Addressing is not correct");
                            } 
                            if(visiting_number == 1) {
                                get_symbol_name = flexLexer->YYText();
                                token_saved = tokens;
                            }
                            tokens = flexLexer->yylex();
                            if(tokens != 25) {
                                return error_function("Error! Addressing is not correct");
                            }
                            tokens = flexLexer->yylex();
                            if(tokens != 0 && tokens != 4) {
                                return error_function("Error! Addressing is not correct");
                            }
                        }
                        else if(tokens == 25) {
                            // regind addressing
                            tokens = flexLexer->yylex();
                            if(tokens != 0 && tokens != 4) {
                                return error_function("Error! Addressing is not correct.");
                            }
                        }
                    }
                    else {
                        return error_function("Error! Register should be the part of the instruction");
                    }
                }

                if(visiting_number == 1) {
                    if(op_name == "call")          instrDescr = callCode;
                    else if(op_name == "jmp")      instrDescr = jmpCode;
                    else if(op_name == "jeq")      instrDescr = jeqCode;
                    else if(op_name == "jne")      instrDescr = jneCode;
                    else if(op_name == "jgt")      instrDescr = jgtCode;
                    else return error_function("Unrecognized jump operation.");
                }

                if(address_mode == regdir || address_mode == regind) {
                    Location_Counter += 3;

                    if(visiting_number == 1) {
                        string reg_src_new;
                        string addr_m;
                        reg_src_new = reg_src.erase(0,1);
                        reg_src = reg_dst + reg_src_new;
                        
                        if(address_mode == regdir) addr_m = "F1";
                        else    addr_m = "02";      // U2

                        registerAddressing_3B(instrDescr, reg_dst+reg_src_new, addr_m, section_name);
                    }
                }
                else {
                    if(visiting_number == 0) Location_Counter += 5;
                    else {
                        int ret = instructions5B(token_saved, get_symbol_name, address_mode, reg_src, reg_dst, instrDescr);
                        
                        if(ret == -1) {
                            return error_function("Error! Arg of the instruction is too large."); 
                        }
                        else if(ret == -2) {
                            return error_function("Error! Symbol used in the instruction does not exist. [II pass]");
                        }
                        else if(ret != 0 && ret != -1 && ret != -2){
                            return error_function("Unknown error. [II pass]");
                        }
                    }
                }
            break;

            case 22:    // instruction ret
                if(section_name == "")
                    return error_function("Error! Instruction is written out of section.");
                tokens = flexLexer->yylex();
                int rtv2;
                rtv2 = firstPass_oneByte("RET", tokens);
                if(rtv2 == -1) return -1;
                 if(visiting_number == 1) {
                    add_to_relocation_code(retCode, section_name, 1);
                }
            break;

            case 31: case 32: case 33: case 34: case 35: case 37: case 38: case 39: case 40: case 41: case 42: case 43:
            // instructions: add regD, regS; sub regD, regS; mul regD, regS; div regD, regS; cmp regD, regS
            // and regD, regS; or regD, regS; xor regD, regS; test regD, regS
            // xchg regD, regS; shl regD, regS; shr regD, regS

                if(section_name == "")
                    return error_function("Error! Instruction is written out of section.");

                op_name = (string) flexLexer->YYText();
                tokens = flexLexer->yylex();

                // first register
                if(tokens == 17 || tokens == 44) {
                    string regInDir = "";
                    if(tokens == 44) {
                        get_symbol_name = flexLexer->YYText();
                        if(get_symbol_name == "pc")     regInDir = "r7";
                        else if(get_symbol_name == "sp")    regInDir = "r6";
                    }
                    else {
                        regInDir = (string) flexLexer->YYText();
                    }
                    reg_dst = regInDir;
                    
                    tokens = flexLexer->yylex();

                    if(tokens != 5) {
                        return error_function("Error! Unexpected character in the arithmetic operation");
                    }

                    tokens = flexLexer->yylex();
                   
                    // second register
                    if(tokens == 17 || tokens == 44) {
                        if(tokens == 44) {
                            get_symbol_name = flexLexer->YYText();
                            if(get_symbol_name == "pc")     regInDir = "r7";
                            else if(get_symbol_name == "sp")    regInDir = "r6";
                        }
                        else {
                            regInDir = (string) flexLexer->YYText();
                        }
                        reg_src = regInDir;

                        tokens = flexLexer->yylex();

                        if(tokens != 0 && tokens != 4) {
                            return error_function("Error! Unexpected character in the arithmetic operation");
                        }

                        Location_Counter += 2;

                        if(visiting_number == 1) {
                            string op_code;
                            if(op_name == "add")            op_code = addCode;
                            else if(op_name == "sub")       op_code = subCode;
                            else if(op_name == "mul")       op_code = mulCode;
                            else if(op_name == "div")       op_code = divCode;
                            else if(op_name == "cmp")       op_code = cmpCode;
                            else if(op_name == "and")       op_code = andCode;
                            else if(op_name == "or")        op_code = orCode;
                            else if(op_name == "xor")       op_code = xorCode;
                            else if(op_name == "test")      op_code = testCode;
                            else if(op_name == "shl")       op_code = shlCode;
                            else if(op_name == "shr")       op_code = shrCode;
                            else if(op_name == "xchg")      op_code = xchgCode;
                            else return error_function("Unrecognized arithmetic operation.");

                            string regsDescr = form_regs_descr(reg_dst, reg_src);
                            
                            add_to_relocation_code(op_code, section_name, 1);
                            add_to_rel_code_exists(regsDescr, section_name, 1);
                        }
                    }
                    else {
                        return error_function("Error! Unexpected name of the register.");
                    }
                }
                else {
                    return error_function("Error! Unexpected name of the register.");
                }
            break;

            case 36:
            // instructions: not regD
                if(section_name == "")
                    return error_function("Error! Instruction is written out of section.");
                tokens = flexLexer->yylex();

                if(tokens == 17 || tokens == 44) {
                    string regInDir = "";
                    if(tokens == 44) {
                        get_symbol_name = flexLexer->YYText();
                        if(get_symbol_name == "pc")     regInDir = "r7";
                        else if(get_symbol_name == "sp")    regInDir = "r6";
                    }
                    else {
                        regInDir = (string) flexLexer->YYText();
                    }
                    reg_dst = regInDir;

                    tokens = flexLexer->yylex();

                    if(tokens != 0 && tokens != 4) {
                        return error_function("Error! Unexpected character in the logic operation");
                    }

                    Location_Counter += 2; 

                    if(visiting_number == 1) {  
                        reg_src = "F"; 
                        string regsDescr = form_regs_descr(reg_dst, reg_src);
                        add_to_relocation_code(notCode, section_name, 1);
                        add_to_rel_code_exists(regsDescr, section_name, 1);
                    }

                }
                else {
                    return error_function("Error! Unexpected name of the register.");
                }
            break;

            case 45: case 46:
            // instructions: ldr regD, operand; str regD, operand
            // token_saved saves operand
            // token_instr saves instruction type - load or store
                if(section_name == "")
                    return error_function("Error! Instruction is written out of section.");
                int token_instr;
                char* symbol_check;

                token_instr = tokens;

                if(tokens == 45) instrDescr = ldrCode;
                else if(tokens == 46) instrDescr = strCode;

                tokens = flexLexer->yylex();
                
                // first operand is register
                if(tokens != 17 && tokens != 44) {
                    return error_function("Error! Unexpected character in the LDR or STR [1]");
                }

                // save destination register
                if(visiting_number == 1) {
                    string regInDir = "";
                    if(tokens == 44) {
                        get_symbol_name = flexLexer->YYText();
                        if(get_symbol_name == "pc")     regInDir = "r7";
                        else if(get_symbol_name == "sp")    regInDir = "r6";
                    }
                    else {
                        regInDir = (string) flexLexer->YYText();
                    }
                    reg_dst = regInDir;
                }

                tokens = flexLexer->yylex();

                if(tokens != 5) {
                    return error_function("Error! Unexpected character in the instruction LDR or STR [2]");
                }

                tokens = flexLexer->yylex();
                if(tokens != 23 && tokens != 7 && tokens != 8 && tokens != 9 && tokens != 10
                && tokens != 24 && tokens != 17 && tokens != 44 && tokens != 47
                && tokens != 52 && tokens != 55 && tokens != 54 && tokens != 53) {
                    return error_function("Error in the instruction LDR or STR [3]");
                }
                if(tokens == 47) {
                    address_mode = immediate;
                    tokens = flexLexer->yylex();
                    if(visiting_number == 1) {
                        token_saved = tokens;
                        get_symbol_name = flexLexer->YYText();
                        reg_src = "F";
                    }
                    if(tokens != 7 && tokens != 8 && tokens != 9 && tokens != 10 && tokens != 52 && tokens != 55) {
                        return error_function("Error! Unexpected character in the instruction LDR or STR [4]");
                    }
                }
                else if(tokens == 23) {
                    address_mode = pcrel;
                    tokens = flexLexer->yylex();

                    if(visiting_number == 1) {
                        token_saved = tokens;
                        get_symbol_name = flexLexer->YYText();
                        reg_src = "R7";
                    }
                    if(tokens != 9 && tokens != 10 && tokens != 52 && tokens != 53 && tokens != 54) {
                        return error_function("Error! Unexpected character in the instruction LDR or STR [5]");
                    }
                }
                else if(tokens == 17 || tokens == 44) {
                    address_mode = regdir;
                    if(visiting_number == 1) {
                        // save source register
                        string regInDir = "";
                        if(tokens == 44) {
                            get_symbol_name = flexLexer->YYText();
                            if(get_symbol_name == "pc")     regInDir = "r7";
                            else if(get_symbol_name == "sp")    regInDir = "r6";
                        }
                        else {
                            regInDir = (string) flexLexer->YYText();
                        }
                        reg_src = regInDir;
                    }
                }
                else if(tokens == 7 || tokens == 8 || tokens == 9 || tokens == 10 || tokens == 52 || tokens == 55
                        || tokens == 53 || tokens == 54) {
                    address_mode = memdir;
                    if(visiting_number == 1) {
                        token_saved = tokens;
                        get_symbol_name = flexLexer->YYText();
                        reg_src = "F";
                    }
                }
                else {
                    tokens = flexLexer->yylex();
                    
                    if(tokens != 17 && tokens != 44) {
                        return error_function("Error! Unexpected character in the instruction LDR or STR [6]");
                    }

                    // save source register
                    if(visiting_number == 1) {
                        string regInDir = "";
                        if(tokens == 44) {
                            get_symbol_name = flexLexer->YYText();
                            if(get_symbol_name == "pc")     regInDir = "r7";
                            else if(get_symbol_name == "sp")    regInDir = "r6";
                        }
                        else {
                            regInDir = (string) flexLexer->YYText();
                        }
                        reg_src = regInDir;

                    }

                    tokens = flexLexer->yylex();

                    if(tokens == 25) {
                        address_mode = regind;
                    }
                    else {
                        address_mode = regindpom;
                        tokens = flexLexer->yylex();
                        
                        if(visiting_number == 1) {
                            token_saved = tokens;
                            get_symbol_name = flexLexer->YYText();
                        }

                        if(tokens != 7 && tokens != 8 && tokens != 9 && tokens != 10 && tokens != 52 && tokens != 55 
                        && tokens != 53 && tokens != 54) {
                            return error_function("Error! Unexpected character in the instruction LDR or STR [7]");
                        }

                        tokens = flexLexer->yylex();
                        if(tokens != 25) {
                            return error_function("Error! Unexpected character in the instruction LDR or STR [8]");
                        }
                    }
                }
                tokens = flexLexer->yylex();

                if(tokens != 0 && tokens != 4) {
                    return error_function("Error! Unexpected character in the instruction LDR or STR [9]");
                }

                if(address_mode == immediate || address_mode == memdir || address_mode == pcrel || address_mode == regindpom) {
                    if(visiting_number == 0) Location_Counter += 5;
                    else {
                        reg_dst = reg_dst.erase(0,1);
                        int ret = instructions5B(token_saved, get_symbol_name, address_mode, reg_src, reg_dst, instrDescr);
                        if(ret == -1) {
                            return error_function("Error! Arg of the instruction is too large."); 
                        }
                        else if(ret == -2) {
                            return error_function("Error! Symbol used in the instruction does not exist. [II pass]");
                        }
                        else if(ret != 0 && ret != -1 && ret != -2){
                            return error_function("Unknown error. [II pass]");
                        }
                    }
                }
                else if(address_mode == regdir || address_mode == regind) {
                    Location_Counter += 3;

                    if(visiting_number == 1) {
                        string reg_src_new;
                        string reg_dst_new;
                        string addr_m;
                        reg_src_new = reg_src.erase(0,1);
                        reg_dst_new = reg_dst.erase(0,1);
                        reg_src = reg_dst + reg_src_new;
                        if(address_mode == regdir) addr_m = "F1";
                        else    addr_m = "02";

                        registerAddressing_3B(instrDescr, reg_dst+reg_src_new, addr_m, section_name);
                    }
                }
            break;

            case 48: case 49:
            // instructions: push regD; pop regD
                if(section_name == "")
                    return error_function("Error! Instruction is written out of section.");

                string regInDir = "";
                string addr_mode = "";

                op_name = (string) flexLexer->YYText();
                tokens = flexLexer->yylex();
               
                if(tokens == 44) {
                    get_symbol_name = flexLexer->YYText();
                    if(get_symbol_name == "pc")     regInDir = "r7";
                    else if(get_symbol_name == "sp")    regInDir = "r6";
                }
                else {
                    regInDir = (string) flexLexer->YYText();
                }

                reg_dst = regInDir;
                reg_dst = reg_dst.erase(0,1);

                if(tokens != 17 && tokens != 44) {
                    return error_function("Error in the instruction " + op_name + ".");
                }

                tokens = flexLexer->yylex();

                if(tokens == 0 || tokens == 4) {
                    Location_Counter += 3;

                    // no relocations because PUSH and POP use register addressing
                    if(visiting_number == 1) {
                        string op_code;
                        if(op_name == "push")           { op_code = strCode; addr_mode = "12"; }
                        else if(op_name == "pop")       { op_code = ldrCode; addr_mode = "42"; }
                        else return error_function("Unrecognized " + op_name + " instruction.");
                        // source register is always SP (stack pointer)
                        reg_src = "6";     
                        registerAddressing_3B(op_code, reg_dst+reg_src, addr_mode, section_name);
                    }
                }
                else {
                    return error_function("Error in the instruction PUSH or POP");
                }
            break;
        }   // end of switch
                
        tokens = flexLexer->yylex(); 

        if(end_of_process == 0) {
            visiting_number++;
            if(visiting_number == 1) {
                assign_id_to_all();
                is.clear();
                is.seekg(0, is.beg);
                tokens = flexLexer->yylex();
                end_of_process = -1;

                // reset global values
                section_name = "";
                Location_Counter = 0;
                get_symbol_name = "";
                address_mode = -1;
                isGlobal = false;
                token_saved = 0;
                UND_one_time = 0;
                new_val_name = "";
                op_name = "";
                reg_dst = "";
                reg_src = "";
                instrDescr = "";
                addr_m = "";
                globalExtern_s = "";
                
                // Symbol table
                out << setw(30) << "TABLE OF SYMBOLS" << endl;
                out << setw(15) << "ID" << setw(15) << "NAME" << setw(15) << "SECTION" << setw(15) << "VALUE" << setw(15) << "SCOPE" << endl;
                
                list<tableType>::iterator i = symbol_table.begin();
                for(int o = 0; o < symbol_table.size(); o++) {
                    advance(i, o);
                    out << setw(15) << i->id;
                    out << setw(15) << i->name; 
                    out << setw(15) << i->section; 
                    out << setw(15) << i->value; 
                    out << setw(15) << i->scope; 
                    out << endl;
                    i = symbol_table.begin();
                }
                out<<endl;
            }
            else {
                tokens = flexLexer->yylex(); 
                break;
            }
        }   // end of if
    } // end of while

    if(relocation_table.size() > 0) {
        out<<"____________________________________________________________________________________"<<endl;
        out<<endl;
    }

    // Relocation table
    list<RelocationTableType>::iterator ww = relocation_table.begin();
    string name;
    string name_before;
    name_before = "";
    for(int qq = 0; qq < relocation_table.size(); qq++) {
        advance(ww, qq);
        name = ww->getSectionName();
        if(name == name_before) {
            out << setw(15) << ww->getOffset();
            out << setw(15) << ww->getRelType();
            out << setw(15) << ww->getSymbId();
            out << endl;
        }
        else {
            out << setw(15) << "#.rel." + name << endl;
            out << setw(15) << "#OFFSET" << setw(15) << "TYPE" << setw(15) << "REF_ID" << endl;
            out << setw(15) << ww->getOffset();
            out << setw(15) << ww->getRelType();
            out << setw(15) << ww->getSymbId();
            out << endl;

            name_before = name;
        }
        ww = relocation_table.begin();
    }

    if(code_table.size() > 0) {
        out<<endl;
        out<<"____________________________________________________________________________________"<<endl;
        out<<endl;
    }

    // Code
    list<RelocationCodeType>::iterator w = code_table.begin();
    int cnt_data_list;
    cnt_data_list = 0;
    for(int q = 0; q < code_table.size(); q++) {
        advance(w, q);
        out<<"#." + w->getSectionName();
        out<<endl;
        for(string item : w->data) {
            out<<item<<" ";
            cnt_data_list++;
            if(cnt_data_list == 20) {
                cnt_data_list = 0;
                out<<endl;
            }
        }
        w = code_table.begin();
        out<<endl;
    }
    

    out.close();
    fbIn.close();
    symbol_table.clear();

    return 0;
}


