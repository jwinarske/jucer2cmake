#include "project.h"

#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <sys/stat.h>
#include "utilities.h"

project::project()
{
    version = "1.0.0";
    pluginAAXCategory = -1;
    downloadJuceSource = false;
}

std::string project::get_resource_files()
{
    std::stringstream ss;

    pugi::xpath_node_set resources = m_Doc.select_nodes("/JUCERPROJECT/MAINGROUP/GROUP[@name = 'Resources']/FILE");

    ss << "\n";
    ss << "set(RESOURCE_FILES\n";
    std::string directory;
    for (pugi::xpath_node_set::const_iterator it = resources.begin(); it != resources.end(); ++it)
    {
        pugi::xpath_node node = *it;
        std::string file = node.node().attribute("file").as_string();
        directory = file.substr(0, file.find_first_of("\\/"));
        ss << "\n    " << file;
    }
    ss << "\n";
    ss << ")\n";
    ss << "\n";

    if(directory.empty())
    {
        std::cout << "Default Resource Folder Name" << std::endl;
        directory = "Resources";
    }

    ss << "if(RESOURCE_FILES)\n";
    ss << "    add_custom_command(\n";
    ss << "        OUTPUT BinaryData.cpp BinaryData.h\n";
    ss << "        COMMAND ${BINARY_BUILDER}\n";
    ss << "        ARGS ${CMAKE_CURRENT_SOURCE_DIR}/" << directory << " ${CMAKE_CURRENT_SOURCE_DIR}/JuceLibraryCode BinaryData\n";
    ss << "    )\n";
    ss << "    add_custom_target(BinaryData DEPENDS BinaryData.cpp BinaryData.h)\n";
    ss << "endif()\n";
    ss << "\n";

    return ss.str();
}

std::vector<std::string> project::get_export_formats()
{
    std::vector<std::string> res;

    pugi::xpath_node_set xformats = m_Doc.select_nodes("/JUCERPROJECT/EXPORTFORMATS");

    for (pugi::xpath_node_set::const_iterator it = xformats.begin(); it != xformats.end(); ++it)
    {
        pugi::xpath_node node = *it;
        for (pugi::xml_node child: node.node().children())
        {
            res.push_back(child.name());
        }
    }

    return res;
}

std::list<std::string> project::get_module_path_list()
{
    std::list<std::string> list;
    std::vector<std::string> export_formats = get_export_formats();

    for(auto const& format : export_formats)
    {
        std::string xpath = "/JUCERPROJECT/EXPORTFORMATS/" + format + "/MODULEPATHS/MODULEPATH";
        pugi::xpath_node_set paths = m_Doc.select_nodes(xpath.c_str());
        for (pugi::xpath_node_set::const_iterator it = paths.begin(); it != paths.end(); ++it)
        {
            pugi::xpath_node node = *it;
            std::string path = node.node().attribute("path").value();
            if(!path.empty())
            {
                list.push_back(path);
            }
            else
            {
                // keep looking for non-empty path
                continue;
            }
        }
    }

    list.unique();

    return list;
}

std::string project::get_module_paths()
{
    std::stringstream ss;
    auto list = get_module_path_list();
    for(auto const& path : list)
    {
        ss << "    " << path << "\n";
    }
    return ss.str();
}

std::string project::get_autogen_vars()
{
    std::stringstream ss;

    ss << "if(APPLE)\n";
    ss << "    file(GLOB JUCE_LIBRARY_CODE_CPP RELATIVE ${CMAKE_SOURCE_DIR} CONFIGURE_DEPENDS\n";
    ss << "       JuceLibraryCode/*.mm\n";
    ss << "       JuceLibraryCode/BinaryData.cpp\n";
    ss << "    )\n";
    ss << "    if(CMAKE_BUILD_TYPE STREQUAL \"Debug\")\n";
    ss << "        add_compile_definitions(DEBUG)\n";
    ss << "    endif()\n";
    ss << "else()\n";
    ss << "    file(GLOB JUCE_LIBRARY_CODE_CPP RELATIVE ${CMAKE_SOURCE_DIR} CONFIGURE_DEPENDS JuceLibraryCode/*.cpp)\n";
    ss << "endif()";
    ss << "\n";

    return ss.str();
}

std::vector<std::string> project::get_module_list()
{
    std::vector<std::string> res;

    pugi::xpath_node_set set = m_Doc.select_nodes("/JUCERPROJECT/MODULES/MODULE");
    for (pugi::xpath_node_set::const_iterator it = set.begin(); it != set.end(); ++it)
    {
        pugi::xpath_node node = *it;
        res.push_back(node.node().attribute("id").value());
    }

    return res;
}

