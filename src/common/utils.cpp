/*
 * utils.cpp
 *
 *  Created on: Oct 28, 2013
 *      Author: Moy
 */
#include "utils.h"

namespace osu {

void trim(std::string& str, const std::string& chars)
{
	std::string::size_type pos1 = str.find_first_not_of(' ');
	std::string::size_type pos2 = str.find_last_not_of(' ');
	str = str.substr(pos1 == std::string::npos ? 0 : pos1,
	pos2 == std::string::npos ? str.length() - 1 : pos2 - pos1 + 1);
}

void tokenize(const std::string& str,
		std::vector<std::string>& tokens,
				  const std::string& delimiters) {
	// Skip delimiters at beginning.
	std::string::size_type lastPos = str.find_first_not_of(delimiters, 0);
	// Find first "non-delimiter".
	std::string::size_type pos     = str.find_first_of(delimiters, lastPos);

	while (std::string::npos != pos || std::string::npos != lastPos)
	{
		// Found a token, add it to the vector.
		std::string token = str.substr(lastPos, pos - lastPos);
		trim(token);
		tokens.push_back(token);
		// Skip delimiters.  Note the "not_of"
		lastPos = str.find_first_not_of(delimiters, pos);
		// Find next "non-delimiter"
		pos = str.find_first_of(delimiters, lastPos);
	}

}

void msg(bool condition, const std::string& message) {
	if (condition) std::cout << message << std::endl;
}

}
