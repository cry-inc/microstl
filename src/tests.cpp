#include <microstl.h>

#define REQUIRE(x) {if (!(x)) throw std::exception(); }

int main()
{
	{
		microstl::MeshParserHandler handler;
		auto res = microstl::Parser::parseStlFile("../../testdata/simple_ascii.stl", handler);
		REQUIRE(res == microstl::Parser::Result::Success);
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
		microstl::MeshParserHandler handler;
		auto res = microstl::Parser::parseStlFile("../../testdata/half_donut_ascii.stl", handler);
		REQUIRE(res == microstl::Parser::Result::Success);
		REQUIRE(handler.name == "Half Donut");
		REQUIRE(handler.ascii);
		REQUIRE(handler.errorLineNumber == 0);
		REQUIRE(handler.header.empty());
		REQUIRE(handler.mesh.facets.size() == 288);
	}

	{
		microstl::MeshParserHandler handler;
		auto res = microstl::Parser::parseStlFile("../../testdata/stencil_binary.stl", handler);
		REQUIRE(res == microstl::Parser::Result::Success);
		REQUIRE(handler.name.empty());
		REQUIRE(!handler.ascii);
		REQUIRE(handler.errorLineNumber == 0);
		REQUIRE(handler.header.size() == 80);
		for (size_t i = 0; i < 80; i++)
			REQUIRE(handler.header[i] == 0);
		REQUIRE(handler.mesh.facets.size() == 2330);
	}

	{
		microstl::MeshParserHandler handler;
		auto res = microstl::Parser::parseStlFile("../../testdata/empty_file.stl", handler);
		REQUIRE(res == microstl::Parser::Result::MissingDataError);
		REQUIRE(handler.name.empty());
		REQUIRE(!handler.ascii);
		REQUIRE(handler.errorLineNumber == 0);
		REQUIRE(handler.header.empty());
		REQUIRE(handler.mesh.facets.empty());
	}

	return 0;
}
