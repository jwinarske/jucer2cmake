#include <iostream>
#include <sys/stat.h>

#include "project.h"


int main(int argc, char* argv[])
{
    std::vector<std::string> args(argv + 1, argv + argc);

    if (args.size() != 0)
    {
        bool verbose = false;
        std::string inputpath = "";
        std::string outputpath = "";

        int i = 0;
        while (i < args.size() && (args[i].rfind("-", 0) == 0))
        {
            std::string arg = args[i++];
            if (arg == "-v")
            {
                verbose = true;
            }
            else if (arg == "-i")
            {
                if (i < args.size())
                {
                    inputpath = args[i++];
                }
                else
                {
                    std::cerr << "-i requires a file path" << std::endl;
                }
            }
            else if (arg == "-o")
            {
                if (i < args.size())
                {
                    outputpath = args[i++];
                    break;
                }
                else
                {
                    std::cerr << "-o requires a path" << std::endl;
                }
            }
        }

        if(!inputpath.empty())
        {
            struct stat info;
            if( stat( inputpath.c_str(), &info ) == 0 )
            {
                std::cout << "Opening \"" << inputpath << "\"" << std::endl;
                auto proj = project(inputpath.c_str(), outputpath);
                proj.gen_cmake();
                if(verbose)
                {
                    proj.print();
                }
                std::cout << "Created \"" << proj.get_cmake_file() << "\"" << std::endl;
            }
            else
            {
                std::cerr << "In-Valid file: " << inputpath << std::endl;
            }
        }
    }
}
