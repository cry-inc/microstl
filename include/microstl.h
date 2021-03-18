#include <cstdint>
#include <string>
#include <fstream>
#include <sstream>
#include <filesystem>

namespace microstl
{
	class IReceiver
	{
	public:
		virtual ~IReceiver() {}
		virtual void receiveName(const std::string& name) = 0;
		virtual void receiveFace(const float v1[3], const float v2[3], const float v3[3], const float n[3]) = 0;
	};

	enum class Result : uint16_t {
		Okay = 0,
		NotImplementedError = 2,
		FileReadError = 3,
		UnexpectedFileEnd = 4,
		InvalidFile = 5,
	};

	bool isAsciiFormat(std::ifstream& ifs)
	{
		char header[5] = { 0, };
		ifs.read(header, sizeof(header));
		ifs.seekg(0, std::ios::beg);
		return memcmp("solid", header, sizeof(header)) == 0;
	}

	bool readNextLine(std::ifstream& ifs, std::string& output)
	{
		output.resize(0);
		if (!ifs.is_open() || !ifs.good() || ifs.eof())
			return false;

		while (!ifs.eof())
		{
			char byte;
			ifs.read(&byte, 1);
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

	Result parseAsciiStream(std::ifstream& ifs, IReceiver& receiver)
	{
		// State machine variables
		bool activeSolid = false;
		bool activeFacet = false;
		bool activeLoop = false;
		size_t solidCount = 0, facetCount = 0, loopCount = 0, vertexCount = 0;
		float n[3] = {0,};
		float v[9] = {0,};

		// Line parse with loop to work the state machine
		while (true)
		{
			std::string line;
			if (!readNextLine(ifs, line))
				break;
			line = stringTrim(line);
			if (stringStartsWith(line, "solid"))
			{
				if (activeSolid || solidCount != 0)
					return Result::InvalidFile;
				activeSolid = true;
				if (line.length() > 5)
				{
					std::string name = stringTrim(line.substr(5));
					receiver.receiveName(name);
				}
			}
			if (stringStartsWith(line, "endsolid"))
			{
				if (!activeSolid || activeFacet || activeLoop)
					return Result::InvalidFile;
				activeSolid = false;
				solidCount++;
			}
			if (stringStartsWith(line, "facet normal"))
			{
				if (!activeSolid || activeLoop || activeFacet)
					return Result::InvalidFile;
				activeFacet = true;
				line = stringTrim(line.substr(12));
				if (!stringParseThreeValues(line, n[0], n[1], n[2]))
					return Result::InvalidFile;
			}
			if (stringStartsWith(line, "endfacet"))
			{
				if (!activeSolid || activeLoop || !activeFacet || loopCount != 1)
					return Result::InvalidFile;
				activeFacet = false;
				facetCount++;
				loopCount = 0;
				receiver.receiveFace(v + 0, v + 3, v + 6, n);
			}
			if (stringStartsWith(line, "outer loop"))
			{
				if (!activeSolid || !activeFacet || activeLoop)
					return Result::InvalidFile;
				activeLoop = true;
			}
			if (stringStartsWith(line, "endloop"))
			{
				if (!activeSolid || !activeFacet || !activeLoop || vertexCount != 3)
					return Result::InvalidFile;
				activeLoop = false;
				loopCount++;
				vertexCount = 0;
			}
			if (stringStartsWith(line, "vertex"))
			{
				if (!activeSolid || !activeFacet || !activeLoop || vertexCount >= 3)
					return Result::InvalidFile;
				line = stringTrim(line.substr(6));
				if (!stringParseThreeValues(line, v[vertexCount * 3 + 0], v[vertexCount * 3 + 1], v[vertexCount * 3 + 2]))
					return Result::InvalidFile;
				vertexCount++;
			}
		}

		if (activeSolid || activeFacet || activeLoop || solidCount != 1)
			return Result::InvalidFile;

		return Result::Okay;
	}

	Result parseBinaryStream(std::ifstream& ifs, IReceiver& receiver)
	{
		char buffer[80];
		ifs.read(buffer, sizeof(buffer));
		if (!ifs)
			return Result::UnexpectedFileEnd;
		ifs.read(buffer, 4);
		if (!ifs)
			return Result::UnexpectedFileEnd;
		uint32_t triangles = reinterpret_cast<uint32_t*>(buffer)[0];
		for (size_t t = 0; t < triangles; t++)
		{
			ifs.read(buffer, 50);
			if (!ifs)
				return Result::UnexpectedFileEnd;
			float n[3];
			float v[9];
			memcpy(n, buffer + 0, 12);
			memcpy(v + 0, buffer + 12, 12);
			memcpy(v + 3, buffer + 24, 12);
			memcpy(v + 6, buffer + 36, 12);
			receiver.receiveFace(v + 0, v + 3, v + 6, n);
		}

		return Result::Okay;
	}

	Result parseStlStream(std::ifstream& ifs, IReceiver& receiver)
	{
		if (isAsciiFormat(ifs))
			return parseAsciiStream(ifs, receiver);
		else
			return parseBinaryStream(ifs, receiver);
	}

	Result parseStlFile(std::filesystem::path& filePath, IReceiver& receiver)
	{
		std::ifstream ifs(filePath, std::ios::binary);
		if (!ifs.is_open())
			return Result::FileReadError;

		return parseStlStream(ifs, receiver);
	};
};