std::list<std::string> project::get_libraries(const project::map_t& system)
{
    std::list<std::string> res;
    for(auto &item : get_module_list())
    {
        //std::cout << "Module: " << item << "\n";
        if ( system.find(item) != system.end() )
        {
            auto libs = system.at(item);
            //std::cout << "libs: " << libs << "\n";
            for(auto const &lib : utilities::getValueList(libs))
            {
                //std::cout << "lib: [" << lib << "]\n";
                res.push_back(lib);
            }
        }
    }
    res.sort();
    res.unique();
    #if 0
    std::cout << "Libraries" << std::endl;
    for(auto const& item : res)
    {
        std::cout << "[" << item << "]" << std::endl;
    }
    #endif

    return res;
}

std::string project::get_modules()
{
    std::stringstream ss;

    pugi::xpath_node_set modules = m_Doc.select_nodes("/JUCERPROJECT/MODULES/MODULE");
    for (pugi::xpath_node_set::const_iterator it = modules.begin(); it != modules.end(); ++it)
    {
        pugi::xpath_node node = *it;
        ss << "    JuceLibraryCode/include_" << node.node().attribute("id").value() << ".cpp\n";
    }

    return ss.str();
}

std::string project::get_source_groups()
{
    std::stringstream ss;

    pugi::xpath_node_set sources = m_Doc.select_nodes("/JUCERPROJECT/MAINGROUP/GROUP[@name = 'Source']/FILE");
    ss << "\nsource_group (Source FILES\n";
    for (pugi::xpath_node_set::const_iterator it = sources.begin(); it != sources.end(); ++it)
    {
        pugi::xpath_node node = *it;
        if(!node.node().attribute("resource").as_bool()) {
            ss << "\n    " << node.node().attribute("file").value();
        }
    }
    ss << "\n";
    ss << ")\n";

    pugi::xpath_node_set source_groups = m_Doc.select_nodes("/JUCERPROJECT/MAINGROUP/GROUP[@name = 'Source']/GROUP");
    for (pugi::xpath_node_set::const_iterator it = source_groups.begin(); it != source_groups.end(); ++it)
    {
        pugi::xpath_node node = *it;
        ss << "\nsource_group (Source\\\\" << node.node().attribute("name").value() << " FILES\n";

        std::stringstream q;
        q << "/JUCERPROJECT/MAINGROUP/GROUP/GROUP[@name = '" << node.node().attribute("name").value() << "']/FILE";
        pugi::xpath_node_set sub_source = m_Doc.select_nodes(q.str().c_str());
        for (pugi::xpath_node_set::const_iterator it = sub_source.begin(); it != sub_source.end(); ++it)
        {
            pugi::xpath_node node = *it;
            ss << "\n    " << node.node().attribute("file").value();
        }
        ss << "\n)\n";
    }
    ss << "\n";

    return ss.str();
}

std::string project::get_source_list()
{
    std::stringstream ss;

    ss << "set(SRC_FILES\n";

    pugi::xpath_node_set files = m_Doc.select_nodes("/JUCERPROJECT/MAINGROUP/GROUP/FILE");
    for (pugi::xpath_node_set::const_iterator it = files.begin(); it != files.end(); ++it)
    {
        pugi::xpath_node node = *it;
        if(node.node().attribute("compile").as_bool()) {
            ss << "\n    " << node.node().attribute("file").value();
        }
    }

    files = m_Doc.select_nodes("/JUCERPROJECT/MAINGROUP/GROUP/GROUP/FILE");
    for (pugi::xpath_node_set::const_iterator it = files.begin(); it != files.end(); ++it)
    {
        pugi::xpath_node node = *it;
        if(node.node().attribute("compile").as_bool()) {
            ss << "\n    " << node.node().attribute("file").value();
        }
    }

    files = m_Doc.select_nodes("/JUCERPROJECT/MAINGROUP/GROUP/GROUP/GROUP/FILE");
    for (pugi::xpath_node_set::const_iterator it = files.begin(); it != files.end(); ++it)
    {
        pugi::xpath_node node = *it;
        if(node.node().attribute("compile").as_bool()) {
            ss << "\n    " << node.node().attribute("file").value();
        }
    }

    files = m_Doc.select_nodes("/JUCERPROJECT/MAINGROUP/GROUP/GROUP/GROUP/GROUP/FILE");
    for (pugi::xpath_node_set::const_iterator it = files.begin(); it != files.end(); ++it)
    {
        pugi::xpath_node node = *it;
        if(node.node().attribute("compile").as_bool()) {
            ss << "\n    " << node.node().attribute("file").value();
        }
    }

    ss << "\n";
    ss << ")\n";
    ss << "\n";
    return ss.str();
}

