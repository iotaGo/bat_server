#ifdef _WIN32
#pragma warning(disable: 4786)
#pragma warning(disable: 4503)
#endif

#include <stdio.h>
#include <stdlib.h>
#include "stringutils.h"
#include <algorithm>
#include <set>
#include <string>
#include <stdarg.h>
#include <locale>
#include <map>

#ifndef ABS
#define ABS(a) (((a)<0)?(-(a)):(a))
#endif

int StringUtils::SplitString(const std::string& input, 
							 const std::string& delimiter, std::vector<std::string>& results, 
							 bool includeEmpties)
{
	int iPos = 0;
	int newPos = -1;
	int sizeS2 = (int)delimiter.size();
	int isize = (int)input.size();

	if( 
		( isize == 0 )
		||
		( sizeS2 == 0 )
		)
	{
		return 0;
	}

	std::vector<int> positions;

	newPos = (int)input.find(delimiter, 0);

	if( newPos < 0 )
	{ 
		results.push_back(input);
		return 1; 
	}

	int numFound = 0;

	while( newPos >= iPos )
	{
		numFound++;
		positions.push_back(newPos);
		iPos = newPos;
		newPos = (int)input.find(delimiter, iPos + sizeS2);
	}

	if( numFound == 0 )
	{
		return 0;
	}

	for( int i=0; i <= (int)positions.size(); ++i )
	{
		std::string s("");
		if( i == 0 ) 
		{ 
			s = input.substr( i, positions[i] ); 
		}
		else {
		int offset = positions[i-1] + sizeS2;
		if( offset < isize )
		{
			if( i == positions.size() )
			{
				s = input.substr(offset);
			}
			else if( i > 0 )
			{
				s = input.substr( positions[i-1] + sizeS2, 
					positions[i] - positions[i-1] - sizeS2 );
			}
		}
		}

		if( includeEmpties || ( s.size() > 0 ) )
		{
			results.push_back(s);
		}
	}
	return (int)results.size();
}

std::vector<std::string> StringUtils::split(const std::string & strtem, char a)
{
    std::vector<std::string> strvec;

    std::string::size_type pos1, pos2;
    pos2 = strtem.find(a);
    pos1 = 0;
    while (std::string::npos != pos2)
    {
            strvec.push_back(strtem.substr(pos1, pos2 - pos1));

            pos1 = pos2 + 1;
            pos2 = strtem.find(a, pos1);
    }
    strvec.push_back(strtem.substr(pos1));
    return strvec;
}

std::string StringUtils::join(const std::vector<std::string> & strlist, char a)
{
    std::string s;
    size_t n = strlist.size();
    for(size_t i=0;i<n;i++) {
        if(i>0)
            s += a;
        s += strlist[i];
    }
    return s;
}

std::string StringUtils::RemoveString(const std::string& input, 
									  const std::string& delimiter)
{
	std::string v;
	std::vector<std::string> results;
	int n = SplitString(input, delimiter, results, false);
	for(int i=0;i<n;i++)
		v += results[i];
	return v;
}

int StringUtils::strtok(const std::string& input, const std::string& seps , std::vector<std::string>& results)
{
	results.clear();
	std::string temp = input;
	char * token = ::strtok( (char*)temp.c_str(), seps.c_str());
	while( token != NULL )
	{
		/* While there are tokens in "string" */
		results.push_back(token);

		/* Get next token: */
		token = ::strtok( NULL, seps.c_str() );
	}
	return (int)results.size();
}

int StringUtils::strtok(const std::string& input, const std::string& seps, std::vector<std::string>& results, std::vector<int> & idxs)
{
	std::string temp = input;
	char * token = ::strtok( (char*)temp.c_str(), seps.c_str());
	while( token != NULL )
	{
		/* While there are tokens in "string" */
		results.push_back(token);

		int kkk = int(token-temp.c_str());
		idxs.push_back(kkk);

		/* Get next token: */
		token = ::strtok( NULL, seps.c_str() );
	}
	return (int)results.size();
}

std::string & StringUtils::ReplaceString(std::string& str, const std::string& to_replaced, const std::string& newchars)
{
    for(std::string::size_type pos(0); pos != std::string::npos; pos += newchars.length())
    {
        pos = str.find(to_replaced,pos);
        if(pos!=std::string::npos)
           str.replace(pos,to_replaced.length(),newchars);
        else
            break;
	}
    return   str;
}


std::string & StringUtils::RReplaceString(std::string& str, const std::string& to_replaced, const std::string& newchars)
{
    std::size_t pos = str.rfind(to_replaced);
    if (pos!=std::string::npos)
        str.replace (pos,to_replaced.length(),newchars);

    return   str;
}

std::string StringUtils::Trim(std::string& input)
{
	std::string s = input;
	int len = (int)s.length();
	while (len > 0) {
		char c = s.at(len - 1);
		if (c == ' ' || c == '\t' || c == '\r' || c == '\n')
			len--;
		else
			break;
	}
	s.resize(len);
	input = s;
	return s;
}

