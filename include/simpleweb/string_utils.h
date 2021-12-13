#ifndef SIMPLEWEB_STRING_UTILS_INCLUDED
#define SIMPLEWEB_STRING_UTILS_INCLUDED


#include <string>
#include <string_view>
#include <algorithm>
#include <vector>


namespace simpleweb {


inline std::string trim_left(std::string_view str)
{
	auto it  = str.begin();
	auto end = str.end();

	while (it != end && std::isspace(*it)) ++it;
	return std::string(it, end);
}


inline std::string trim_right(std::string_view str)
{
    std::ptrdiff_t pos = static_cast<std::ptrdiff_t>(str.size()) - 1;

	while (pos >= 0 && std::isspace(str[pos])) --pos;
	return std::string(str, 0, pos + 1);
}


inline std::string trim(std::string_view str)
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


std::vector<std::string_view> split(std::string_view s, std::string_view delimiter)
{
    size_t start = 0;
    size_t found = s.npos;
    std::vector<std::string_view> res;

    while((found = s.find(delimiter, start)) != s.npos)
    {
        res.push_back(s.substr(start, found - start));
        start = found + delimiter.length();
    }
    if (start != s.size()) res.push_back(s.substr(start));
    return res;
}


} // namespace simpleweb


#endif // SIMPLEWEB_STRING_UTILS_INCLUDED
