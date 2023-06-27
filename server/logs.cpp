#include "./server.hpp"

using namespace Lambda;

void Server::addLogRecord(std::string message) {

	LogEntry newRecord;
		newRecord.message = message;
		newRecord.timestamp = time(nullptr);
		newRecord.datetime = HTTP::serverDate(newRecord.timestamp);

	std::lock_guard<std::mutex>lock(mtLock);
	this->logQueue.push_back(std::move(newRecord));
}

bool Server::hasNewLogs() {
	return this->logQueue.size() > 0;
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
	std::lock_guard<std::mutex>lock(mtLock);
	for (auto& item : this->logQueue) {
		logEntries.push_back("[" + item.datetime + "] " + item.message);
	}
	this->logQueue.clear();

	return logEntries;
}
