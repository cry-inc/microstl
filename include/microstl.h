#include <cstdint>
#include <string>
#include <fstream>
#include <sstream>
#include <filesystem>

namespace microstl
{
	// Interface that must be implemented to receive the data from the parsed STL file.
	class IHandler
	{
	public:
		virtual ~IHandler() {}
		
		// Called when the parser is using ASCII mode before any other methods.
		virtual void onAscii() {};

		// Called when the parser is using binary mode before any other methods.
		virtual void onBinary() {};

		// Always called when parsing a binary STL. Before onFacet() is called for the first time.
		virtual void onTriangleCount(uint32_t triangles) {};

		// May be called when parsing an ASCII STL file with a valid name. Will be always called before onFacet().
		virtual void onName(const std::string& name) {};

		// Might be called in ASCII mode when an error is detected to signal the line number of the problem.
		// Do not rely on this method to be called when an error occurs, its fully optional!
		virtual void onError(size_t lineNumber) {};

		// Will be called for each triangle (a.k.a facet/face) in the STL file
		virtual void onFacet(const float v1[3], const float v2[3], const float v3[3], const float n[3]) = 0;
	};

	enum class Result : uint16_t {
		Success = 0, // Everything went smooth, the STL file was parsed without issues
		FileError = 1, // Unable to read the specified STL file
		MissingDataError = 2, // STL data ended unexpectely and is incomplete or otherwise broken
		UnexpectedError = 3, // Found an unexpected keyword or token in an ASCII STL file
		ParserError = 4, // Unable to parse vertex coordinates or normal vector in an ASCII STL file
	};

	bool isAsciiFormat(std::istream& is)
	{
		char header[5] = { 0, };
		is.read(header, sizeof(header));
		is.seekg(0, std::ios::beg);
		return memcmp("solid", header, sizeof(header)) == 0;
	}

	bool readNextLine(std::istream& is, std::string& output)
	{
		output.resize(0);
		if (!is.good() || is.eof())
			return false;

		while (!is.eof())
		{
			char byte;
			is.read(&byte, 1);
			if (byte == '\n')
				return true;
			else
				output.push_back(byte);
		}

		return true;
	}

	std::string stringTrim(const std::string& line)
	{
		std::string result;
		size_t index = 0;
		while (index < line.size() && (line[index] == '\t' || line[index] == ' ' || line[index] == '\r' || line[index] == '\n'))
		{
			index++;
		}

		if (index == line.size())
			return result;

		while (index < line.size())
		{
			result.push_back(line[index]);
			index++;
		}

		index = result.size() - 1;
		while (index >= 0 && (result[index] == '\t' || result[index] == ' ' || result[index] == '\r' || result[index] == '\n'))
		{
			index--;
		}

		result.resize(index + 1);
		return result;
	}

	bool stringStartsWith(const std::string& str, const char* prefix)
	{
		return memcmp(prefix, str.data(), strlen(prefix)) == 0;
	}

	bool stringParseThreeValues(const std::string& str, float& v1, float& v2, float& v3)
	{
		std::stringstream ss(str);
		ss >> v1;
		if (!ss)
			return false;

		ss >> v2;
		if (!ss)
			return false;

		ss >> v3;
		if (!ss)
			return false;

		return true;
	}