project::project(std::string file, std::string outpath)
{
    static constexpr char DEFAULT_VERSION[] = { '1', '.', '0', '.', '0', 0 };

    version = DEFAULT_VERSION;

    base_path = file.substr(0, file.find_last_of("\\/"));
    sepd = (base_path.find("\\") != std::string::npos) ? "\\" : "/";

    if(outpath.empty())
    {
        output_path = base_path;
    }
    else
    {
        output_path = outpath;
    }

    pugi::xml_parse_result result = m_Doc.load_file(file.c_str());
    if (!result)
    {
        std::cout << "XML [" << file << "] parsed with errors\n";
        std::cout << "Error description: " << result.description() << "\n";
        std::cout << "Error offset: " << result.offset << " (error at [..." << (file.c_str() + result.offset) << "]\n" << std::endl;
    }

    for (pugi::xml_node node: m_Doc.children("JUCERPROJECT"))
    {
        id = node.attribute("id").as_string();
        name = node.attribute("name").as_string();
        displaySplashScreen = node.attribute("displaySplashScreen").as_bool();
        reportAppUsage  = node.attribute("reportAppUsage").as_bool();
        splashScreenColour = node.attribute("splashScreenColour").as_string();
        projectType = node.attribute("projectType").as_string();
        juceFolder = node.attribute("juceFolder").as_string();
        std::string _version = node.attribute("version").as_string();
        if(!_version.empty())
        {
            version = _version;
        }
        bundleIdentifier = node.attribute("bundleIdentifier").as_string();
        includeBinaryInAppConfig = node.attribute("includeBinaryInAppConfig").as_bool();
        cppLanguageStandard = node.attribute("cppLanguageStandard").as_string();
        companyCopyright = node.attribute("companyCopyright").as_string();
        buildVST = node.attribute("buildVST").as_bool();
        buildVST3 = node.attribute("buildVST3").as_bool();
        buildAU = node.attribute("buildAU").as_bool();
        buildAUv3 = node.attribute("buildAUv3").as_bool();
        buildRTAS = node.attribute("buildRTAS").as_bool();
        buildAAX = node.attribute("buildAAX").as_bool();
        buildStandalone = node.attribute("buildStandalone").as_bool();
        buildUnity = node.attribute("buildUnity").as_bool();
        enableIAA = node.attribute("enableIAA").as_bool();
        pluginName = node.attribute("pluginName").as_string();
        pluginDesc = node.attribute("pluginDesc").as_string();
        pluginManufacturer = node.attribute("pluginManufacturer").as_string();
        pluginManufacturerCode = node.attribute("pluginManufacturerCode").as_string();
        pluginCode = node.attribute("pluginCode").as_string();
        pluginChannelConfigs = node.attribute("pluginChannelConfigs").as_string();
        pluginIsSynth = node.attribute("pluginIsSynth").as_bool();
        pluginWantsMidiIn = node.attribute("pluginWantsMidiIn").as_bool();
        pluginProducesMidiOut = node.attribute("pluginProducesMidiOut").as_bool();
        pluginIsMidiEffectPlugin = node.attribute("pluginIsMidiEffectPlugin").as_bool();
        pluginEditorRequiresKeys= node.attribute("pluginEditorRequiresKeys").as_bool();
        pluginAUExportPrefix = node.attribute("pluginAUExportPrefix").as_string();
        aaxIdentifier = node.attribute("aaxIdentifier").as_string();
        pluginAAXCategory = node.attribute("pluginAAXCategory").as_int();
        jucerVersion = node.attribute("jucerVersion").as_string();
        companyName = node.attribute("companyName").as_string();
        std::string _headerPath = node.attribute("headerPath").as_string();
        if(!_headerPath.empty())
        {
            headerPath = utilities::split(_headerPath, '\n');
        }
        companyWebsite = node.attribute("companyWebsite").as_string();
        defines = node.attribute("defines").as_string();
        pluginFormats = node.attribute("pluginFormats").as_string();
        pluginCharacteristicsValue = node.attribute("pluginCharacteristicsValue").as_string();
        userNotes = node.attribute("userNotes").as_string();
    }
}

std::string project::get_defines()
{
    std::stringstream ss;
    ss << "\n";
    ss << "add_compile_definitions(\n";

    if(!defines.empty())
    {
        ss << "    " << defines << "\n";
    }

    ss << "    JUCE_APP_VERSION=" << version << "\n";
    ss << "    JUCE_APP_VERSION_HEX=0x";
    auto ver = utilities::split(version, '.');
    ss << std::hex << std::setw(2) << std::setfill('0') << ver[0];
    ss << std::hex << std::setw(2) << std::setfill('0') << ver[1];
    ss << std::hex << std::setw(2) << std::setfill('0') << ver[2];
    ss << "\n";
    ss << "    JucePlugin_Build_VST=" << buildVST << "\n";
    ss << "    JucePlugin_Build_VST3=" << buildVST3 << "\n";
    ss << "    JucePlugin_Build_AU=" << buildAU << "\n";
    ss << "    JucePlugin_Build_AUv3=" << buildAUv3 << "\n";
    ss << "    JucePlugin_Build_RTAS=" << buildRTAS << "\n";
    ss << "    JucePlugin_Build_AAX=" << buildAAX << "\n";
    ss << "    JucePlugin_Build_Standalone=" << buildStandalone << "\n";
    ss << "    JucePlugin_Build_Unity=" << buildUnity << "\n";
    ss << ")\n";
    ss << "\n";
    return ss.str();
}

