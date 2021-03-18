#include <microstl.h>
#include <iostream>
#include <vector>

struct ConsoleReceiver : microstl::IHandler
{
	void onAscii() override
	{
		std::cout << "ASCII mode enabled!" << std::endl;
	}

	void onBinary() override
	{
		std::cout << "Binary mode enabled!" << std::endl;
	}

	void onName(const std::string& name) override
	{
		std::cout << "Name: " << name << std::endl;
	}

	void onTriangleCount(uint32_t triangles) override
	{
		std::cout << "Triangles: " << triangles << std::endl;
	}

	void onFacet(const float v1[3], const float v2[3], const float v3[3], const float n[3]) override
	{
		std::cout << "Face: " << v1[0] << "|" << v1[1] << "|" << v1[2] << ", "
			<< v2[0] << "|" << v2[1] << "|" << v2[2] << ", "
			<< v3[0] << "|" << v3[1] << "|" << v3[2] << ", "
			<< n[0] << "|" << n[1] << "|" << n[2] << std::endl;
	}
};

struct SimpleReceiver : microstl::IHandler
{
	struct Facet
	{
		std::vector<float> normal;
		std::vector<std::vector<float>> vertices;
	};

	std::string name;
	std::vector<Facet> facets;

	void onName(const std::string& name) override
	{
		this->name = name;
	}

	void onFacet(const float v1[3], const float v2[3], const float v3[3], const float n[3]) override
	{
		Facet f;
		f.normal = { n[0], n[1], n[2] };
		f.vertices.push_back(std::vector<float>{v1[0], v1[1], v1[2]});
		f.vertices.push_back(std::vector<float>{v2[0], v2[1], v2[2]});
		f.vertices.push_back(std::vector<float>{v3[0], v3[1], v3[2]});
		facets.push_back(std::move(f));
	}
};

int main()
{
	std::filesystem::path simplePath = "../../../testdata/simple_ascii.stl";
	std::filesystem::path halfDonutPath = "../../../testdata/half_donut_ascii.stl";
	std::filesystem::path binaryPath = "../../../testdata/stencil_binary.stl";
	
	microstl::Result result1 = microstl::parseStlFile(simplePath, ConsoleReceiver());
	std::cout << "Result 1: " << uint16_t(result1) << std::endl;

	SimpleReceiver mesh2;
	microstl::Result result2 = microstl::parseStlFile(halfDonutPath, mesh2);
	std::cout << "Result 2: " << uint16_t(result2) << std::endl;

	SimpleReceiver mesh3;
	microstl::Result result3 = microstl::parseStlFile(binaryPath, mesh3);
	std::cout << "Result 3: " << uint16_t(result3) << std::endl;

	return 0;
}
