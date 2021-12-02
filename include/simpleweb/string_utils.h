#ifndef SIMPLEWEB_STRING_UTILS_INCLUDED
#define SIMPLEWEB_STRING_UTILS_INCLUDED


#include <string>
#include <string_view>


namespace simpleweb {


inline bool is_space(int c)
{
    return std::isblank(c);
}


std::string trim_left(std::string_view str)
{
	auto it  = str.begin();
	auto end = str.end();

	while (it != end && is_space(*it)) ++it;
	return std::string(it, end);
}


std::string trim_right(std::string_view str)
{
    std::ptrdiff_t pos = static_cast<std::ptrdiff_t>(str.size()) - 1;

	while (pos >= 0 && is_space(str[pos])) --pos;
	return std::string(str, 0, pos + 1);
}


std::string trim(std::string_view str)
{
	std::ptrdiff_t first = 0;
	std::ptrdiff_t last  = static_cast<std::ptrdiff_t>(str.size()) - 1;

	while (first <= last && is_space(str[first])) ++first;
	while (last >= first && is_space(str[last])) --last;

	return std::string(str, first, last - first + 1);
}


} // namespace simpleweb


#endif // SIMPLEWEB_STRING_UTILS_INCLUDED
