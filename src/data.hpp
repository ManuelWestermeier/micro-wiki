#pragma once

#include <LittleFS.h>
#include <vector>
#include <string>

#include "article-renderer.hpp"

void initFS();
std::vector<std::string> readDir(const std::string &path);
Article readArticleBinary(const std::string &path);
