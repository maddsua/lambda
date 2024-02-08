#ifndef __LIB_MADDSUA_LAMBDA_EXTRA_VFS_FORMATS__
#define __LIB_MADDSUA_LAMBDA_EXTRA_VFS_FORMATS__

#include "./vfs.hpp"

#include <queue>
#include <future>

namespace Lambda::VFS::Formats {

	class FSQueue {
		private:
			std::queue<VirtualFile> m_queue;
			std::mutex m_lock;
			bool m_done = false;
			std::future<void>* watchForExit = nullptr;

		public:
			FSQueue& operator=(const FSQueue& other) = delete;
			FSQueue& operator=(FSQueue&& other) noexcept;

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

		void exportArchive(const std::string& path, FSQueue& queue);
		void importArchive(const std::string& path, FSQueue& queue);
	};
	
};

#endif
