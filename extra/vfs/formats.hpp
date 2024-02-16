#ifndef __LIB_MADDSUA_LAMBDA_EXTRA_VFS_FORMATS__
#define __LIB_MADDSUA_LAMBDA_EXTRA_VFS_FORMATS__

#include "./vfs.hpp"
#include "../../buildopts.hpp"

#include <vector>
#include <array>
#include <future>
#include <shared_mutex>

namespace Lambda::VFS::Formats {

	class SyncQueue {
		private:
			std::vector<VirtualFile> m_queue;
			std::mutex m_queue_mtx;
			std::shared_mutex m_future_mtx;
			bool m_done = false;
			std::future<void>* m_watch_future = nullptr;

		public:
			SyncQueue& operator=(const SyncQueue& other) = delete;
			SyncQueue& operator=(SyncQueue&& other) noexcept;

			bool await();
			bool awaitEmpty();
			VirtualFile next();
			void push(VirtualFile&& item);
			void close() noexcept;
			bool done() const noexcept;
			bool empty() const noexcept;
			void setPromiseExitWatcher(std::future<void>* watch) noexcept;
	};

	namespace Tar {

		#ifdef LAMBDA_BUILDOPTS_ENABLE_COMPRESSION
			static const std::array<std::string, 3> Extensions {
				".tar", ".tar.gz", ".tgz"
			};
		#else
			static const std::array<std::string, 1> Extensions {
				".tar"
			};
		#endif

		void exportArchive(const std::string& path, SyncQueue& queue);
		void importArchive(const std::string& path, SyncQueue& queue);
	};
	
};

#endif
