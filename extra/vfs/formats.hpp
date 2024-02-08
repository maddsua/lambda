#ifndef __LIB_MADDSUA_LAMBDA_EXTRA_VFS_FORMATS__
#define __LIB_MADDSUA_LAMBDA_EXTRA_VFS_FORMATS__

#include "./vfs.hpp"

#include <queue>
#include <future>

namespace Lambda::VFS::Formats {

	class SyncQueue {
		private:
			std::queue<VirtualFile> m_queue;
			std::mutex m_lock;
			bool m_done = false;
			std::future<void>* watchForExit = nullptr;

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
			void setWatcher(std::future<void>* watch) {
				this->watchForExit = watch;
			}
	};

	namespace Tar {

		extern const std::initializer_list<std::string> supportedExtensions;

		void exportArchive(const std::string& path, SyncQueue& queue);
		void importArchive(const std::string& path, SyncQueue& queue);
	};
	
};

#endif