std::string project::get_include_dirs()
{
    std::stringstream ss;
    ss << "include_directories(\n";
    ss << "\n";
    ss << "    JuceLibraryCode\n";
    if(!downloadJuceSource)
    {
        ss << get_module_paths();
    }
    else
    {
        ss << "    ${JUCE_ROOT}/modules\n";
    }
    
    if(!headerPath.empty())
    {
        for (auto & path : headerPath) {
            ss << "    " << path << "\n";
        }
    }
    ss << ")\n";
    ss << "\n";
    return ss.str();
}

std::string project::get_header()
{
    std::stringstream ss;

    ss << "################################################\n";
    ss << "#\n";
    ss << "#    This file was auto-generated by jucer2cmake:\n";
    ss << "#        https://github.com/jwinarske/jucer2cmake\n";
    ss << "#\n";
    ss << "################################################\n";
    ss << "\n";
    ss << "\n";
    ss << "cmake_minimum_required(VERSION 3.11)\n";
    ss << "\n";
    ss << "if(NOT CMAKE_BUILD_TYPE)\n";
    ss << "    set(CMAKE_BUILD_TYPE \"Release\" CACHE STRING \"Choose the type of build, options are: Debug, Release, or MinSizeRel.\" FORCE)\n";
    ss << "    message(STATUS \"CMAKE_BUILD_TYPE not set, defaulting to Release.\")\n";
    ss << "endif()\n";
    ss << "\n";
    ss << "project(" << name << " ";
    if (!pluginDesc.empty())
    {
        ss << "DESCRIPTION \"" << pluginDesc << "\"\n";
    }
    ss << "LANGUAGES CXX C)\n";
    ss << "\n";
    ss << "message(STATUS \"Generator .............. ${CMAKE_GENERATOR}\")\n";
    ss << "message(STATUS \"Build Type ............. ${CMAKE_BUILD_TYPE}\")\n";
    ss << "message(STATUS \"AppVersion ............. " << version << "\")\n";
    ss << "\n";

    return ss.str();
}

std::string project::get_dependencies()
{
    auto list = get_module_path_list();

    std::stringstream ss;
    ss << "include(ExternalProject)\n";
    ss << "\n";

    for(auto const& path : list)
    {
        std::string sepd = (path.find("\\") != std::string::npos) ? "\\" : "/";

        struct stat info;
        std::string pathname = base_path + sepd + path;
        if( stat( pathname.c_str(), &info ) == 0 )
        {
            std::cout << "Valid: " << base_path << sepd << path << std::endl;
        }
        else
        {
            std::cout << "Path not present: " << base_path << sepd << path << std::endl;
            std::cout << "Configured to download JUCE" << std::endl;

            if(juceFolder.empty())
            {
                ss << "set(JUCE_ROOT ${CMAKE_CURRENT_BINARY_DIR}/juce)\n";
                ss << "\n";
                ss << "ExternalProject_Add(juce_root\n";
                ss << "    GIT_REPOSITORY https://github.com/WeAreROLI/JUCE.git\n";
                ss << "    GIT_TAG " << jucerVersion << "\n";
                ss << "    GIT_SHALLOW 1\n";
                ss << "    BUILD_IN_SOURCE 0\n";
                ss << "    SOURCE_DIR ${JUCE_ROOT}\n";
                ss << "    PATCH_COMMAND \"\"\n";
                ss << "    UPDATE_COMMAND \"\"\n";
                ss << "    CONFIGURE_COMMAND \"\"\n";
                ss << "    BUILD_COMMAND \"\"\n";
                ss << "    INSTALL_COMMAND \"\"\n";
                ss << ")\n";
                ss << "set(EXTERNAL_JUCE TRUE)\n";
                ss << "\n";
                downloadJuceSource = true;
            }
            else
            {
                ss << "set(JUCE_ROOT " << path << ")\n";
            }
        }
    }

    return ss.str();
}

std::string project::get_cpp_standard()
{
    if(cppLanguageStandard.empty())
    {
        cppLanguageStandard = "11";
    }

    std::stringstream ss;
    ss << "set(CMAKE_CXX_STANDARD " << cppLanguageStandard << ")\n";
    ss << "set(CMAKE_CXX_STANDARD_REQUIRED ON)\n";
    ss << "set(CMAKE_CXX_EXTENSIONS OFF)\n";
    return ss.str();
}

