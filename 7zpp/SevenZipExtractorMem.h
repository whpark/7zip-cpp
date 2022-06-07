// PWH. 2022.04.24.

#pragma once

#include <deque>
#include <vector>

#include "SevenZipExtractor.h"

namespace SevenZip
{
	class SevenZipExtractorMem : public SevenZipExtractor
	{
	public:

		SevenZipExtractorMem( const SevenZipLibrary& library, const TString& archivePath ) : SevenZipExtractor(library, archivePath) {}
		virtual ~SevenZipExtractorMem() {}

		bool ExtractFileToMemoryMulti(std::vector<uint32_t> const& indices, std::deque<std::vector<BYTE>>& out_buffer, ProgressCallback* callback = nullptr);
	};
}
