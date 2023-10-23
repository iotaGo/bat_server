#ifndef STRINGUTILS_H
#define STRINGUTILS_H

#include <vector>
#include <string>
#include <string.h>

typedef std::vector<unsigned char> bytestring;

class  StringUtils
{
public:
	static std::string RemoveString(const std::string& input, 
		const std::string& delimiter);

	static int SplitString(const std::string& input, 
		const std::string& delimiter, std::vector<std::string>& results, 
		bool includeEmpties = false);

    static std::vector<std::string> split(const std::string & strtem, char a);
	static std::string join(const std::vector<std::string> & strlist, char a);

	static int strtok(const std::string& input, const std::string& seps, 
		std::vector<std::string>& results);

	static int strtok(const std::string& input, const std::string& seps, 
		std::vector<std::string>& results, std::vector<int> & idxs);
	
	static std::string & ReplaceString(std::string& input, 
        const std::string& to_replaced, const std::string& newchars);

    static std::string & RReplaceString(std::string& input,
        const std::string& to_replaced, const std::string& newchars);

    static std::string Trim(std::string& input);

	static std::string toHexS(const bytestring& input);
	static bytestring fromHexS(const std::string& input);

	static std::string normalizeString(std::string as_HzString, int len);

    static std::string substract(std::string & left, std::string & right);

    static bool startswith(const std::string& str, const std::string& start);
    static bool endswith(const std::string& str, const std::string& end);

    static std::string right(const std::string& str, size_t len);
	static std::vector<std::pair<int, int>> str2pos(const std::string& str);

	/// 解析时间，如23h，2day，15s等 Meong 2023-6-5 ///
	static bool time_split(const std::string& str, int &data, std::string& unit);
};

#endif