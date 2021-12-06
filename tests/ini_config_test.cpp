#include "simpleweb/ini_config.h"
#include <sstream>
#include <iostream>
#include <assert.h>

using namespace std;

const double FLOAT_SEPERATION = 0.00000000001;
bool check_float(double test, double expected)
{
   double seperation = expected * (1 + FLOAT_SEPERATION) / expected;
   if ((test < expected + seperation) && (test > expected - seperation))
   {
      return true;
   }
   return false;
}

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
    ss << "hex = 0x43\n";
    ss << "oct = 044\n";
    // ss << "bin = 0b101010\n";

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

void check_result(simpleweb::IniConfig &conf)
{
   string expected_global_string = "global value";

   string expected_unreg_option = "unregistered_entry";
   string expected_unreg_value = "unregistered value";

   string expected_strings_word = "word";
   string expected_strings_phrase = "this is a phrase";
   string expected_strings_quoted = "\"quotes are in result\"";

   int expected_int_postitive = 41;
   int expected_int_negative = -42;
   int expected_int_hex = 0x43;
   int expected_int_oct = 044;
//    int expected_int_bin = 0b101010;

   float expected_float_positive = 51.1f;
   float expected_float_negative = -52.1f;
   double expected_float_double = 53.1234567890;
   float expected_float_int = 54.0f;
   float expected_float_int_dot = 55.0f;
   float expected_float_dot = .56f;
   float expected_float_exp_lower = 57.1e5f;
   float expected_float_exp_upper = 58.1E5f;
   float expected_float_exp_decimal = .591e5f;
   float expected_float_exp_negative = 60.1e-5f;
   float expected_float_exp_negative_val = -61.1e5f;
   float expected_float_exp_negative_negative_val = -62.1e-5f;

   bool expected_number_true = true;
   bool expected_number_false = false;
   bool expected_yn_true = true;
   bool expected_yn_false = false;
   bool expected_tf_true = true;
   bool expected_tf_false = false;
   bool expected_onoff_true = true;
   bool expected_onoff_false = false;

   assert(conf.get_string("global_string") == expected_global_string);
   assert(conf.get_string("unregistered_entry") == expected_unreg_value);

   assert(conf.get_string("strings.word") == expected_strings_word);
   assert(conf.get_string("strings.phrase") == expected_strings_phrase);
   assert(conf.get_string("strings.quoted") == expected_strings_quoted);

   assert(conf.get_int32("ints.positive") == expected_int_postitive);
   assert(conf.get_int32("ints.negative") == expected_int_negative);
   assert(conf.get_int32("ints.hex") == expected_int_hex);
   assert(conf.get_int32("ints.oct") == expected_int_oct);
//    assert(conf.get_int32("ints.bin") == expected_int_bin);

   assert(check_float(conf.get_float("floats.positive"), expected_float_positive));
   assert(check_float(conf.get_float("floats.negative"), expected_float_negative));
   assert(check_float(conf.get_float("floats.double"), expected_float_double));
   assert(check_float(conf.get_float("floats.int"), expected_float_int));
   assert(check_float(conf.get_float("floats.int_dot"), expected_float_int_dot));
   assert(check_float(conf.get_float("floats.dot"), expected_float_dot));
   assert(check_float(conf.get_float("floats.exp_lower"), expected_float_exp_lower));
   assert(check_float(conf.get_float("floats.exp_upper"), expected_float_exp_upper));
   assert(check_float(conf.get_float("floats.exp_decimal"), expected_float_exp_decimal));
   assert(check_float(conf.get_float("floats.exp_negative"), expected_float_exp_negative));
   assert(check_float(conf.get_float("floats.exp_negative_val"), expected_float_exp_negative_val));
   assert(check_float(conf.get_float("floats.exp_negative_negative_val"), expected_float_exp_negative_negative_val));

   assert(conf.get_bool("booleans.number_true") == expected_number_true);
   assert(conf.get_bool("booleans.number_false") == expected_number_false);
   assert(conf.get_bool("booleans.yn_true") == expected_yn_true);
   assert(conf.get_bool("booleans.yn_false") == expected_yn_false);
   assert(conf.get_bool("booleans.tf_true") == expected_tf_true);
   assert(conf.get_bool("booleans.tf_false") == expected_tf_false);
   assert(conf.get_bool("booleans.onoff_true") == expected_onoff_true);
   assert(conf.get_bool("booleans.onoff_false") == expected_onoff_false);
}

int main([[maybe_unused]] int argc, 
         [[maybe_unused]] char const *argv[])
{
    simpleweb::IniConfig conf;
    auto ss = make_file();
    conf.load(ss);
    check_result(conf);
    return 0;
}
