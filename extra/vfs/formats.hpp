#ifndef __LIB_MADDSUA_LAMBDA_EXTRA_VFS_FORMATS__
#define __LIB_MADDSUA_LAMBDA_EXTRA_VFS_FORMATS__

#include "./vfs.hpp"

#include <vector>
#include <future>

namespace Lambda::VFS::Formats {

	class SyncQueue {
		private:
			std::vector<VirtualFile> m_queue;
			std::mutex m_queue_lock;
			std::mutex m_future_lock;
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

		extern const std::initializer_list<std::string> supportedExtensions;

		void exportArchive(const std::string& path, SyncQueue& queue);
		void importArchive(const std::string& path, SyncQueue& queue);
	};
	
};

#endif
