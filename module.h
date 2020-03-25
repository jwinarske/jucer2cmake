#pragma once

#include <string>
#include <vector>
#include <map>

class module
{
public:
    module(std::string basePath, std::string module);

    std::string getID();
    std::string getVendor();
    std::string getVersion();
    std::string getName();
    std::string getDescription();
    std::string getWebsite();
    std::string getLicense();
    std::string getDependencies();
    std::string getOSXFrameworks();
    std::string getiOSFrameworks();
    std::string getLinuxLibs();
    std::string getLinuxPackages();
    std::string getMingwLibs();

    friend std::ostream& operator<<(std::ostream& os, const module& mod);

private:
    void getMetaData(std::string inpfile);
    void listFilesRecursively(std::string basePath, std::string filename);

    std::string base_path;
    std::string sepd;
    std::string module_header;
    std::map<std::string, std::string> metaData;

    static constexpr const char* META_ID = "ID";
    static constexpr const char* META_VENDOR = "vendor";
    static constexpr const char* META_VERSION = "version";
    static constexpr const char* META_NAME = "name";
    static constexpr const char* META_DESCRIPTION = "description";
    static constexpr const char* META_WEBSITE = "website";
    static constexpr const char* META_LICENSE = "license";
    static constexpr const char* META_DEPENDENCIES = "dependencies";
    static constexpr const char* META_OSX_FRAMEWORKS = "OSXFrameworks";
    static constexpr const char* META_IOS_FRAMEWORKS = "iOSFrameworks";
    static constexpr const char* META_LINUX_LIBS = "linuxLibs";
    static constexpr const char* META_LINUX_PACKAGES = "linuxPackages";
    static constexpr const char* META_MINGW_LIBS = "mingwLibs";
    static constexpr const char* META_MINIMUM_CPP = "minimumCppStandard";
};
