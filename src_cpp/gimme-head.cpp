/*
	OneLoneCoder - Single Header Maker - v1.00

	Uses special comments to construct a single header library out of a multi-file
	project. The following special comments are recognised:

	//! START section_name      : Starts a named section of code
	//! END section_name        : Ends a named section of code

	//! GRAB file section_name  : Inserts a named section of code at this location

	Note "//!" must be followed by a space!

	A 'template_file' should be used to construct an 'output_file'. Ordinarily
	the template file will not have neamed sections, and simply grabs and guards
	what it needs via GRAB commands.

	This utility does not do anything clever, so it is up to the library writer
	to consider how their project is structured, their sections named and controlled, 
	and finally what is grabbed and where.

	Usage: gimme-head template_filename output_filename

	Example (assume working directory is set)
	gimme-head pge3_template.h ../../olcPixelGameEngine3.h
	


	License (OLC-3)
	~~~~~~~~~~~~~~~

	Copyright 2018 - 2025 OneLoneCoder.com

	Redistribution and use in source and binary forms, with or without modification,
	are permitted provided that the following conditions are met:

	1. Redistributions or derivations of source code must retain the above copyright
	notice, this list of conditions and the following disclaimer.

	2. Redistributions or derivative works in binary form must reproduce the above
	copyright notice. This list of conditions and the following	disclaimer must be
	reproduced in the documentation and/or other materials provided with the distribution.

	3. Neither the name of the copyright holder nor the names of its contributors may
	be used to endorse or promote products derived from this software without specific
	prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS	"AS IS" AND ANY
	EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
	OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
	SHALL THE COPYRIGHT	HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
	INCIDENTAL,	SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
	TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
	BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
	CONTRACT, STRICT LIABILITY, OR TORT	(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
	ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
	SUCH DAMAGE.

*/

#include <algorithm>
#include <array>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <list>
#include <optional>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

// Database of all named code sections
std::unordered_map<std::string, std::unordered_map<std::string, std::vector<std::string>>> mapCodeSections;

// Reads a file into a vector of strings
static std::optional<std::vector<std::string>> ImportFile(const std::string& sFilename)
{
	std::ifstream ifs(sFilename);
	if (ifs.is_open())
	{
		std::vector<std::string> vecLines;
		std::string sLine;
		while (std::getline(ifs, sLine))
			vecLines.push_back(sLine);
		return vecLines;
	}

	return std::nullopt;
}

// Borrowed from CIY Programming Language #2
namespace lut
{
	static constexpr auto MakeLUT(const std::string_view sAcceptedCharacters)
	{
		std::array<bool, 256> lut{ 0 };
		for (const auto c : sAcceptedCharacters)
			lut.at(uint8_t(c)) = true;
		return lut;
	};

	constexpr auto WhitespaceDigits =
		MakeLUT(" \t\n\r\v\f");
}

// Returns a vector of tokens if special comment "//!"
static std::vector<std::string> ParseLine(const std::string& sLine)
{
	std::vector<std::string> vecTokens;
	std::string token;
	bool bFirstToken = true;
	for (const auto c : sLine)
	{
		if (lut::WhitespaceDigits.at(c))
		{
			if (!token.empty())
			{
				if (!bFirstToken) // Ignore "//!"
				{					
					vecTokens.push_back(token);					
				}
				else
				{
					if (token != "//!")
						return {};

					bFirstToken = false;
				}

				token.clear();
			}
		}
		else
			token += c;
	}

	if(!vecTokens.empty() && !token.empty())
		vecTokens.push_back(token);
	
	return vecTokens;
}

// Recursively unravels code sections from template
void Reconstruct(std::vector<std::string>& out, const std::string& file, const std::string& section)
{
	const auto& code_section = mapCodeSections[file][section];
	bool bIgnoreLine = false;
	for (const auto& line : code_section)
	{
		const auto tokens = ParseLine(line);
		if (!tokens.empty())
		{
			if (tokens[0] == "START")
			{
				bIgnoreLine = true;
			}
			else if (tokens[0] == "END")
			{
				bIgnoreLine = false;
			}
			else if (tokens[0] == "GRAB" && !bIgnoreLine)
			{
				Reconstruct(out, tokens[1], tokens[2]);
			}
		}
		else
		{
			if (!bIgnoreLine)
				out.push_back(line);
		}
	}
}


