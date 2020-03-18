#include <string>
#include <ostream>
#include <vector>
#include <list>
#include <pugixml.hpp>

class project
{
public:

    struct buildConfig
    {
        bool valid;
        std::string name;
        bool isDebug;
        bool optimisation;
        std::string targetName;
        std::vector<std::string> headerPath;
        std::vector<std::string> libraryPath;
    };

    struct buildExport
    {
        bool valid;
        std::string targetFolder;
        std::vector<std::string> extraDefs;
        std::vector<std::string> externalLibraries;
        std::string cppLanguageStandard;
        std::string extraCompilerFlags;
        std::string extraLinkerFlags;

        std::string extraFrameworks;
        std::string smallIcon;
        std::string bigIcon;
        std::string vstLegacyFolder;
        std::string vst3Folder;
        bool microphonePermissionNeeded;

        struct buildConfig debug;
        struct buildConfig release;
    };
    
    project();
    project(std::string file, std::string outpath);
    void print();

    void gen_cmake();
    std::string get_cmake_file();

private:

    pugi::xml_document m_Doc;

    std::vector<std::string> split(const std::string& s, char delimiter);
    bool downloadJuceSource;
    std::string base_path;
    std::string output_path;
   
    std::string id;
    std::string name;
    bool displaySplashScreen;
    bool reportAppUsage;
    std::string projectType;
    std::string version;
    std::string splashScreenColour;
    std::string bundleIdentifier;
    bool includeBinaryInAppConfig;
    std::string cppLanguageStandard;
    std::string companyCopyright;
    std::string juceFolder;
    bool buildVST;
    bool buildVST3;
    bool buildAU;
    bool buildAUv3;
    bool buildRTAS;
    bool buildAAX;
    bool buildStandalone;
    bool buildUnity;
    bool enableIAA;
    std::string pluginName;
    std::string pluginDesc;
    std::string pluginManufacturer;
    std::string pluginManufacturerCode;
    std::string pluginCode;
    std::string pluginChannelConfigs;
    bool pluginIsSynth;
    bool pluginWantsMidiIn;
    bool pluginProducesMidiOut;
    bool pluginIsMidiEffectPlugin;
    bool pluginEditorRequiresKeys;
    std::string pluginAUExportPrefix;
    std::string aaxIdentifier;
    int pluginAAXCategory;
    std::string jucerVersion;
    std::string companyName;
    std::vector<std::string> headerPath;
    std::string companyWebsite;
    std::string defines;
    std::string userNotes;
    std::string pluginFormats;
    std::string pluginCharacteristicsValue;
    
    std::string get_resource_files();

    std::string get_header();

    std::string get_dependencies();

    std::string get_modules();

    std::string get_resource_list();
    std::string get_source_groups();
    std::string get_source_list();

    std::vector<std::string> get_export_formats();
    std::list<std::string> get_module_path_list();
    std::string get_module_paths();

    std::string get_autogen_vars();

    std::string get_include_dirs();
    std::string get_defines();
    std::string get_cpp_standard();
    std::string get_executable();
    std::string get_common_options();
    std::string get_target_config();
    std::string get_apple_config();
    std::string get_linux_config();
    std::string get_msvc_config();

    void get_export(std::string target, project::buildExport &build);
};
