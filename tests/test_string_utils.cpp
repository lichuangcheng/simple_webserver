#include <iostream>
#include <assert.h>
#include "simpleweb/string_utils.h"

using namespace std;
using simpleweb::split;

int main(int argc, char const *argv[])
{
   string_view s1 = "abc++12356++cba";
   auto split_1 = split(s1, "++");

   assert(split_1.size() == 3);
   assert(split_1[0] == "abc");
   assert(split_1[1] == "12356");
   assert(split_1[2] == "cba");

   string_view s2 = "111 @ 222 @ cba @ ";
   auto split_2 = split(s2, " @ ");
   
   assert(split_2.size() == 3);
   assert(split_2[0] == "111");
   assert(split_2[1] == "222");
   assert(split_2[2] == "cba");
   return 0;
}