int main(int argc, char* argv[])
{
	// Tidy up arguments
	std::vector<std::string> vecArgs;
	for (int i = 0; i < argc; i++)
		vecArgs.push_back(argv[i]);

	if (vecArgs.size() < 2)
	{
		std::cout << "Error: Not enough arguments\n";
		std::cout << "Usage: gimme-head template_filename output_filename\n";

		return -1;
	}

	// Argument 1 is the template file, so start there and discover
	// source files via "GRAB" comments
	auto f = ImportFile(vecArgs[1]);
	if (!f)
	{
		std::cout << "Error: Could not open file - " << vecArgs[1] << "\n";
		return -1;
	}

	// Template file is considered a code section by itself, called "#OG_TEMPLATE"	
	std::unordered_map<std::string, std::vector<std::string>> primary_section;
	primary_section.insert({ "OG_TEMPLATE", *f });
	mapCodeSections.insert({ vecArgs[1], primary_section });
	
	// As we scan through the source, we'll discover all files accessed by "GRAB"
	// command. 
	std::list<std::string> listDiscoveredFiles;

	// Thanks slavka, pointing out std::unordered_set is no good for this
	auto list_append_no_duplicates = [&listDiscoveredFiles](const std::string& sFile)
		{
			if (std::find(listDiscoveredFiles.begin(), listDiscoveredFiles.end(), sFile) == listDiscoveredFiles.end())
				listDiscoveredFiles.push_back(sFile);
		};
	

	// Stage 1) Discovery phase, note listDiscoveredFiles will be mutated during loop
	list_append_no_duplicates(vecArgs[1]);
	for (const auto& file : listDiscoveredFiles)
	{
		auto f = ImportFile(file);
		if (!f)
		{
			std::cout << "Error: Could not open file:\n\t" << file << "\n";
			return -1;
		}

		std::string sCurrentSectionName = "";
		std::string sTargetFile = file;

		for (const auto& line : *f)
		{
			const auto tokens = ParseLine(line);
			

			if (!tokens.empty())
			{
				if (tokens[0] == "GRAB")
				{
					// If file is legit, add it to discovered files
					if(std::filesystem::exists(tokens[1]))
						list_append_no_duplicates(tokens[1]);
					// otherwise its a "virtual file"
				}

				else if (tokens[0] == "START")
				{
					// Arg1 = Section Name
					//
					// Start of a named section of code, appended to file path
					// If GLOBAL is specified, section is accumulated with no
					// duplicated lines
					sCurrentSectionName = tokens[1];

					if (tokens.size() == 3)
					{
						sTargetFile = tokens[2];
					}

					std::unordered_map<std::string, std::vector<std::string>> section;
					section.insert({ sCurrentSectionName, {} });
					mapCodeSections.insert({ sTargetFile, section });
					
				}

				else if (tokens[0] == "END")
				{
					// Arg1 = Section Name
					//
					// Ends a named section
					std::cout << "Section found: " << sTargetFile << "#" << sCurrentSectionName << "\n";
					sTargetFile = file;
					sCurrentSectionName.clear();
				}

				else if (!sCurrentSectionName.empty())
				{
					// Append code to current section
					if(sTargetFile == file)
						mapCodeSections[sTargetFile][sCurrentSectionName].push_back(line);
					else
					{
						// Targeting a global virtual file - these dont accept duplicates
						if (std::find(
							mapCodeSections[sTargetFile][sCurrentSectionName].begin(), 
							mapCodeSections[sTargetFile][sCurrentSectionName].end(), line) == mapCodeSections[sTargetFile][sCurrentSectionName].end())
						{
							mapCodeSections[sTargetFile][sCurrentSectionName].push_back(line);
						}
					}
				}
			}
			else if (!sCurrentSectionName.empty())
			{
				// Append code to current section
				if (sTargetFile == file)
					mapCodeSections[sTargetFile][sCurrentSectionName].push_back(line);
				else
				{
					// Targeting a global virtual file - these dont accept duplicates
					if (std::find(
						mapCodeSections[sTargetFile][sCurrentSectionName].begin(),
						mapCodeSections[sTargetFile][sCurrentSectionName].end(), line) == mapCodeSections[sTargetFile][sCurrentSectionName].end())
					{
						mapCodeSections[sTargetFile][sCurrentSectionName].push_back(line);
					}
				}
			}			
		}
	}


	// Pass 2) Reconstruction
	std::vector<std::string> vecOutputFile;
	Reconstruct(vecOutputFile, vecArgs[1], "OG_TEMPLATE");


	// Stage 3) Write out
	std::ofstream ofs(vecArgs[2]);
	for (const auto& l : vecOutputFile)
		ofs << l << "\n";
	ofs.close();
	
	return 0;
}




