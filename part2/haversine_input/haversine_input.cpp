#include <cmath>
#include <cstdio>
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

const int numberOfRegions = 64;

struct JsonFile
{
    std::string path;
    std::ofstream file;
    std::shared_ptr<std::vector<Pair> > pairs = std::make_shared<std::vector<Pair> >();

    JsonFile(std::string path) : path(path)
    {}

    void AddRandom(int number)
    {
        int numberOf = 0;
        for (int i = numberOfRegions; i > 0; i--)
        {
            int lonRange = 360;
            int latRange = 180;
            for (int j = 0; j < number / numberOfRegions; j++)
            {
                int x0 = (rand() % 361 - 180) / i;
                int x1 = (rand() % 361 - 180) / i;
                int y0 = (rand() % 181 - 90) / i;
                int y1 = (rand() % 181 - 90) / i;
                pairs->push_back({x0, y0, x1, y1});
                numberOf++;
            }

        }

        printf("%d\n", numberOf);
    }

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
            file << "        { \"x0\": " << pairs->at(i).x0 << ", \"y0\": " << pairs->at(i).y0
                 << ", \"x1\": " << pairs->at(i).x1 << ", \"y1\": " << pairs->at(i).y1 << " }";

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

    jsonFile.AddRandom(1000);
    jsonFile.Write();

    return 0;
}