std::string project::get_executable()
{
    if(!name.empty())
    {
        std::stringstream ss;
        if(projectType == "consoleapp" || projectType == "guiapp")
        {
            ss << "add_executable(" << name << " ${SRC_FILES} ${JUCE_LIBRARY_CODE_CPP})\n";
        }
        else if(projectType == "audioplug" || projectType == "dll" || projectType == "library")
        {
            ss << "add_library(" << name << " SHARED ${SRC_FILES} ${JUCE_LIBRARY_CODE_CPP})\n";
        }

        ss << "if(EXTERNAL_JUCE)\n";
        ss << "    add_dependencies(" << name << " juce_root)\n";
        ss << "endif()\n";
        ss << "\n";

        return ss.str();
    }
    return "";
}

std::string project::get_common_options()
{
    std::stringstream ss;
    ss << "if (CMAKE_CXX_COMPILER_ID MATCHES \"Clang\")\n";
    ss << "    if(CMAKE_BUILD_TYPE STREQUAL \"Release\")\n";
    ss << "        target_compile_options(" << name << " PUBLIC -flto)\n";
    ss << "        target_link_options(" << name << " PUBLIC -flto)\n";
    ss << "    elseif(CMAKE_BUILD_TYPE STREQUAL \"Debug\")\n";
    ss << "        target_compile_options(" << name << " PUBLIC -fsanitize=address -fno-omit-frame-pointer)\n";
    ss << "        target_link_options(" << name << " PUBLIC -fsanitize=address)\n";
    ss << "    endif()\n";
    ss << "endif()\n";
    ss << "\n";
    return ss.str();
}

void project::get_export(std::string target, project::buildExport &build)
{
    std::string xpath = "/JUCERPROJECT/EXPORTFORMATS/" + target;
    pugi::xpath_node_set set = m_Doc.select_nodes(xpath.c_str());

    build.valid = false;
    build.debug.valid = false;
    build.release.valid = false;
    
    for (pugi::xpath_node_set::const_iterator it = set.begin(); it != set.end(); ++it)
    {
        pugi::xpath_node node = *it;
        build.targetFolder = node.node().attribute("targetFolder").value();
        std::string _extraDefs = node.node().attribute("extraDefs").value();
        if(!_extraDefs.empty())
        {
            build.extraDefs = utilities::split(_extraDefs, '\n');
        }
        std::string _externalLibraries = node.node().attribute("externalLibraries").value();
        if(!_externalLibraries.empty())
        {
            build.externalLibraries = utilities::split(_externalLibraries, '\n');
        }
        build.extraLinkerFlags = node.node().attribute("extraLinkerFlags").value();
        build.cppLanguageStandard = node.node().attribute("cppLanguageStandard").value();
        build.extraCompilerFlags = node.node().attribute("extraCompilerFlags").value();
        build.valid = true;
    }

    xpath = "/JUCERPROJECT/EXPORTFORMATS/" + target + "/CONFIGURATIONS/CONFIGURATION[@name='Debug']";
    set = m_Doc.select_nodes(xpath.c_str());

    for (pugi::xpath_node_set::const_iterator it = set.begin(); it != set.end(); ++it)
    {
        pugi::xpath_node node = *it;
        build.debug.name = node.node().attribute("name").as_string();
        build.debug.isDebug = node.node().attribute("isDebug").as_bool();
        build.debug.optimisation = node.node().attribute("optimisation").as_bool();
        build.debug.targetName = node.node().attribute("targetName").as_string();
        std::string _headerPath = node.node().attribute("headerPath").as_string();
        if(!_headerPath.empty())
        {
            build.debug.headerPath = utilities::split(_headerPath, '\n');
        }
        std::string _libraryPath = node.node().attribute("libraryPath").as_string();
        if(!_libraryPath.empty())
        {
            build.debug.libraryPath = utilities::split(_libraryPath, '\n');
        }
        build.debug.valid = true;
    }

    xpath = "/JUCERPROJECT/EXPORTFORMATS/" + target + "/CONFIGURATIONS/CONFIGURATION[@name='Release']";
    set = m_Doc.select_nodes(xpath.c_str());

    for (pugi::xpath_node_set::const_iterator it = set.begin(); it != set.end(); ++it)
    {
        pugi::xpath_node node = *it;
        build.release.name = node.node().attribute("name").as_string();
        build.release.isDebug = node.node().attribute("isDebug").as_bool();
        build.release.optimisation = node.node().attribute("optimisation").as_bool();
        build.release.targetName = node.node().attribute("targetName").as_string();
        std::string _headerPath = node.node().attribute("headerPath").as_string();
        if(!_headerPath.empty())
        {
            build.release.headerPath = utilities::split(_headerPath, '\n');
        }
        std::string _libraryPath = node.node().attribute("libraryPath").as_string();
        if(!_libraryPath.empty())
        {
            build.release.libraryPath = utilities::split(_libraryPath, '\n');
        }
        build.release.valid = true;
    }
}

