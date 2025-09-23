#include <cmath>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>

// Prototypes
static double Square(double A);
static double RadiansFromDegrees(double Degrees);
static double ReferenceHaversine(double X0, double Y0, double X1, double Y1, double EarthRadius);

enum class DistributionType
{
    Uniform,
    Cluster
};
struct Pair
{
    double x0;
    double y0;
    double x1;
    double y1;
};

// Globals
DistributionType distributionType = DistributionType::Cluster;
int seed = 1000;
int numPairs = 1000;

struct JsonFile
{
    std::string name;
    std::shared_ptr<std::vector<Pair> > pairs = std::make_shared<std::vector<Pair> >();

    JsonFile(std::string name) : name(name)
    {}

    void AddRandom()
    {
        int index = 0;
        for (int i = 0; i < numPairs / 2; i++)
        {
            double x0 = rand() % 361 - 270;
            double x1 = rand() % 361 - 270;
            double y0 = rand() % 181 - 135;
            double y1 = rand() % 181 - 135;
            pairs->push_back({x0, y0, x1, y1});
            index++;
        }

        for (int i = index; i < numPairs; i++)
        {
            double x0 = rand() % 361 - 180;
            double x1 = rand() % 361 - 180;
            double y0 = rand() % 181 - 90;
            double y1 = rand() % 181 - 90;
            pairs->push_back({x0, y0, x1, y1});
            index++;
        }

        printf("Number of samples: %d\n", index);
    }

    void Write()
    {
        std::string jsonPath = name + ".json";
        std::ofstream file;
        file.open(jsonPath);

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

    void WriteReferenceHaversine()
    {
        std::string filePath = name + ".txt";
        std::ofstream file;
        file.open(filePath);

        if (!file.is_open())
        {
            throw std::runtime_error("File is not open, couldn't not write the file");
        }

        double haversineSum = 0.0;

        file << "Reference Haversine\n";

        for (size_t i = 0; i < pairs->size(); i++)
        {
            double x0 = pairs->at(i).x0;
            double x1 = pairs->at(i).x1;
            double y0 = pairs->at(i).y0;
            double y1 = pairs->at(i).y1;
            double referenceHaversine = ReferenceHaversine(x0, y0, x1, y1, 6372.8);

            haversineSum += referenceHaversine;

            file << "lon1: " << x0;
            file << ", lon2: " << x1;
            file << ", lat1: " << y0;
            file << ", lat2: " << y1;
            file << ", distance: " << referenceHaversine;
            file << std::endl;
        }

        printf("Expected sum: %f\n", (haversineSum / pairs->size()));

        file << "\n";
    }
};

int main(int argc, char *argv[])
{
    if (argc > 1 && (std::string(argv[1]) == "-h" || std::string(argv[1]) == "--help"))
    {
        std::cout << "Usage: program [uniform/cluster] [random seed] [number of coordinate pairs]\n";
        return 0;
    }

    if (argc > 1)
    {
        std::string type = argv[1];
        if (type != "uniform" && type != "cluster")
        {
            std::cout << "Usage: program [uniform/cluster] [random seed] [number of coordinate pairs]\n";
            return 0;
        }

        if (type == "cluster")
        {
            distributionType = DistributionType::Cluster;
        }
    }

    if (argc > 2)
    {
        seed = std::atoi(argv[2]);
    }
    srand(seed);

    if (argc > 3)
    {
        numPairs = std::atoi(argv[3]);
    }

    JsonFile jsonFile("haversine_input");

    jsonFile.AddRandom();
    jsonFile.Write();
    jsonFile.WriteReferenceHaversine();

    return 0;
}

static double Square(double A)
{
    double Result = (A * A);
    return Result;
}

static double RadiansFromDegrees(double Degrees)
{
    double Result = 0.01745329251994329577f * Degrees;
    return Result;
}

// NOTE(casey): EarthRadius is generally expected to be 6372.8
static double ReferenceHaversine(double X0, double Y0, double X1, double Y1, double EarthRadius)
{
    double lat1 = Y0;
    double lat2 = Y1;
    double lon1 = X0;
    double lon2 = X1;

    double dLat = RadiansFromDegrees(lat2 - lat1);
    double dLon = RadiansFromDegrees(lon2 - lon1);
    lat1 = RadiansFromDegrees(lat1);
    lat2 = RadiansFromDegrees(lat2);

    double a = Square(sin(dLat / 2.0)) + cos(lat1) * cos(lat2) * Square(sin(dLon / 2));
    double c = 2.0 * asin(sqrt(a));

    double Result = EarthRadius * c;

    return Result;
}
