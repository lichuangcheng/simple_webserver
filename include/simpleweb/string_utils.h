#ifndef SIMPLEWEB_STRING_UTILS_INCLUDED
#define SIMPLEWEB_STRING_UTILS_INCLUDED


#include <string>
#include <string_view>
#include <algorithm>


namespace simpleweb {


std::string trim_left(std::string_view str)
{
	auto it  = str.begin();
	auto end = str.end();

	while (it != end && std::isspace(*it)) ++it;
	return std::string(it, end);
}


std::string trim_right(std::string_view str)
{
    std::ptrdiff_t pos = static_cast<std::ptrdiff_t>(str.size()) - 1;

	while (pos >= 0 && std::isspace(str[pos])) --pos;
	return std::string(str, 0, pos + 1);
}


std::string trim(std::string_view str)
{
	std::ptrdiff_t first = 0;
	std::ptrdiff_t last  = static_cast<std::ptrdiff_t>(str.size()) - 1;

	while (first <= last && std::isspace(str[first])) ++first;
	while (last >= first && std::isspace(str[last])) --last;

	return std::string(str, first, last - first + 1);
}


inline std::string to_upper(std::string s) 
{
    std::transform(s.begin(), s.end(), s.begin(), 
                   [](unsigned char c){ return std::toupper(c); }
                  );
    return s;
}


inline std::string to_lower(std::string s) 
{
    std::transform(s.begin(), s.end(), s.begin(), 
                   [](unsigned char c){ return std::tolower(c); } 
                  );
    return s;
}


bool iequals(std::string_view a, std::string_view b)
{
    return std::equal(a.begin(), a.end(),
                      b.begin(), b.end(),
                      [](uint8_t a, uint8_t b) {
                          return std::tolower(a) == std::tolower(b);
                      });
}


} // namespace simpleweb


#endif // SIMPLEWEB_STRING_UTILS_INCLUDED
