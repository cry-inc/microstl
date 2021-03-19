#include <microstl.h>
#include <iostream>
#include <vector>

struct ConsoleHandler : microstl::Parser::Handler
{
	void onAscii() override
	{
		std::cout << "ASCII mode enabled!" << std::endl;
	}

	void onBinary() override
	{
		std::cout << "Binary mode enabled!" << std::endl;
	}

	void onBinaryHeader(const uint8_t header[80]) override
	{
		std::cout << "Header: ";
		for (size_t i = 0; i < 80; i++)
			std::cout << int(header[i]) << ' ';
		std::cout << std::endl;
	}

	void onName(const std::string& name) override
	{
		std::cout << "Name: " << name << std::endl;
	}

	void onFacetCount(uint32_t triangles) override
	{
		std::cout << "Triangles: " << triangles << std::endl;
	}

	void onError(size_t lineNumber) override
	{
		std::cout << "Error detected on line: " << lineNumber << std::endl;
	}

	void onFacet(const float v1[3], const float v2[3], const float v3[3], const float n[3]) override
	{
		std::cout << "Facet: " << n[0] << "|" << n[1] << "|" << n[2] << ", "
			<< v1[0] << "|" << v1[1] << "|" << v1[2] << ", "
			<< v2[0] << "|" << v2[1] << "|" << v2[2] << ", "
			<< v3[0] << "|" << v3[1] << "|" << v3[2] << std::endl;
	}

	void onFacetAttribute(const uint8_t attribute[2]) override
	{
		std::cout << "Attribute: " << int(attribute[0]) << ' ' << int(attribute[1]) << std::endl;
	}
};

struct MemoryMeshHandler : microstl::Parser::Handler
{
	struct Facet
	{
		std::vector<float> normalVector;
		std::vector<std::vector<float>> vertices;
	};

	std::string stlName;
	bool ascii = false;
	size_t errorLineNumber = 0;
	std::vector<Facet> facets;

	void onName(const std::string& name) override { stlName = name; }
	void onAscii() override { ascii = true; }
	void onBinary() override { ascii = false; }
	void onError(size_t lineNumber) override { errorLineNumber = lineNumber; }

	void onFacet(const float v1[3], const float v2[3], const float v3[3], const float n[3]) override
	{
		Facet f;
		f.normalVector = { n[0], n[1], n[2] };
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
	
	microstl::Parser::Result result1 = microstl::Parser::parseStlFile(simplePath, ConsoleHandler());
	std::cout << "Result 1: " << uint16_t(result1) << std::endl;

	MemoryMeshHandler mesh2;
	microstl::Parser::Result result2 = microstl::Parser::parseStlFile(halfDonutPath, mesh2);
	std::cout << "Result 2: " << uint16_t(result2) << std::endl;

	MemoryMeshHandler mesh3;
	microstl::Parser::Result result3 = microstl::Parser::parseStlFile(binaryPath, mesh3);
	std::cout << "Result 3: " << uint16_t(result3) << std::endl;

	return 0;
}
