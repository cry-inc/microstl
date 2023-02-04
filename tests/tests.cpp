﻿#include <microstl.h>
#include <random>

#define TEST_SCOPE(x)
#define REQUIRE(x) {if (!(x)) throw std::runtime_error("Test assertion failed!"); }

std::filesystem::path findTestFile(std::string fileName)
{
	auto dir = std::filesystem::current_path();
	dir = std::filesystem::absolute(dir);
	while (std::filesystem::exists(dir))
	{
		auto filePath = dir / "testdata" / std::filesystem::u8path(fileName);
		if (std::filesystem::exists(filePath))
			return filePath;

		if (dir.has_parent_path() && dir.parent_path() != dir)
			dir = dir.parent_path();
		else
			throw std::runtime_error("Unable to find parent folder!");
	}

	throw std::runtime_error("Unable to find test file!");
}

int main()
{
	{
		TEST_SCOPE("Parse minimal ASCII STL file and check all results");
		microstl::MeshReaderHandler handler;
		auto res = microstl::Reader::readStlFile(findTestFile("simple_ascii.stl"), handler);
		REQUIRE(res == handler.result && res == microstl::Result::Success);
		REQUIRE(handler.name == "minimal");
		REQUIRE(handler.ascii);
		REQUIRE(handler.errorLineNumber == 0);
		REQUIRE(handler.header.empty());
		REQUIRE(handler.mesh.facets.size() == 1);
		REQUIRE(handler.mesh.facets[0].n.x == -1);
		REQUIRE(handler.mesh.facets[0].n.y == 0);
		REQUIRE(handler.mesh.facets[0].n.z == 0);
		REQUIRE(handler.mesh.facets[0].v1.x == 0);
		REQUIRE(handler.mesh.facets[0].v1.y == 0);
		REQUIRE(handler.mesh.facets[0].v1.z == 0);
		REQUIRE(handler.mesh.facets[0].v2.x == 0);
		REQUIRE(handler.mesh.facets[0].v2.y == 0);
		REQUIRE(handler.mesh.facets[0].v2.z == 1);
		REQUIRE(handler.mesh.facets[0].v3.x == 0);
		REQUIRE(handler.mesh.facets[0].v3.y == 1);
		REQUIRE(handler.mesh.facets[0].v3.z == 1);
	}

	{
		TEST_SCOPE("Parse ASCII STL file with creative white spaces and check all results");
		microstl::MeshReaderHandler handler;
		auto res = microstl::Reader::readStlFile(findTestFile("crazy_whitespace_ascii.stl"), handler);
		REQUIRE(res == handler.result && res == microstl::Result::Success);
		REQUIRE(handler.name == "min \t imal");
		REQUIRE(handler.ascii);
		REQUIRE(handler.errorLineNumber == 0);
		REQUIRE(handler.header.empty());
		REQUIRE(handler.mesh.facets.size() == 1);
		REQUIRE(handler.mesh.facets[0].n.x == -1);
		REQUIRE(handler.mesh.facets[0].n.y == 0);
		REQUIRE(handler.mesh.facets[0].n.z == 0);
		REQUIRE(handler.mesh.facets[0].v1.x == 0);
		REQUIRE(handler.mesh.facets[0].v1.y == 0);
		REQUIRE(handler.mesh.facets[0].v1.z == 0);
		REQUIRE(handler.mesh.facets[0].v2.x == 0);
		REQUIRE(handler.mesh.facets[0].v2.y == 0);
		REQUIRE(handler.mesh.facets[0].v2.z == 1);
		REQUIRE(handler.mesh.facets[0].v3.x == 0);
		REQUIRE(handler.mesh.facets[0].v3.y == 1);
		REQUIRE(handler.mesh.facets[0].v3.z == 1);
	}

	{
		TEST_SCOPE("Parse small ASCII STL file and do a simple check on the results");
		microstl::MeshReaderHandler handler;
		auto res = microstl::Reader::readStlFile(findTestFile("half_donut_ascii.stl"), handler);
		REQUIRE(res == handler.result && res == microstl::Result::Success);
		REQUIRE(handler.name == "Half Donut");
		REQUIRE(handler.ascii);
		REQUIRE(handler.errorLineNumber == 0);
		REQUIRE(handler.header.empty());
		REQUIRE(handler.mesh.facets.size() == 288);
	}

	{
		TEST_SCOPE("Parse binary STL file and do a simple check on the results");
		microstl::MeshReaderHandler handler;
		auto res = microstl::Reader::readStlFile(findTestFile("stencil_binary.stl"), handler);
		REQUIRE(res == handler.result && res == microstl::Result::Success);
		REQUIRE(handler.name.empty());
		REQUIRE(!handler.ascii);
		REQUIRE(handler.errorLineNumber == 0);
		REQUIRE(handler.header.size() == 80);
		for (size_t i = 0; i < 80; i++)
			REQUIRE(handler.header[i] == 0);
		REQUIRE(handler.mesh.facets.size() == 2330);
	}

	{
		TEST_SCOPE("Parse binary STL file from FreeCAD and do some checks on the results");
		microstl::MeshReaderHandler handler;
		auto res = microstl::Reader::readStlFile(findTestFile("box_freecad_binary.stl"), handler);
		REQUIRE(res == handler.result && res == microstl::Result::Success);
		REQUIRE(handler.name.empty());
		REQUIRE(!handler.ascii);
		REQUIRE(handler.errorLineNumber == 0);
		REQUIRE(handler.header.size() == 80);
		REQUIRE(handler.mesh.facets.size() == 12);
		REQUIRE(handler.mesh.facets[11].n.x == 0);
		REQUIRE(handler.mesh.facets[11].n.y == 0);
		REQUIRE(handler.mesh.facets[11].n.z == 1);
		REQUIRE(handler.mesh.facets[11].v1.x == 20);
		REQUIRE(handler.mesh.facets[11].v1.y == 0);
		REQUIRE(handler.mesh.facets[11].v1.z == 20);
		REQUIRE(handler.mesh.facets[11].v2.x == 0);
		REQUIRE(handler.mesh.facets[11].v2.y == 0);
		REQUIRE(handler.mesh.facets[11].v2.z == 20);
		REQUIRE(handler.mesh.facets[11].v3.x == 20);
		REQUIRE(handler.mesh.facets[11].v3.y == -20);
		REQUIRE(handler.mesh.facets[11].v3.z == 20);
	}

	{
		TEST_SCOPE("Parse ASCII STL file from MeshLab and do some checks on the results");
		microstl::MeshReaderHandler handler;
		auto res = microstl::Reader::readStlFile(findTestFile("box_meshlab_ascii.stl"), handler);
		REQUIRE(res == handler.result && res == microstl::Result::Success);
		REQUIRE(handler.name == "STL generated by MeshLab");
		REQUIRE(handler.ascii);
		REQUIRE(handler.errorLineNumber == 0);
		REQUIRE(handler.header.empty());
		REQUIRE(handler.mesh.facets.size() == 12);
		REQUIRE(handler.mesh.facets[11].n.x == 0);
		REQUIRE(handler.mesh.facets[11].n.y == 0);
		REQUIRE(handler.mesh.facets[11].n.z == 1);
		REQUIRE(handler.mesh.facets[11].v1.x == 20);
		REQUIRE(handler.mesh.facets[11].v1.y == 0);
		REQUIRE(handler.mesh.facets[11].v1.z == 20);
		REQUIRE(handler.mesh.facets[11].v2.x == 0);
		REQUIRE(handler.mesh.facets[11].v2.y == 0);
		REQUIRE(handler.mesh.facets[11].v2.z == 20);
		REQUIRE(handler.mesh.facets[11].v3.x == 20);
		REQUIRE(handler.mesh.facets[11].v3.y == -20);
		REQUIRE(handler.mesh.facets[11].v3.z == 20);
	}

	{
		TEST_SCOPE("Test parsing file paths supplied as UTF8 string");
		auto filePath = findTestFile(u8"简化字.stl");
		auto utf8String = filePath.u8string();
		microstl::MeshReaderHandler handler;
		auto res = microstl::Reader::readStlFile(utf8String.c_str(), handler);
		REQUIRE(res == handler.result && res == microstl::Result::Success);
		REQUIRE(handler.mesh.facets.size() == 1);
	}

	#ifdef _WIN32 // Wide strings are only common on Windows
	{
		TEST_SCOPE("Test parsing file paths supplied as wide string");
		auto filePath = findTestFile(u8"简化字.stl");
		auto wideString = filePath.wstring();
		microstl::MeshReaderHandler handler;
		auto res = microstl::Reader::readStlFile(wideString.c_str(), handler);
		REQUIRE(res == handler.result && res == microstl::Result::Success);
		REQUIRE(handler.mesh.facets.size() == 1);
	}
	#endif

	{
		TEST_SCOPE("Test parsing STL data supplied as memory buffer");
		std::ifstream ifs(findTestFile("simple_ascii.stl"), std::ios::binary | std::ios::ate);
		std::streamsize size = ifs.tellg();
		ifs.seekg(0, std::ios::beg);
		std::vector<char> buffer(size);
		ifs.read(buffer.data(), size);
		REQUIRE(size > 0);

		microstl::MeshReaderHandler handler;
		auto res = microstl::Reader::readStlBuffer(buffer.data(), buffer.size(), handler);
		REQUIRE(res == handler.result && res == microstl::Result::Success);
		REQUIRE(handler.mesh.facets.size() == 1);
	}

	{
		TEST_SCOPE("Test parsing STL data supplied as std::istream");
		std::ifstream ifs(findTestFile("simple_ascii.stl"), std::ios::binary);
		microstl::MeshReaderHandler handler;
		auto res = microstl::Reader::readStlStream(ifs, handler);
		REQUIRE(res == handler.result && res == microstl::Result::Success);
		REQUIRE(handler.mesh.facets.size() == 1);
	}

	{
		TEST_SCOPE("Parse STL with sphere and check all vertices");
		microstl::MeshReaderHandler handler;
		handler.forceNormals = true;
		auto res = microstl::Reader::readStlFile(findTestFile("sphere_binary.stl"), handler);
		REQUIRE(res == handler.result && res == microstl::Result::Success);
		REQUIRE(handler.mesh.facets.size() == 1360);
		const float radius = 10;
		const float allowedDeviation = 0.00001f;
		for (const auto& f : handler.mesh.facets)
		{
			// Check if all vertices are on the sphere surface
			float length1 = sqrt(f.v1.x * f.v1.x + f.v1.y * f.v1.y + f.v1.z * f.v1.z);
			REQUIRE(fabs(length1 - radius) < allowedDeviation);
			float length2 = sqrt(f.v2.x * f.v2.x + f.v2.y * f.v2.y + f.v2.z * f.v2.z);
			REQUIRE(fabs(length2 - radius) < allowedDeviation);
			float length3 = sqrt(f.v3.x * f.v3.x + f.v3.y * f.v3.y + f.v3.z * f.v3.z);
			REQUIRE(fabs(length3 - radius) < allowedDeviation);

			// Check if origin is "behind" the normal plane
			// (normal of all sphere surface triangle should point away from the origin)
			float origin[3] = { 0, 0, 0 };
			float tmp[3] = {origin[0] - f.v1.x, origin[1] - f.v1.y, origin[2] - f.v1.z};
			float dot = f.n.x * tmp[0] + f.n.y * tmp[1] + f.n.z * tmp[2];
			REQUIRE(dot < 0);

			// Check normal vector length
			float length = sqrt(f.n.x * f.n.x + f.n.y * f.n.y + f.n.z * f.n.z);
			REQUIRE(fabs(length - 1.0f) < allowedDeviation);
		}
	}

	{
		TEST_SCOPE("Test default handler implementation");
		struct MyHandler : microstl::Reader::Handler
		{
			size_t facetCount = 0;
			void onFacet(const float v1[3], const float v2[3], const float v3[3], const float n[3]) override
			{ facetCount++; }
		};
		MyHandler handler;
		auto res = microstl::Reader::readStlFile(findTestFile("simple_ascii.stl"), handler);
		REQUIRE(res == microstl::Result::Success);
		REQUIRE(handler.facetCount == 1);
	}

	{
		TEST_SCOPE("Test parsing an otherwise valid ASCII file that exceeds the line limit");
		microstl::MeshReaderHandler handler;
		auto res = microstl::Reader::readStlFile(findTestFile("exceed_ascii_line_limit.stl"), handler);
		REQUIRE(res == handler.result && res == microstl::Result::LineLimitError);
		REQUIRE(handler.name == "minimal");
		REQUIRE(handler.ascii);
		REQUIRE(handler.errorLineNumber == 3);
		REQUIRE(handler.header.empty());
		REQUIRE(handler.mesh.facets.empty());
	}

	{
		TEST_SCOPE("Test parsing an incomplete vertex in an ASCII file");
		microstl::MeshReaderHandler handler;
		auto res = microstl::Reader::readStlFile(findTestFile("incomplete_vertex_ascii.stl"), handler);
		REQUIRE(res == handler.result && res == microstl::Result::ParserError);
		REQUIRE(handler.name == "minimal");
		REQUIRE(handler.ascii);
		REQUIRE(handler.errorLineNumber == 6);
		REQUIRE(handler.header.empty());
		REQUIRE(handler.mesh.facets.empty());
	}

	{
		TEST_SCOPE("Test parsing an incomplete normale in an ASCII file");
		microstl::MeshReaderHandler handler;
		auto res = microstl::Reader::readStlFile(findTestFile("incomplete_normal_ascii.stl"), handler);
		REQUIRE(res == handler.result && res == microstl::Result::ParserError);
		REQUIRE(handler.name == "minimal");
		REQUIRE(handler.ascii);
		REQUIRE(handler.errorLineNumber == 2);
		REQUIRE(handler.header.empty());
		REQUIRE(handler.mesh.facets.empty());
	}

	{
		TEST_SCOPE("Test parsing an empty file and check for correct error");
		microstl::MeshReaderHandler handler;
		auto res = microstl::Reader::readStlFile(findTestFile("empty_file.stl"), handler);
		REQUIRE(res == handler.result && res == microstl::Result::MissingDataError);
		REQUIRE(handler.name.empty());
		REQUIRE(!handler.ascii);
		REQUIRE(handler.errorLineNumber == 0);
		REQUIRE(handler.header.empty());
		REQUIRE(handler.mesh.facets.empty());
	}

	{
		TEST_SCOPE("Test parsing an non-existing file and check for correct error");
		microstl::MeshReaderHandler handler;
		auto res = microstl::Reader::readStlFile("does_not_exist.stl", handler);
		REQUIRE(res == handler.result && res == microstl::Result::FileError);
		REQUIRE(handler.name.empty());
		REQUIRE(!handler.ascii);
		REQUIRE(handler.errorLineNumber == 0);
		REQUIRE(handler.header.empty());
		REQUIRE(handler.mesh.facets.empty());
	}

	{
		TEST_SCOPE("Parse STL with cube and deduplicate vertices");
		microstl::MeshReaderHandler handler;
		auto res = microstl::Reader::readStlFile(findTestFile("box_meshlab_ascii.stl"), handler);
		REQUIRE(res == handler.result && res == microstl::Result::Success);
		REQUIRE(handler.mesh.facets.size() == 12);
		microstl::FVMesh deduplicatedMesh = microstl::deduplicateVertices(handler.mesh);
		REQUIRE(deduplicatedMesh.facets.size() == 12);
		REQUIRE(deduplicatedMesh.vertices.size() == 8);
	}

	{
		TEST_SCOPE("Test incomplete binary STL file");
		microstl::MeshReaderHandler handler;
		auto res = microstl::Reader::readStlFile(findTestFile("incomplete_binary.stl"), handler);
		REQUIRE(res == handler.result && res == microstl::Result::MissingDataError);
		REQUIRE(!handler.ascii);
		REQUIRE(handler.header.size() == 80);
		REQUIRE(handler.mesh.facets.size() == 5);
	}

	{
		TEST_SCOPE("Primitive fuzzing with some selected data");
		struct setup {
			uint32_t seed;
			size_t size;
			microstl::Result expectedResult;
		};
		std::vector<setup> setups = {
			{12345, 12, microstl::Result::MissingDataError},
			{54321, 81, microstl::Result::MissingDataError},
			{67890, 84, microstl::Result::FacetCountError},
			{1, 99, microstl::Result::MissingDataError},
		};
		for (const auto& setup : setups)
		{
			std::vector<uint8_t> data(setup.size);
			std::mt19937 gen(setup.seed);
			for (size_t s = 0; s < setup.size; ++s)
				data[s] = gen.operator()();
			microstl::MeshReaderHandler handler;
			auto buffer = reinterpret_cast<const char*>(data.data());
			auto result = microstl::Reader::readStlBuffer(buffer, data.size(), handler);
			REQUIRE(handler.result == result);
			REQUIRE(result == setup.expectedResult);
		}
	}

	{
		TEST_SCOPE("Primitive fuzzing with some deterministic random data");
		for (uint32_t size = 84; size < 4096; ++size)
		{
			std::vector<uint8_t> data(size);
			std::mt19937 gen(size);
			for (size_t s = 0; s < size; ++s)
				data[s] = gen.operator()();
			microstl::MeshReaderHandler handler;
			auto buffer = reinterpret_cast<const char*>(data.data());
			auto result = microstl::Reader::readStlBuffer(buffer, data.size(), handler);
			REQUIRE(handler.result == result);
			REQUIRE(result != microstl::Result::Success);
		}
	}

	{
		TEST_SCOPE("Simple writer test");
		microstl::MeshReaderHandler handler;
		auto res = microstl::Reader::readStlFile(findTestFile("box_meshlab_ascii.stl"), handler);
		REQUIRE(res == handler.result && res == microstl::Result::Success);

		// Default mode is binary
		microstl::MeshProvider providerBinary(handler.mesh);
		std::filesystem::path path("tmp.stl");
		REQUIRE(!std::filesystem::exists(path));
		res = microstl::Writer::writeStlFile(path, providerBinary);
		REQUIRE(res == microstl::Result::Success);
		REQUIRE(std::filesystem::exists(path));
		std::filesystem::remove(path);

		// Optional ASCII mode
		microstl::MeshProvider providerAscii(handler.mesh);
		providerAscii.ascii = true;
		REQUIRE(!std::filesystem::exists(path));
		res = microstl::Writer::writeStlFile(path, providerAscii);
		REQUIRE(res == microstl::Result::Success);
		REQUIRE(std::filesystem::exists(path));
		std::filesystem::remove(path);
	}

	{
		TEST_SCOPE("Test writer with nulled out normals");
		microstl::MeshReaderHandler handler;
		auto res = microstl::Reader::readStlFile(findTestFile("box_meshlab_ascii.stl"), handler);
		REQUIRE(res == handler.result && res == microstl::Result::Success);

		// Default mode is binary
		microstl::MeshProvider providerBinary(handler.mesh);
		providerBinary.clearNormals = true;
		std::filesystem::path path("tmp.stl");
		REQUIRE(!std::filesystem::exists(path));
		res = microstl::Writer::writeStlFile(path, providerBinary);
		REQUIRE(res == microstl::Result::Success);
		REQUIRE(std::filesystem::exists(path));
		std::filesystem::remove(path);

		// Optional ASCII mode
		microstl::MeshProvider providerAscii(handler.mesh);
		providerAscii.clearNormals = true;
		providerAscii.ascii = true;
		REQUIRE(!std::filesystem::exists(path));
		res = microstl::Writer::writeStlFile(path, providerAscii);
		REQUIRE(res == microstl::Result::Success);
		REQUIRE(std::filesystem::exists(path));
		std::filesystem::remove(path);
	}

	{
		TEST_SCOPE("Test default provider implementation");
		struct MyProvider : microstl::Writer::Provider
		{
			size_t getFacetCount() override
			{ return 1; }
			void getFacet(size_t index, float v1[3], float v2[3], float v3[3], float n[3]) override
			{
				REQUIRE(index == 0);
				v1[0] = 0; v1[1] = 0; v1[2] = 0;
				v2[0] = 0; v2[1] = 0; v2[2] = 1;
				v3[0] = 0; v3[1] = 1; v3[2] = 1;
				n[0] = -1; n[1] = 0; n[2] = 0;
			}
		};
		MyProvider provider;
		std::filesystem::path path("tmp.stl");
		REQUIRE(!std::filesystem::exists(path));
		auto res = microstl::Writer::writeStlFile(path, provider);
		REQUIRE(res == microstl::Result::Success);
		REQUIRE(std::filesystem::exists(path));
		std::filesystem::remove(path);
	}

	{
		TEST_SCOPE("Test writer with UTF8 file path");
		microstl::MeshReaderHandler handler;
		auto res = microstl::Reader::readStlFile(findTestFile("box_meshlab_ascii.stl"), handler);
		REQUIRE(res == handler.result && res == microstl::Result::Success);

		microstl::MeshProvider provider(handler.mesh);
		auto utf8Path = std::filesystem::u8path(u8"简化字.stl");
		REQUIRE(!std::filesystem::exists(utf8Path));
		res = microstl::Writer::writeStlFile(utf8Path.u8string().c_str(), provider);
		REQUIRE(res == microstl::Result::Success);
		REQUIRE(std::filesystem::exists(utf8Path));
		std::filesystem::remove(utf8Path);
	}

	#ifdef _WIN32 // Wide strings are only common on Windows
	{
		TEST_SCOPE("Test writer with wide string file path");
		microstl::MeshReaderHandler handler;
		auto res = microstl::Reader::readStlFile(findTestFile("box_meshlab_ascii.stl"), handler);
		REQUIRE(res == handler.result && res == microstl::Result::Success);

		microstl::MeshProvider provider(handler.mesh);
		std::filesystem::path widePath(L"简化字.stl");
		REQUIRE(!std::filesystem::exists(widePath));
		res = microstl::Writer::writeStlFile(widePath.wstring().c_str(), provider);
		REQUIRE(res == microstl::Result::Success);
		REQUIRE(std::filesystem::exists(widePath));
		std::filesystem::remove(widePath);
	}
	#endif

	{
		TEST_SCOPE("Test writer with buffer interface");
		microstl::MeshReaderHandler handler;
		auto res = microstl::Reader::readStlFile(findTestFile("box_meshlab_ascii.stl"), handler);
		REQUIRE(res == handler.result && res == microstl::Result::Success);

		microstl::MeshProvider provider(handler.mesh);
		std::string buffer;
		res = microstl::Writer::writeStlBuffer(buffer, provider);
		REQUIRE(res == microstl::Result::Success);
		REQUIRE(buffer.size() == 80 + 4 + 12 * (12 * 4 + 2));
	}

	{
		TEST_SCOPE("Test writer with stream interface");
		microstl::MeshReaderHandler handler;
		auto res = microstl::Reader::readStlFile(findTestFile("box_meshlab_ascii.stl"), handler);
		REQUIRE(res == handler.result && res == microstl::Result::Success);

		microstl::MeshProvider provider(handler.mesh);
		std::ofstream ofs("tmp.stl", std::ios::binary);
		res = microstl::Writer::writeStlStream(ofs, provider);
		REQUIRE(res == microstl::Result::Success);
		REQUIRE(ofs.tellp() == 80 + 4 + 12 * (12 * 4 + 2));
		ofs.close();
		REQUIRE(std::filesystem::exists("tmp.stl"));
		std::filesystem::remove("tmp.stl");
	}

	{
		TEST_SCOPE("Test writer with invalid file path");
		microstl::MeshReaderHandler handler;
		auto res = microstl::Reader::readStlFile(findTestFile("box_meshlab_ascii.stl"), handler);
		REQUIRE(res == handler.result && res == microstl::Result::Success);

		microstl::MeshProvider provider(handler.mesh);
		std::filesystem::path path("folder/does/not/exist/out.stl");
		res = microstl::Writer::writeStlFile(path, provider);
		REQUIRE(res == microstl::Result::FileError);
		REQUIRE(!std::filesystem::exists(path));
	}

	{
		TEST_SCOPE("Full cycle test of reader, deduplicator and writer");
		microstl::MeshReaderHandler handler;
		auto res = microstl::Reader::readStlFile(findTestFile("box_meshlab_ascii.stl"), handler);
		REQUIRE(res == handler.result && res == microstl::Result::Success);
		auto orgMeshCopy = handler.mesh;

		auto fvMesh = microstl::deduplicateVertices(handler.mesh);
		REQUIRE(fvMesh.vertices.size() == 8);

		microstl::FVMeshProvider fvProvider(fvMesh);
		res = microstl::Writer::writeStlFile("binary.stl", fvProvider);
		REQUIRE(res == microstl::Result::Success);

		res = microstl::Reader::readStlFile("binary.stl", handler);
		REQUIRE(!handler.ascii);
		REQUIRE(res == handler.result && res == microstl::Result::Success);
		REQUIRE(handler.mesh.facets.size() == 12);

		microstl::MeshProvider provider(handler.mesh);
		provider.ascii = true;
		res = microstl::Writer::writeStlFile("ascii.stl", provider);
		REQUIRE(res == microstl::Result::Success);

		res = microstl::Reader::readStlFile("ascii.stl", handler);
		REQUIRE(res == handler.result && res == microstl::Result::Success);
		REQUIRE(handler.ascii);
		REQUIRE(handler.mesh.facets.size() == 12);

		for (size_t i = 0; i < 12; i++)
		{
			const auto& orgFacet = orgMeshCopy.facets[i];
			const auto& facet = handler.mesh.facets[i];
			REQUIRE(orgFacet.n.x == facet.n.x);
			REQUIRE(orgFacet.n.y == facet.n.y);
			REQUIRE(orgFacet.n.z == facet.n.z);
			REQUIRE(orgFacet.v1.x == facet.v1.x);
			REQUIRE(orgFacet.v1.y == facet.v1.y);
			REQUIRE(orgFacet.v1.z == facet.v1.z);
			REQUIRE(orgFacet.v2.x == facet.v2.x);
			REQUIRE(orgFacet.v2.y == facet.v2.y);
			REQUIRE(orgFacet.v2.z == facet.v2.z);
			REQUIRE(orgFacet.v3.x == facet.v3.x);
			REQUIRE(orgFacet.v3.y == facet.v3.y);
			REQUIRE(orgFacet.v3.z == facet.v3.z);
		}

		std::filesystem::remove("ascii.stl");
		std::filesystem::remove("binary.stl");
	}

	{
		TEST_SCOPE("Test string conversion for results");
		REQUIRE(microstl::getResultString(microstl::Result::Undefined) == "Undefined");
		REQUIRE(microstl::getResultString(microstl::Result::Success) == "Success");
		REQUIRE(microstl::getResultString(microstl::Result::FileError) == "FileError");
		REQUIRE(microstl::getResultString(microstl::Result::MissingDataError) == "MissingDataError");
		REQUIRE(microstl::getResultString(microstl::Result::UnexpectedError) == "UnexpectedError");
		REQUIRE(microstl::getResultString(microstl::Result::ParserError) == "ParserError");
		REQUIRE(microstl::getResultString(microstl::Result::LineLimitError) == "LineLimitError");
		REQUIRE(microstl::getResultString(microstl::Result::FacetCountError) == "FacetCountError");
		REQUIRE(microstl::getResultString(microstl::Result::EndianError) == "EndianError");
	}

	return 0;
}