std::string project::get_apple_ios_config()
{
    std::stringstream ss;

    project::buildExport b;
    get_export("XCODE_IPHONE", b);
    if(!b.valid)
    {
        return "";
    }
    std::cout << "Using iOS Config" << std::endl;

    if(!b.debug.headerPath.empty())
    {
        ss << "            target_include_directories(" << name << " PUBLIC\n";
        for(auto const& path : b.debug.headerPath)
        {
            ss << "                " << path << "\n";
        }
        ss << "            )\n";
        ss << "\n";
    }

    if(!b.extraCompilerFlags.empty())
    {
        ss << "            target_compile_options(" << name << " PUBLIC " << b.extraCompilerFlags << ")\n";
        ss << "\n";
    }

    if(!b.debug.libraryPath.empty())
    {
        ss << "            target_link_directories(" << name << " BEFORE PUBLIC\n";
        for(auto const& path : b.debug.libraryPath)
        {
            ss << "                " << path << "\n";
        }
        ss << "            )\n";
        ss << "\n";
    }

    int count = 0;
    for(auto const& framework : get_libraries(iOSFrameworks))
    {
        ss << "            find_library(FRAMEWORK" << count++ << " " << framework << ")\n";
    }
    ss << "\n";
    ss << "            target_link_libraries(" << name << "\n";
    for(auto const& library : b.externalLibraries)
    {
        ss << "                " << library << "\n";
    }
    for(int i = 0; i < count; i++)
    {
        ss << "                ${FRAMEWORK" << i <<  "}\n";
    }
    ss << "            )\n";

    return ss.str();
}

std::string project::get_apple_osx_config()
{
    std::stringstream ss;

    project::buildExport b;
    get_export("XCODE_MAC", b);
    if(!b.valid)
    {
        return "";
    }
    std::cout << "Using XCode Config" << std::endl;

    if(!b.debug.headerPath.empty())
    {
        ss << "            target_include_directories(" << name << " PUBLIC\n";
        for(auto const& path : b.debug.headerPath)
        {
            ss << "                " << path << "\n";
        }
        ss << "            )\n";
        ss << "\n";
    }

    if(!b.extraCompilerFlags.empty())
    {
        ss << "            target_compile_options(" << name << " PUBLIC " << b.extraCompilerFlags << ")\n";
        ss << "\n";
    }

    if(!b.debug.libraryPath.empty())
    {
        ss << "            target_link_directories(" << name << " BEFORE PUBLIC\n";
        for(auto const& path : b.debug.libraryPath)
        {
            ss << "                " << path << "\n";
        }
        ss << "            )\n";
        ss << "\n";
    }

    int count = 0;
    for(auto const& framework : get_libraries(OSXFramework))
    {
        ss << "            find_library(FRAMEWORK" << count++ << " " << framework << ")\n";
    }
    ss << "\n";
    ss << "            target_link_libraries(" << name << "\n";
    for(auto const& library : b.externalLibraries)
    {
        ss << "                " << library << "\n";
    }
    for(int i = 0; i < count; i++)
    {
        ss << "                ${FRAMEWORK" << i <<  "}\n";
    }
    ss << "            )\n";

    return ss.str();
}

std::string project::get_linux_config()
{
    std::stringstream ss;

    project::buildExport b;
    get_export("LINUX_MAKE", b);
    if(!b.valid)
    {
        return "";
    }
    std::cout << "Using Linux Config" << std::endl;

    auto packages = get_libraries(linuxPackages);
    if(!packages.empty())
    {
        ss << "\n";
        ss << "        include(FindPkgConfig)\n";
        ss << "        pkg_check_modules(JUCE_LIBS REQUIRED\n";
        ss << "            ";
        for(auto const& package : packages)
        {
            ss << package << " ";
        }
        ss << "\n)\n";
    }
    if(!b.debug.headerPath.empty() || !packages.empty())
    {
        ss << "\n";
        ss << "        target_include_directories(" << name << " PUBLIC\n";
        if(!packages.empty())
        {
            ss << "            ${JUCE_LIBS_INCLUDE_DIRS}\n";
        }
        for(auto const& path : b.debug.headerPath)
        {
            ss << "            " << path << "\n";
        }
        ss << "        )\n";
    }
    if(!b.extraCompilerFlags.empty())
    {
        ss << "\n";
        ss << "        target_compile_options(" << name << " PUBLIC " << b.extraCompilerFlags << ")\n";
    }
    if(!b.debug.libraryPath.empty())
    {
        ss << "\n";
        ss << "        target_link_directories(" << name << " BEFORE PUBLIC\n";
        for(auto const& path : b.debug.libraryPath)
        {
            ss << "            " << path << "\n";
        }
        ss << "        )\n";
    }

    auto libs = get_libraries(linuxLibs);
    if(!b.externalLibraries.empty() || !packages.empty() || !libs.empty())
    {
        ss << "\n";
        ss << "        target_link_libraries(" << name << "\n";
        for(auto const& library : b.externalLibraries)
        {
            ss << "            " << library << "\n";
        }
        if(!packages.empty())
        {
            ss << "            ${JUCE_LIBS_LIBRARIES}\n";
        }
        if(!libs.empty())
        {
            ss << "            ";
            for(auto const& lib : get_libraries(linuxLibs))
            {
                ss << lib << " ";
            }
            ss << "\n";
        }
        ss << "        )\n";
    }

    return ss.str();
}

