#include "simpleweb/ini_config.h"
#include <sstream>
#include <iostream>

std::stringstream make_file()
{
    std::stringstream ss;
    ss << "# This file checks parsing of various types of config values\n";
    //FAILS: ss << "; a windows style comment\n";

    ss << "global_string = global value # hello comment \n";
    ss << "unregistered_entry = unregistered value;# hello comment \n";

    ss << "\n[ strings]\n";
    ss << "word = word\n";
    ss << "phrase = this is a phrase ;hello comment \n";
    ss << "quoted = \"quotes are in result\"\n";
    
    ss << "\n[ints  ]\n";
    ss << "positive = 41\n";
    ss << "negative = -42\n";
    //FAILS: Lexical cast doesn't support hex, oct, or bin
    //ss << "hex = 0x43\n";
    //ss << "oct = 044\n";
    //ss << "bin = 0b101010\n";

    ss << "\n[  floats]\n";
    ss << "positive = 51.1\n";
    ss << "negative = -52.1\n";
    ss << "double = 53.1234567890\n";
    ss << "int = 54\n";
    ss << "int_dot = 55.\n";
    ss << "dot = .56\n";
    ss << "exp_lower = 57.1e5\n";
    ss << "exp_upper = 58.1E5\n";
    ss << "exp_decimal = .591e5\n";
    ss << "exp_negative = 60.1e-5\n";
    ss << "exp_negative_val = -61.1e5\n";
    ss << "exp_negative_negative_val = -62.1e-5\n";

    ss << "\n[  booleans ]\n";
    ss << "number_true = 1\n";
    ss << "number_false = 0\n";
    ss << "yn_true = yes\n";
    ss << "yn_false = no\n";
    ss << "tf_true = true\n";
    ss << "tf_false = false\n";
    ss << "onoff_true = on\n";
    ss << "onoff_false = off\n";
    ss << "present_equal_true = \n";
    //FAILS: Must be an = 
    //ss << "present_no_equal_true\n";

    ss.seekp(std::ios_base::beg);
    return ss;
}


int main(int argc, char const *argv[])
{
    simpleweb::IniConfig conf;
    auto ss = make_file();
    conf.load(ss);
    conf.display();
    return 0;
}
