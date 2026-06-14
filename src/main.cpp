#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <filesystem>
#include <algorithm>
#include <vector>

#include "interpreter/Interpreter.hpp"
#include "lexer/Lexer.hpp"
#include "parser/Parser.hpp"

namespace
{
    std::string readSource(
        int argc,
        char* argv[]
    )
    {
        if(
            argc >= 3 &&
            std::string(argv[1]) == "-e"
        )
        {
            return argv[2];
        }

        if(argc >= 2)
        {
            std::ifstream input(argv[1]);

            if(!input)
            {
                throw std::runtime_error(
                    "Unable to open source file: " +
                    std::string(argv[1])
                );
            }

            std::ostringstream buffer;
            buffer << input.rdbuf();
            return buffer.str();
        }

        std::ostringstream buffer;
        buffer << std::cin.rdbuf();
        return buffer.str();
    }

    std::string trimTrailing(const std::string& str)
    {
        std::string s = str;
        s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
            return !std::isspace(ch);
        }).base(), s.end());
        return s;
    }

    void runTests(const std::string& dirPath)
    {
        namespace fs = std::filesystem;
        std::vector<fs::path> jsFiles;
        for(const auto& entry : fs::directory_iterator(dirPath))
        {
            if(entry.is_regular_file() && entry.path().extension() == ".js")
            {
                jsFiles.push_back(entry.path());
            }
        }
        
        std::sort(jsFiles.begin(), jsFiles.end());
        
        int passed = 0;
        int failed = 0;
        std::vector<std::string> failedTests;
        
        for(const auto& jsPath : jsFiles)
        {
            std::string testName = jsPath.stem().string();
            fs::path expectedPath = jsPath;
            expectedPath.replace_extension(".expected");
            
            if(!fs::exists(expectedPath))
            {
                std::cout << "SKIP " << testName << " (no .expected file)\n";
                continue;
            }
            
            // Read expected
            std::ifstream expFile(expectedPath);
            std::string expectedContent((std::istreambuf_iterator<char>(expFile)), std::istreambuf_iterator<char>());
            expectedContent = trimTrailing(expectedContent);
            
            // Read source
            std::ifstream srcFile(jsPath);
            std::string sourceContent((std::istreambuf_iterator<char>(srcFile)), std::istreambuf_iterator<char>());
            
            // Redirect stdout
            std::streambuf* oldCoutBuf = std::cout.rdbuf();
            std::stringstream ss;
            std::cout.rdbuf(ss.rdbuf());
            
            bool success = true;
            std::string errorMsg;
            try
            {
                Lexer lexer(sourceContent);
                auto tokens = lexer.tokenize();
                
                Parser parser(tokens);
                auto program = parser.parseProgram();
                
                Interpreter interpreter;
                interpreter.execute(program.get());
            }
            catch(const std::exception& e)
            {
                success = false;
                errorMsg = e.what();
            }
            
            // Restore stdout
            std::cout.rdbuf(oldCoutBuf);
            
            std::string actualContent = trimTrailing(ss.str());
            if(!success)
            {
                actualContent += " (Error: " + errorMsg + ")";
            }
            
            if(success && actualContent == expectedContent)
            {
                std::cout << "PASS " << testName << "\n";
                passed++;
            }
            else
            {
                std::cout << "FAIL " << testName << "\n";
                std::cout << "  Expected: " << expectedContent << "\n";
                std::cout << "  Actual:   " << actualContent << "\n";
                failed++;
                failedTests.push_back(testName);
            }
        }
        
        std::cout << "\nResults: " << passed << " passed, " << failed << " failed, out of " << (passed + failed) << " total\n";
        if(!failedTests.empty())
        {
            std::cout << "Failed tests:\n";
            for(const auto& name : failedTests)
            {
                std::cout << "  - " << name << "\n";
            }
            std::exit(1);
        }
        std::exit(0);
    }
}

int main(
    int argc,
    char* argv[]
)
{
    try
    {
        if (argc >= 2 && std::filesystem::is_directory(argv[1]))
        {
            runTests(argv[1]);
            return 0;
        }

        std::string source =
            readSource(argc, argv);

        Lexer lexer(source);
        auto tokens = lexer.tokenize();

        Parser parser(tokens);
        auto program = parser.parseProgram();

        Interpreter interpreter;
        interpreter.execute(program.get());

        return 0;
    }
    catch(const std::exception& error)
    {
        std::cerr
            << error.what()
            << '\n';

        return 1;
    }
}
