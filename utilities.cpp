#include "utilities.h"

#include <string>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <stack>
#include <map>

namespace utilities
{
    const std::string WHITESPACE = " \n\r\t\f\v";

    std::string ltrim(const std::string& s)
    {
        size_t start = s.find_first_not_of(WHITESPACE);
        return (start == std::string::npos) ? "" : s.substr(start);
    }

    std::string rtrim(const std::string& s)
    {
        size_t end = s.find_last_not_of(WHITESPACE);
        return (end == std::string::npos) ? "" : s.substr(0, end + 1);
    }

    std::string trim(const std::string& s)
    {
        return rtrim(ltrim(s));
    }

    std::vector<std::string> split(const std::string& s, char delimiter)
    {
        std::vector<std::string> tokens;
        std::string token;
        std::istringstream tokenStream(s);
        while (std::getline(tokenStream, token, delimiter))
        {
            tokens.push_back(token);
        }
        return tokens;
    }

    bool replace(std::string& str, const std::string& from, const std::string& to)
    {
        size_t start_pos = str.find(from);
        if(start_pos == std::string::npos)
        {
            return false;
        }
        str.replace(start_pos, from.length(), to);
        return true;
    }

    std::vector<std::string> getValueList(std::string value)
    {
        if(value.empty())
        {
            return std::vector<std::string>();
        }

        value = utilities::trim(value);
        std::replace( value.begin(), value.end(), ',', ' ');
        while(replace(value, "  ", " "));
        auto list = utilities::split(value,' ');
#if defined(DEBUG)
        std::cout << "KeyValue List: [" << value << "]" << std::endl;
        for(auto &item : list)
        {
        std::cout << "Value: [" << item << "]" << std::endl;
            item = utilities::trim(item);
        }
#endif            
        return list;
    }


    std::string normalizePath(std::string &path)
    {
        std::string sep = (path.find("\\") != std::string::npos) ? "\\" : "/";

        std::stack<std::string> st;
        std::string dir;
        std::string res = sep;

        size_t len_path = path.length();

        for (size_t i = 0; i < len_path; i++)
        {
            dir.clear();
            while (path[i] == sep[0])
                i++;
            while (i < len_path && path[i] != sep[0]) {
                dir.push_back(path[i]);
                i++;
            }

            if (dir.compare("..") == 0)
            {
                if (!st.empty())
                    st.pop();
            }
            else if (dir.compare(".") == 0)
                continue;
            else if (dir.length() != 0)
                st.push(dir);
        }

        std::stack<std::string> st1;
        while (!st.empty())
        {
            st1.push(st.top());
            st.pop();
        }

        while (!st1.empty())
        {
            std::string temp = st1.top();
            
            if (st1.size() != 1)
            {
                res.append(temp + sep);
            }
            else
            {
                res.append(temp);
            }

            st1.pop();
        }

        return res;
    }
}