	Result parseAsciiStream(std::istream& is, IHandler& handler)
	{
		handler.onAscii();

		// State machine variables
		bool activeSolid = false;
		bool activeFacet = false;
		bool activeLoop = false;
		size_t lineNumber = 0, solidCount = 0, facetCount = 0, loopCount = 0, vertexCount = 0;
		float n[3] = {0,};
		float v[9] = {0,};

		// Line parse with loop to work the state machine
		while (true)
		{
			std::string line;
			if (!readNextLine(is, line))
				break;
			lineNumber++;
			line = stringTrim(line);
			if (stringStartsWith(line, "solid"))
			{
				if (activeSolid || solidCount != 0)
				{
					handler.onError(lineNumber);
					return Result::UnexpectedError;
				}
				activeSolid = true;
				if (line.length() > 5)
				{
					std::string name = stringTrim(line.substr(5));
					handler.onName(name);
				}
			}
			if (stringStartsWith(line, "endsolid"))
			{
				if (!activeSolid || activeFacet || activeLoop)
				{
					handler.onError(lineNumber);
					return Result::UnexpectedError;
				}
				activeSolid = false;
				solidCount++;
			}
			if (stringStartsWith(line, "facet normal"))
			{
				if (!activeSolid || activeLoop || activeFacet)
				{
					handler.onError(lineNumber);
					return Result::UnexpectedError;
				}
				activeFacet = true;
				line = stringTrim(line.substr(12));
				if (!stringParseThreeValues(line, n[0], n[1], n[2]))
				{
					handler.onError(lineNumber);
					return Result::ParserError;
				}
			}
			if (stringStartsWith(line, "endfacet"))
			{
				if (!activeSolid || activeLoop || !activeFacet || loopCount != 1)
				{
					handler.onError(lineNumber);
					return Result::UnexpectedError;
				}
				activeFacet = false;
				facetCount++;
				loopCount = 0;
				handler.onFacet(v + 0, v + 3, v + 6, n);
			}
			if (stringStartsWith(line, "outer loop"))
			{
				if (!activeSolid || !activeFacet || activeLoop)
				{
					handler.onError(lineNumber);
					return Result::UnexpectedError;
				}
				activeLoop = true;
			}
			if (stringStartsWith(line, "endloop"))
			{
				if (!activeSolid || !activeFacet || !activeLoop || vertexCount != 3)
				{
					handler.onError(lineNumber);
					return Result::UnexpectedError;
				}
				activeLoop = false;
				loopCount++;
				vertexCount = 0;
			}
			if (stringStartsWith(line, "vertex"))
			{
				if (!activeSolid || !activeFacet || !activeLoop || vertexCount >= 3)
				{
					handler.onError(lineNumber);
					return Result::UnexpectedError;
				}
				line = stringTrim(line.substr(6));
				if (!stringParseThreeValues(line, v[vertexCount * 3 + 0], v[vertexCount * 3 + 1], v[vertexCount * 3 + 2]))
				{
					handler.onError(lineNumber);
					return Result::ParserError;
				}
				vertexCount++;
			}
		}

		if (activeSolid || activeFacet || activeLoop || solidCount == 0)
			return Result::MissingDataError;

		return Result::Success;
	}

	Result parseBinaryStream(std::istream& is, IHandler& handler)
	{
		handler.onBinary();

		char buffer[80];
		is.read(buffer, sizeof(buffer));
		if (!is)
			return Result::MissingDataError;
		is.read(buffer, 4);
		if (!is)
			return Result::MissingDataError;
		uint32_t triangles = reinterpret_cast<uint32_t*>(buffer)[0];
		handler.onTriangleCount(triangles);
		for (size_t t = 0; t < triangles; t++)
		{
			is.read(buffer, 50);
			if (!is)
				return Result::MissingDataError;
			float n[3];
			float v[9];
			memcpy(n, buffer + 0, 12);
			memcpy(v, buffer + 12, 3 * 12);
			handler.onFacet(v + 0, v + 3, v + 6, n);
		}

		return Result::Success;
	}

	Result parseStlStream(std::istream& is, IHandler& handler)
	{
		if (isAsciiFormat(is))
			return parseAsciiStream(is, handler);
		else
			return parseBinaryStream(is, handler);
	}

	Result parseStlFile(std::filesystem::path& filePath, IHandler& handler)
	{
		std::ifstream ifs(filePath, std::ios::binary);
		if (!ifs)
			return Result::FileError;

		return parseStlStream(ifs, handler);
	};
};