std::string project::get_msvc_config()
{
    std::stringstream ss;

    project::buildExport b;

    get_export("VS2019", b);
    if(!b.valid)
    {
        get_export("VS2017", b);
        if(!b.valid)
        {
            get_export("VS2015", b);
            if(b.valid)
            {
                std::cout << "Using VS2015 Config" << std::endl;
            }
            else
            {
                return "";
            }
            
        }
        else
        {
            std::cout << "Using VS2017 Config" << std::endl;
        }
    }
    else
    {
        std::cout << "Using VS2019 Config" << std::endl;
    }
    
    if(!b.debug.headerPath.empty())
    {
        ss << "\n";
        ss << "    target_include_directories(" << name << " PUBLIC\n";
        for(auto const& path : b.debug.headerPath)
        {
            ss << "        \"" << path << "\"\n";
        }
        ss << "    )\n";
        ss << "\n";
    }
    if(!b.extraCompilerFlags.empty() || !b.extraDefs.empty())
    {
        ss << "\n";
        ss << "    target_compile_options(" << name << " PUBLIC \n";
        if(!b.extraCompilerFlags.empty())
        {
            ss << "        " << b.extraCompilerFlags << "\n";
        }
        for(auto const& def : b.extraDefs)
        {
            ss << "        " << def << "\n";
        }
        ss << "    )\n";
    }
    if(!b.debug.libraryPath.empty())
    {
        ss << "\n";
        ss << "    target_link_directories(" << name << " BEFORE PUBLIC\n";
        for(auto const& path : b.debug.libraryPath)
        {
            ss << "        \"" << path << "\"\n";
        }
        ss << "    )\n";
    }
    if(!b.externalLibraries.empty() || !b.extraLinkerFlags.empty())
    {
        ss << "\n";
        ss << "    target_link_libraries(" << name << "\n";
        for(auto const& library : b.externalLibraries)
        {
            ss << "        " << library << "\n";
        }
        ss << "        " << b.extraLinkerFlags << "\n";
        ss << "    )\n";
    }

    return ss.str();
}

std::string project::get_target_config()
{
    std::stringstream ss;
    ss << "if(UNIX)\n";
    ss << "    if(APPLE)\n";
    ss << "        if(${CMAKE_SYSTEM_NAME} STREQUAL \"Darwin\")\n";
    ss <<              get_apple_osx_config();
    ss << "        elseif(${CMAKE_SYSTEM_NAME} STREQUAL \"iOS\")\n";
    ss <<              get_apple_ios_config();
    ss << "        endif()\n";
    ss << "    elseif(ANDROID)\n";
    ss << "    else()\n";
    ss <<          get_linux_config();
    ss << "    endif()\n";
    ss << "elseif(MSVC)\n";
    ss <<      get_msvc_config();
    ss << "endif()\n";
    return ss.str();
}

std::string project::get_cmake_file()
{
    std::string sepd = (base_path.find("\\") != std::string::npos) ? "\\" : "/";
    std::string path = output_path + sepd + "CMakeLists.txt";
    return path;
}

void project::gen_cmake()
{
    std::ofstream outfile (get_cmake_file(), std::ofstream::binary);

    outfile << get_header();
    outfile << get_dependencies();
    outfile << get_cpp_standard();
    outfile << get_defines();
    outfile << get_include_dirs();
    outfile << get_autogen_vars();
    outfile << get_resource_files();
    outfile << get_source_list();
    //outfile << get_source_groups();
    outfile << get_executable();
    outfile << get_common_options();
    outfile << get_target_config();

    outfile.close();
}

