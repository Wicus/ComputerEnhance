#include <fstream>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>

struct Pair
{
    int x0;
    int y0;
    int x1;
    int y1;
};

struct JsonFile
{
    std::string path;
    std::ofstream file;
    std::shared_ptr<std::vector<Pair> > pairs = std::make_shared<std::vector<Pair> >();

    JsonFile(std::string path) : path(path)
    {}

    void Write()
    {
        file.open(path);

        if (!file.is_open())
        {
            throw std::runtime_error("File is not open, couldn't not write the file");
        }

        file << "{\n";
        file << "    \"pairs\": [\n";

        for (size_t i = 0; i < pairs->size(); i++)
        {
            file << "        {\n";
            file << "            \"x0\": " << pairs->at(i).x0 << std::endl;
            file << "            \"y0\": " << pairs->at(i).y0 << std::endl;
            file << "            \"x1\": " << pairs->at(i).x1 << std::endl;
            file << "            \"y1\": " << pairs->at(i).y1 << std::endl;
            file << "        }";

            if (i < pairs->size() - 1)
            {
                file << ",\n";
            }
            else
            {
                file << "\n";
            }
        }

        file << "    ]\n";
        file << "}";
    }
};

int main(int argc, char *argv[])
{
    JsonFile jsonFile("haversine_input.json");

    jsonFile.pairs->push_back({0, 0, 1, 1});
    jsonFile.pairs->push_back({1, 1, 2, 2});

    jsonFile.Write();

    return 0;
}
