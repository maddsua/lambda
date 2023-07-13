#include "./server.hpp"

using namespace Lambda;

void Server::addLogRecord(std::string message, LogLevel level) {

	LogEntry newRecord;
		newRecord.message = message;
		newRecord.timestamp = time(nullptr);
		newRecord.datetime = HTTP::serverDate(newRecord.timestamp);
		newRecord.loglevel = level;

	std::lock_guard<std::mutex>lock(mtLock);
	this->logQueue.push_back(std::move(newRecord));
}

std::vector<LogEntry> Server::logs() {

	if (!hasNewLogs()) return {};

	std::lock_guard<std::mutex>lock(mtLock);
	auto queueCopy = std::vector<LogEntry>(this->logQueue.begin(), this->logQueue.end());
	this->logQueue.clear();

	return queueCopy;
}

std::vector<std::string> Server::logsText() {

	if (!hasNewLogs()) return {};

	std::vector<std::string> logEntries;
	std::string loglevelstring;
	std::lock_guard<std::mutex>lock(mtLock);
	for (auto& item : this->logQueue) {

		switch (item.loglevel) {
			case LAMBDA_LOG_ERROR: {
				loglevelstring = "ERROR";
			} break;

			case LAMBDA_LOG_WARN: {
				loglevelstring = "WARN";
			} break;

			case LAMBDA_LOG_INFO: {
				loglevelstring = "INFO";
			} break;

			case LAMBDA_LOG_CRITICAL: {
				loglevelstring = "CRITICAL";
			} break;
			
			default:{
				loglevelstring.clear();
			} break;
		}

		logEntries.push_back(item.datetime + (loglevelstring.size() ? (" [" + loglevelstring + "] ") : " ") + item.message);
	}
	this->logQueue.clear();

	return logEntries;
}
