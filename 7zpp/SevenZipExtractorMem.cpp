#include "stdafx.h"

#include "MemExtractCallback.h"
#include "PropVariant.h"
#include "OutMemStream.h"
#include <comdef.h>

#include <7zip/cpp/7zip/Archive/IArchive.h>
#include <7zip/cpp/7zip/IPassword.h>
#include "ProgressCallback.h"

#include "SevenZipExtractorMem.h"
#include "GUIDs.h"
#include "FileSys.h"
#include "ArchiveOpenCallback.h"
#include "ArchiveExtractCallback.h"
#include "MemExtractCallback.h"
#include "InStreamWrapper.h"
#include "UsefulFunctions.h"

namespace SevenZip {

	namespace intl {

		class MemExtractCallback2 : public MemExtractCallback {
		private:

			inline static std::vector<BYTE> s_dummy;
			std::deque<std::vector<BYTE>>& m_buffers;
		public:

			MemExtractCallback2(const CComPtr< IInArchive >& archiveHandler, std::deque<std::vector<BYTE>>& buffers, const TString& archivePath, const TString& password, ProgressCallback* callback) :
				MemExtractCallback(archiveHandler, s_dummy, archivePath, password, callback),
				m_buffers(buffers) {
			}

			virtual ~MemExtractCallback2() = default;

			//// IProgress
			//STDMETHOD(SetTotal)( UInt64 size );
			//STDMETHOD(SetCompleted)( const UInt64* completeValue );

			//// Early exit, this is not part of any interface
			//STDMETHOD(CheckBreak)();

			// IMemExtractCallback
			STDMETHOD(GetStream)(UInt32 index, ISequentialOutStream** outStream, Int32 askExtractMode) {
				try
				{
					// Retrieve all the various properties for the file at this index.
					GetPropertyFilePath(index);
					if (askExtractMode != NArchive::NExtract::NAskMode::kExtract)
					{
						return S_OK;
					}

					GetPropertyIsDir(index);
					GetPropertySize(index);
				}
				catch (_com_error& ex)
				{
					return ex.Error();
				}

				if (!m_isDir)
				{
					CComPtr<ISequentialOutStream> outStreamLoc(new COutMemStream(m_buffers[index]));
					m_outMemStream = outStreamLoc;
					*outStream = outStreamLoc.Detach();
				}


				return CheckBreak();
			}
			//STDMETHOD(PrepareOperation)( Int32 askExtractMode );
			//STDMETHOD(SetOperationResult)( Int32 resultEOperationResult );

			//// ICryptoGetTextPassword
			//STDMETHOD(CryptoGetTextPassword)( BSTR* password );
		};
	}

	using namespace intl;

	bool SevenZipExtractorMem::ExtractFileToMemoryMulti(std::vector<uint32_t> const& indices, std::deque<std::vector<BYTE>>& out_buffer, ProgressCallback* callback /*= nullptr*/)
	{
		CComPtr< IStream > archiveStream = FileSys::OpenFileToRead(m_archivePath);
		if (archiveStream == nullptr)
		{
			return false;	//Could not open archive
		}

		CComPtr< IInArchive > archive = UsefulFunctions::GetArchiveReader(m_library, m_compressionFormat);
		CComPtr< InStreamWrapper > inFile = new InStreamWrapper(archiveStream);
		CComPtr< ArchiveOpenCallback > openCallback = new ArchiveOpenCallback(m_password);

		HRESULT hr = archive->Open(inFile, 0, openCallback);
		if (hr != S_OK)
		{
			return false;	//Open archive error
		}

		out_buffer.resize((size_t)indices.size());
		CComPtr< MemExtractCallback2 > extractCallback = new MemExtractCallback2(archive, out_buffer, m_archivePath, m_password, callback);

		hr = archive->Extract(indices.data(), indices.size(), false, extractCallback);
		if (hr != S_OK)
		{
			// returning S_FALSE also indicates error
			return false;	//Extract archive error
		}

		if (callback)
		{
			callback->OnDone(m_archivePath);
		}

		archive->Close();

		return true;

	}

}
