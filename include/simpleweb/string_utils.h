#ifndef SIMPLEWEB_STRING_UTILS_INCLUDED
#define SIMPLEWEB_STRING_UTILS_INCLUDED


#include <string>
#include <string_view>


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


} // namespace simpleweb


#endif // SIMPLEWEB_STRING_UTILS_INCLUDED
