#include <microstl.h>

struct DummyHandler : microstl::Parser::Handler
{
	void onFacet(const float v1[3], const float v2[3], const float v3[3], const float n[3]) override {}
};

int main()
{
	DummyHandler dummyHandler;

	{
		auto res = microstl::Parser::parseStlFile("../../testdata/simple_ascii.stl", dummyHandler);
		if (res != microstl::Parser::Result::Success)
			return 1;
	}

	{
		auto res = microstl::Parser::parseStlFile("../../testdata/half_donut_ascii.stl", dummyHandler);
		if (res != microstl::Parser::Result::Success)
			return 2;
	}

	{
		auto res = microstl::Parser::parseStlFile("../../testdata/stencil_binary.stl", dummyHandler);
		if (res != microstl::Parser::Result::Success)
			return 3;
	}

	{
		auto res = microstl::Parser::parseStlFile("../../testdata/empty_file.stl", dummyHandler);
		if (res != microstl::Parser::Result::MissingDataError)
			return 4;
	}

	return 0;
}
