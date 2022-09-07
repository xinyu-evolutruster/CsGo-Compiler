#ifndef PPMACRO_H
#define PPMACRO_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
/*
 * preprocess the macros in the input code.
 * input:
 *      std::string filename : the filename of original code
 * return: 
 *      std::string : the filename of intermediate code
 */
std::string preProcess(std::string filename);

class macroTable
{
    friend std::string preProcess(std::string filename);

private:
    std::string Name;                 // the name of this macro
    bool withArgs;                    // whether it is a macro with arg(s)
    std::vector<std::string> srcArgs; // args of the macro
    std::vector<std::string> defs;    // splited destination code which is used for replacing
    /* 
     * judge whether there is a macro in this sentence 
     * if exists, return true; 
     * else false.
     */
    bool match(std::string token)
    {
        if (token.find(this->Name) != token.npos)
            return true;
        else
            return false;
    }
    /*
     * replace the macro in input
     * input :
     *      std::string input : source code
     * return :
     *      std::string : processed code 
     */
    std::string replace(std::string input)
    {
        int pos = input.find(this->Name);
        if (!withArgs)
        {
            // if there is no args, it is just a simple replace
            std::string head = input.substr(0, pos), tail = input.substr(pos + this->Name.length());
            return head + " " + this->defs[0] + " " + tail;
        }
        else
        {
            // if there is arg(s), we need to find out all args
            std::string head = input.substr(0, pos), cache = input.substr(pos);
            int rec = 1, delta = this->Name.length();
            do
            {
                delta++;
                if (cache[delta] == '(')
                    rec++;
                else if (cache[delta] == ')')
                    rec--;
            } while (rec != 0);
            std::string tail = cache.substr(delta + 1), body = cache.substr(this->Name.length() + 1, delta - this->Name.length());
            cache.clear();
            std::vector<std::string> args;
            rec = 1;
            // find all args
            for (int i = 0; i < body.length(); i++)
            {
                if (body[i] == ',' || body[i] == ' ' || (body[i] == ')' && rec == 1))
                {
                    if (cache.length() != 0)
                    {
                        args.push_back(cache);
                        cache.clear();
                    }
                }
                else
                {
                    if (body[i] == '(')
                        rec++;
                    else if (body[i] == ')')
                        rec--;
                    cache += body[i];
                }
            }
            // use the macro in wrong way
            if (args.size() != this->srcArgs.size())
            {
                perror("Wrong MACRO ARGS!");
            }
            std::vector<std::string> outputs(defs);
            for (int i = 0; i < this->srcArgs.size(); i++)
            {
                for (auto &output : outputs)
                {
                    if (this->srcArgs[i] == output)
                    //if this substring is corresponding to an arg, use it to replcae the arg in the intermediate code
                    {
                        output = args[i];
                    }
                }
            }
            cache.clear();
            for (auto output : outputs)
            {
                cache += output;
            }
            return head + " " + cache + " " + tail;
        }
    }

public:
    /* 
     * constructor
     * input :
     *      std::string firstPart : the name of the macro
     *      std::string secondPart : the destination code 
     */
    macroTable(std::string firstPart, std::string secondPart)
    {
        // judge if it is with arg(s)
        if (firstPart.find('(') == firstPart.npos)
        {
            this->Name = firstPart;
            this->defs.push_back(secondPart);
            withArgs = false;
        }
        else
        {
            withArgs = true;
            int pos = firstPart.find('(');
            std::string cache;
            this->Name = firstPart.substr(0, pos);
            cache.clear();
            // find all args' name
            for (int i = pos + 1; i < firstPart.length(); i++)
            {
                if (firstPart[i] == ',' || firstPart[i] == ' ' || firstPart[i] == ')')
                {
                    if (cache.length() != 0)
                    {
                        this->srcArgs.push_back(cache);
                        cache.clear();
                    }
                }
                else
                {
                    cache += firstPart[i];
                }
            }
            cache.clear();
            // split the destination code for replace
            for (int i = 0; i < secondPart.length(); i++)
            {
                if ((!isalnum(secondPart[i]) && secondPart[i] != '_') || i == (secondPart.length() - 1))
                {
                    if (cache.length() != 0)
                    {
                        this->defs.push_back(cache);
                        cache.clear();
                    }
                    std::string stack;
                    stack.push_back(secondPart[i]);
                    this->defs.push_back(stack);
                }
                else
                {
                    cache += secondPart[i];
                }
            }
        }
    }

    ~macroTable() {}
};

#endif
