#include <microstl.h>
#include <iostream>
#include <vector>

struct ConsoleReceiver : microstl::IReceiver
{
	void receiveName(const std::string& name) override
	{
		std::cout << "Name: " << name << std::endl;
	}

	void receiveFace(const double v1[3], const double v2[3], const double v3[3], const double n[3]) override
	{
		std::cout << "Face: " << v1[0] << "|" << v1[1] << "|" << v1[2] << ", "
			<< v2[0] << "|" << v2[1] << "|" << v2[2] << ", "
			<< v3[0] << "|" << v3[1] << "|" << v3[2] << ", "
			<< n[0] << "|" << n[1] << "|" << n[2] << std::endl;
	}
};

struct SimpleReceiver : microstl::IReceiver
{
	struct Facet
	{
		std::vector<double> normal;
		std::vector<std::vector<double>> vertices;
	};

	std::string name;
	std::vector<Facet> facets;

	void receiveName(const std::string& name) override
	{
		this->name = name;
	}

	void receiveFace(const double v1[3], const double v2[3], const double v3[3], const double n[3]) override
	{
		Facet f;
		f.normal = { n[0], n[1], n[2] };
		f.vertices.push_back(std::vector<double>{v1[0], v1[1], v1[2]});
		f.vertices.push_back(std::vector<double>{v2[0], v2[1], v2[2]});
		f.vertices.push_back(std::vector<double>{v3[0], v3[1], v3[2]});
		facets.push_back(std::move(f));
	}
};

int main()
{
	std::filesystem::path simpleAsciiPath = "../../../testdata/simple_ascii.stl";
	std::filesystem::path halfDonutPath = "../../../testdata/half_donut_ascii.stl";
	
	microstl::Result result1 = microstl::parseStlFile(simpleAsciiPath, ConsoleReceiver());
	std::cout << "Result 1: " << uint16_t(result1) << std::endl;

	SimpleReceiver mesh;
	microstl::Result result2 = microstl::parseStlFile(halfDonutPath, mesh);
	std::cout << "Result 2: " << uint16_t(result2) << std::endl;

	return 0;
}
