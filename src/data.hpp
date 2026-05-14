#pragma once

#include <LittleFS.h>

#include <vector>
#include <string>

using namespace std;

void initFS()
{
    LittleFS.begin(true);
}

vector<string> readDir(const string &path)
{
    vector<string> out;

    File dir = LittleFS.open(path.c_str());
    if (!dir || !dir.isDirectory())
        return out;

    File file = dir.openNextFile();
    while (file)
    {
        out.push_back(string(file.name()));
        file = dir.openNextFile();
    }

    return out;
}