bytestring StringUtils::fromHexS(const std::string& input)
{
	char buff[3];
	bytestring result;
	unsigned int c;
	
	for(int i=0;i<input.size()/2;i++) {
		buff[0] = input[i*2];
		buff[1] = input[i*2+1];
		buff[2] = '\0';
		
		sscanf(buff, "%X", &c);
		result.push_back((unsigned char)c);
	}
	return result;
}

std::string StringUtils::toHexS(const bytestring& input)
{
	char buff[8];
	std::string result;
	
	for(int i=0;i<input.size();i++) {
		unsigned char c = input[i];
		sprintf(buff,"%02X",c);
		
		result+=buff;
	}
	return result;
}

std::string StringUtils::normalizeString(std::string as_HzString, int len)//GBK
{
    std::string result;

	int stringlen = (int)as_HzString.length();
    char * buff = new char[stringlen+1];
    strcpy(buff, as_HzString.c_str());

    int H,L;
    for(int i = 0; i < stringlen && i<len; i ++ ) {
        H = (unsigned char)(buff[i]);
        L = (unsigned char)(buff[i+1]);
        if(H < 0xA1 || L < 0xA1) {
            result += buff[i];
        } else {
            if(i+1<len) {
                result += buff[i];
                result += buff[i+1];
            }
            i++;
        }
    }
    delete[]buff;
    return result;
}

bool StringUtils::startswith(const std::string& str, const std::string& start)
{
    int srclen = str.size();
    int startlen = start.size();
    if(srclen >= startlen)
    {
        std::string temp = str.substr(0, startlen);
        if(temp == start)
            return true;
    }

    return false;
}

bool StringUtils::endswith(const std::string& str, const std::string& end)
{
    int srclen = str.size();
    int endlen = end.size();
    if(srclen >= endlen)
    {
        std::string temp = str.substr(srclen - endlen, endlen);
        if(temp == end)
            return true;
    }

    return false;
}

std::string StringUtils::right(const std::string& str, size_t n)
{
    std::vector<std::string> tokens;
	int len = (int)str.length();
    const char * buff = str.c_str();

    int H,L;
	for (int i = 0; i < len; i++) {
        std::string result;
        H = (unsigned char)(buff[i]);
        L = (unsigned char)(buff[i+1]);
        if(H < 0xA1 || L < 0xA1) {
            result += buff[i];
        } else {
            if(i+1<len) {
                result += buff[i];
                result += buff[i+1];
            }
            i++;
        }
        tokens.push_back(result);
    }

    std::string result;
    size_t c = tokens.size();
    while(result.size()<n && c>0) {
        std::string last = tokens[c-1];
        if(last.size()+result.size()<=n) {
            result = last + result;
        }
        else
            break;
        c--;
    }
    return result;
}

std::vector<std::pair<int, int>> StringUtils::str2pos(const std::string& str)
{
	std::vector<std::pair<int, int>> ret, emp;

	if (!str.length())
		return emp;

	std::vector<std::string> pos_str_vec = StringUtils::split(str, ' ');
	for (int i = 0; i < pos_str_vec.size(); ++i)
	{
		const std::string& pos_str = pos_str_vec[i];
		std::vector<std::string> pos_vec = StringUtils::split(pos_str, ',');
		if (pos_vec.size() < 2)
			return emp;

		ret.push_back(std::pair<int, int>(atoi(pos_vec[0].c_str()), atoi(pos_vec[1].c_str())));
	}

	return ret;
}

bool StringUtils::time_split(const std::string& str, int &data, std::string& unit)
{
	if (str.empty())
		return false;

	std::string d = str;
	if (endswith(str, "sec") || endswith(str, "s"))
	{
		unit = "s";
		d = RemoveString(d, "sec");
		d = RemoveString(d, "s");
	}
	else if (endswith(str, "min") || endswith(str, "m"))
	{
		unit = "m";
		d = RemoveString(d, "min");
		d = RemoveString(d, "m");
	}
	else if (endswith(str, "hour") || endswith(str, "h"))
	{
		unit = "h";
		d = RemoveString(d, "hour");
		d = RemoveString(d, "h");
	}
	else if (endswith(str, "day") || endswith(str, "d"))
	{
		unit = "d";
		d = RemoveString(d, "day");
		d = RemoveString(d, "d");
	}
	else if (endswith(str, "month") || endswith(str, "M"))
	{
		unit = "M";
		d = RemoveString(d, "month");
		d = RemoveString(d, "M");
	}
	else if (endswith(str, "year") || endswith(str, "Y"))
	{
		unit = "Y";
		d = RemoveString(d, "year");
		d = RemoveString(d, "Y");
	}
	else
		return false;

	if (d.empty() || d == str)
		return false;

	data = atoi(d.c_str());

	return true;
}
