#include "data.hpp"

void initFS()
{
    LittleFS.begin(true);
}

std::vector<std::string> readDir(const std::string &path)
{
    std::vector<std::string> out;

    File dir = LittleFS.open(path.c_str());
    if (!dir || !dir.isDirectory())
        return out;

    File file = dir.openNextFile();
    while (file)
    {
        out.push_back(std::string(file.name()));
        file = dir.openNextFile();
    }

    return out;
}

Article readArticleBinary(const std::string &path)
{
    Article article;

    File f = LittleFS.open(path.c_str(), "r");
    if (!f)
        return article;

    while (f.available())
    {
        uint8_t type;
        uint16_t len;

        if (f.read((uint8_t *)&type, 1) != 1)
            break;
        if (f.read((uint8_t *)&len, 2) != 2)
            break;

        String text = "";
        text.reserve(len);

        for (int i = 0; i < len; i++)
        {
            if (!f.available())
                break;
            text += (char)f.read();
        }

        Item it;
        it.typ = (Type)type;
        it.daten.push_back(text);

        article.push_back(it);
    }

    f.close();
    return article;
}