void project::print()
{
    std::stringstream ss;
    ss << "id = " << id << "\n";
    ss << "name = " << name << "\n";
    ss << "displaySplashScreen = " << displaySplashScreen << "\n";
    ss << "reportAppUsage = " << reportAppUsage << "\n";
    ss << "splashScreenColour = " << splashScreenColour << "\n";
    ss << "projectType = " << projectType << "\n";
    ss << "version = " << version << "\n";
    ss << "bundleIdentifier = " << bundleIdentifier << "\n";
    ss << "includeBinaryInAppConfig = " << includeBinaryInAppConfig << "\n";
    ss << "cppLanguageStandard = " << cppLanguageStandard << "\n";
    ss << "companyCopyright = " << companyCopyright << "\n";
    ss << "buildVST = " << buildVST << "\n";
    ss << "buildVST3 = " << buildVST3 << "\n";
    ss << "buildAU = " << buildAU << "\n";
    ss << "buildAUv3 = " << buildAUv3 << "\n";
    ss << "buildRTAS = " << buildRTAS << "\n";
    ss << "buildAAX = " << buildAAX << "\n";
    ss << "buildStandalone = " << buildStandalone << "\n";
    ss << "buildUnity = " << buildUnity << "\n";
    ss << "enableIAA = " << enableIAA << "\n";
    ss << "pluginName = " << pluginName << "\n";
    ss << "pluginDesc = " << pluginDesc << "\n";
    ss << "pluginManufacturer = " << pluginManufacturer << "\n";
    ss << "pluginManufacturerCode = " << pluginManufacturerCode << "\n";
    ss << "pluginCode = " << pluginCode << "\n";
    ss << "pluginChannelConfigs = " << pluginChannelConfigs << "\n";
    ss << "pluginIsSynth = " << pluginIsSynth << "\n";
    ss << "pluginWantsMidiIn = " << pluginWantsMidiIn << "\n";
    ss << "pluginProducesMidiOut = " << pluginProducesMidiOut << "\n";
    ss << "pluginIsMidiEffectPlugin = " << pluginIsMidiEffectPlugin << "\n";
    ss << "pluginEditorRequiresKeys = " << pluginEditorRequiresKeys << "\n";
    ss << "pluginAUExportPrefix = " << pluginAUExportPrefix << "\n";
    ss << "aaxIdentifier = " << aaxIdentifier << "\n";
    ss << "pluginAAXCategory = " << pluginAAXCategory << "\n";
    ss << "jucerVersion = " << jucerVersion << "\n";
    ss << "companyName = " << companyName << "\n";
    ss << "headerPath = \n";

    if(headerPath.size())
    {
        for (auto & path : headerPath) {
            ss << "    " << path << "\n";
        }
    }
    ss << "companyWebsite = " << companyWebsite << "\n";
    ss << "defines = " << defines << "\n";
    ss << "pluginFormats = " << pluginFormats << "\n";
    ss << "pluginCharacteristicsValue = " << pluginCharacteristicsValue << "\n";
    ss << "userNotes = " << userNotes << "\n";
    std::cout << ss.str() << std::endl;
}

/* These are used when module directories are not present */
const project::map_t project::OSXFramework = {
    { "juce_audio_basics", "Accelerate" },
    { "juce_audio_devices", "CoreAudio CoreMIDI AudioToolbox" },
    { "juce_audio_formats", "CoreAudio CoreMIDI QuartzCore AudioToolbox" },
    { "juce_audio_processors", "CoreAudio CoreMIDI AudioToolbox" },
    { "juce_audio_utils", "CoreAudioKit DiscRecording" },
    { "juce_core", "Cocoa IOKit" },
    { "juce_dsp", "Accelerate" },
    { "juce_graphics", "Cocoa QuartzCore" },
    { "juce_gui_basics", "Cocoa Carbon QuartzCore" },
    { "juce_gui_extra", "WebKit" },
    { "juce_opengl", "OpenGL" },
    { "juce_video", "AVKit AVFoundation CoreMedia" },
};

const project::map_t project::iOSFrameworks = {
    { "juce_audio_basics", "Accelerate" },
    { "juce_audio_devices", "CoreAudio CoreMIDI AudioToolbox AVFoundation" },
    { "juce_audio_formats", "AudioToolbox QuartzCore" },
    { "juce_audio_processors", "AudioToolbox" },
    { "juce_audio_utils", "CoreAudioKit" },
    { "juce_core", "Foundation" },
    { "juce_dsp", "Accelerate" },
    { "juce_graphics", "CoreGraphics CoreImage CoreText QuartzCore" },
    { "juce_gui_basics", "UIKit MobileCoreServices" },
    { "juce_opengl", "OpenGLES" },
    { "juce_video", "AVKit AVFoundation CoreMedia" },
};

const project::map_t project::linuxPackages = {
    { "juce_audio_devices", "alsa" },
    { "juce_graphics", "x11 xinerama xext freetype2" },
    { "juce_gui_basics", "x11 xinerama xext freetype2" },
};

const project::map_t project::linuxLibs = {
    { "juce_core", "curl rt dl pthread" },
    { "juce_opengl", "GL" },
};

const project::map_t project::mingwLibs = {
    { "juce_audio_devices", "winmm" },
    { "juce_core", "uuid wsock32 wininet version ole32 ws2_32 oleaut32 imm32 comdlg32 shlwapi rpcrt4 winmm" },
    { "juce_opengl", "opengl32" },
};

