#include "module.h"

#include <iostream>
#include <sstream>
#include <istream>
#include <fstream>
#include <algorithm>
#include <dirent.h>
#include <cstring>

#include "utilities.h"


void module::listFilesRecursively(std::string basePath, std::string filename)
{
    DIR *dir = opendir(basePath.c_str());
    if (!dir)
    {
        return;
    }

    std::string path;
    struct dirent *dp;
    while ((dp = readdir(dir)) != NULL)
    {
        if (strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0)
        {
            if (strcmp(dp->d_name, filename.c_str()) == 0)
            {
                module_header = basePath + sepd + dp->d_name;
                break;
            }

            // Construct new path from our base path
            path = basePath + sepd + std::string(dp->d_name);

            listFilesRecursively(path, filename);
        }
    }

    closedir(dir);
}

module::module(std::string basePath, std::string module)
{
    base_path = basePath;
    sepd = (base_path.find("\\") != std::string::npos) ? "\\" : "/";

#if defined(DEBUG)
    std::cout << "looking for module: " << module << std::endl;
#endif
    listFilesRecursively(basePath, module + ".h");

    if(!module_header.empty())
    {
        getMetaData( utilities::normalizePath(module_header) );
#if defined(DEBUG)
        std::cout << module << " : metaData " << metaData.size() << std::endl;
        std::cout << "dependencies length: " << metaData["dependencies"].length() << std::endl;
#endif
    }
}

std::string module::getID()
{
    return metaData [ META_ID ];
}

std::string module::getVendor()
{
    return metaData [ META_VENDOR ];
}

std::string module::getVersion()
{
    return metaData [ META_VERSION ];
}

std::string module::getName()
{
    return metaData [ META_NAME ];
}

std::string module::getDescription()
{
    return metaData [ META_DESCRIPTION ];
}

std::string module::getWebsite()
{
    return metaData[ META_WEBSITE ];
}

std::string module::getLicense()
{
    return metaData [ META_LICENSE ];
}

std::string module::getDependencies()
{
    return metaData [ META_DEPENDENCIES ];
}

std::string module::getOSXFrameworks()
{
    return metaData [ META_OSX_FRAMEWORKS ];
}

std::string module::getiOSFrameworks()
{
    return metaData [ META_IOS_FRAMEWORKS ];
}

std::string module::getLinuxLibs()
{
    return metaData [ META_LINUX_LIBS ];
}

std::string module::getLinuxPackages()
{
    return metaData [ META_LINUX_PACKAGES ];
}

std::string module::getMingwLibs()
{
    return metaData [ META_MINGW_LIBS ];
}

void module::getMetaData(std::string inpfile)
{
    enum metadata_state
    {
        start = 0,
        begin,
        end
    };

    std::vector<std::string> tokens;
    metadata_state state = start;
    std::stringstream ss;
    
    std::ifstream is(inpfile);
    if (!is)
        throw std::runtime_error("Error opening file: " + inpfile);

    std::string line;
    while (std::getline(is, line)) {

        if(state == start)
        {
            if(line.find("BEGIN_JUCE_MODULE_DECLARATION") != std::string::npos)
            {
                state = begin;
                continue;
            }
        }
        else if(state == begin)
        {
            if(line.find("END_JUCE_MODULE_DECLARATION") != std::string::npos)
            {
                state = end;
            }
            else
            {
                ss << line << " \n";
            }
        }
    }

    metaData.clear();

    std::string key;
    auto lines = utilities::split(ss.str(),'\n');
    for(auto &l : lines)
    {
        if(l.find(':') != std::string::npos)
        {
            auto kv = utilities::split(l, ':');
            
            std::string key = utilities::trim(kv[0]);
            std::string value = utilities::trim(kv[1]);

            metaData[key] = value;
        }
        else
        {
            if(l.length() > 2)
            {
                std::replace( l.begin(), l.end(), ',', ' ');
                metaData[key] += " " + l;
            }
        }
    }
#if defined(DEBUG)
    std::cout << "[dependencies] = [" << metaData["dependencies"] << "]" << std::endl;
#endif
}

std::ostream& operator<<(std::ostream& os, const module& mod)
{
    auto m = mod.metaData;

    os << "** Module **\n";
    os << "ID : " << m[ module::META_ID ] << "\n";
    os << "Vendor : " << m[ module::META_VENDOR ] << "\n";
    os << "Version : " << m[ module::META_VERSION ] << "\n";
    os << "Name : " << m[ module::META_NAME ] << "\n";
    os << "Description : " << m[ module::META_DESCRIPTION ] << "\n";
    os << "Website : " << m[ module::META_WEBSITE ] << "\n";
    os << "License : " << m[ module::META_LICENSE ] << "\n";

    auto list = utilities::getValueList( m[ module::META_DEPENDENCIES ] );
    for(auto const& item : list)
    {
        os << "<< dep >> " << item << "\n";
    }

    list = utilities::getValueList( m[ module::META_OSX_FRAMEWORKS ] );
    for(auto const& item : list)
    {
        os << "<< OSXFramework >> " << item << "\n";
    }

    list = utilities::getValueList( m[ module::META_IOS_FRAMEWORKS ] );
    for(auto const& item : list)
    {
        os << "<< iOSFrameworks >> " << item << "\n";
    }

    list = utilities::getValueList( m[ module::META_LINUX_LIBS ] );
    for(auto const& item : list)
    {
        os << "<< linuxLibs >> " << item << "\n";
    }

    list = utilities::getValueList( m[ module::META_LINUX_PACKAGES ] );
    for(auto const& item : list)
    {
        os << "<< linuxPackages >> " << item << "\n";
    }

    list = utilities::getValueList( m[ module::META_MINGW_LIBS ] );
    for(auto const& dep : list)
    {
        os << "<< mingwLibs >> " << dep << "\n";
    }

    os << "\n";

    return os;
}
