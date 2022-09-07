#include "ppMacro.h"

std::string preProcess(std::string filename)
{
    std::ifstream fin(filename);
    // intermediate filename
    std::ofstream fout(filename + "pp");
    std::string fileContent;
    std::vector<macroTable> Macros;

    while (getline(fin, fileContent))
    {
        std::stringstream ss;
        std::string tmp;
        ss << fileContent;
        ss >> tmp;
        if (tmp.size() != 0)
        {
            if (tmp == "#define") //find macros
            {
                std::vector<std::string> lineTokens;
                while (ss >> tmp)
                    lineTokens.push_back(tmp);

                int rec = 0;
                std::string firstPart, secondPart;
                if (lineTokens[0].find('(') != lineTokens[0].npos)
                {
                    //if it is with args, it's name must end with a ')'
                    for (int i = 0;; i++)
                    {
                        firstPart += lineTokens[i] + " ";
                        if (lineTokens[i].find(')') != lineTokens[i].npos)
                        {
                            rec = i;
                            break;
                        }
                    }
                }
                else
                    firstPart = lineTokens[0];
                for (int i = rec + 1; i < lineTokens.size(); i++)
                {
                    secondPart += lineTokens[i];
                }
                // store the marco in macrotable
                Macros.push_back(macroTable(firstPart, secondPart));
            }
            else
            {
                // if this line is not a macro define, try to find macros in it and replace it.
                bool found = true;
                while (found)
                {
                    found = false;
                    for (auto macro : Macros)
                    {
                        while (macro.match(fileContent))
                        {
                            fileContent = macro.replace(fileContent);
                            found = true;
                        }
                    }
                }
                fout << fileContent << std::endl;
            }
        }
    }
    fin.close();

    return filename + "pp";
}