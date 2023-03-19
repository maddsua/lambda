#include <iostream>
#include <string>
#include <regex>

#include <nlohmann/json.hpp>
using JSON = nlohmann::json;

#include "include/lambda/lambda.hpp"
#include "include/lambda/localdb.hpp"
#include "include/lambda/fs.hpp"
#include "include/lambda/util.hpp"


//	Pls note:
//
//	while lambda provides the backbone for your server app,
//	you still are responsible for routing and handling requests
//	
//	I'm thinking about adding some kind of simple rule based router,
//	so it will be trivial to use lambda as regular http server,	
//	but it's not the purpose lambda is built for


//	A user-defined structure, is used to pass any kind of data
//	to an isolated process
struct passtrough {
	lambda::localdb* db;
	lambda::virtualFS* vfs;
};

//	declare a request callback function
lambda::Response requestHandeler(lambda::Event event);


int main(int argc, char** argv) {

	//	create lambda server
	lambda::lambda lambdaserver;
	
	//	tweak server settings
	lambda::Config lambdacfg;
		lambdacfg.compression_preferBr = true;

	//	apply server settings
	lambdaserver.setConfig(lambdacfg);

	//	actually start the server
	auto startresult = lambdaserver.start(27015, &requestHandeler);

	//	check if successful
	std::cout << "Lambda instance: " << startresult.cause << std::endl;

	//	and exit if not
	if (!startresult.success) return 1;

	//	report successful lambda start
	std::cout << "Waiting for connections at http://localhost:27015/" << std::endl;


	//	let's create a "wormhole" to share objects with isolated function
	passtrough shared;

	//	creating a database instance
	shared.db = new lambda::localdb;

	//	set a test value to the database, so it can be easily accessed
	//	request url will look like this: "http://localhost:27015/api/db?entry=test"
	shared.db->set("test", "test value", false);

	//	Oh and I wanna show virtual FS too
	//	Sometimes VFS can make resources load muuuch faster
	shared.vfs = new lambda::virtualFS;
	auto loaded = shared.vfs->loadSnapshot("demo/dist.tar.gz");

	std::cout << "VFS: " << (loaded == lambda::virtualFS::st_ok ? "Loaded" : ("Failed with code " + std::to_string(loaded))) << std::endl;


	//	and finally make wormhole accessable to the lambda
	lambdaserver.openWormhole(&shared);

	//	now just chill in the log loop
	while (true) {

		for (auto log : lambdaserver.showLogs()) std::cout << log << std::endl;

		//	just chill while server is running
		Sleep(1000);
	}

	return 0;
}


lambda::Response requestHandeler(lambda::Event event) {


	//	let' demonstrate a database access
	//	we can access database by it's pointer, passed trough the wormhole
	//	check if requested URL starts with something we consider the database API path
	if (event.path.startsWith("/api/db")) {

		//	OK, so user asks to get database access

		//	let's check if we can access it now
		if (!event.wormhole) {
			return {
				200,
				{/* no headers here, using default mimetype - json */},
				JSON({
					{"success", false},
					{"error", "sorry, but the database is not accessable"}
				}).dump()
			};
		}

		//	let's see what he wants, get a record id
		auto entryID = event.searchQuery.find("entry");

		//	return if not specified
		if (!entryID.size()) {
			return {
				200,
				{},
				JSON({
					{"success", false},
					{"error", "entry id is not specified"}
				}).dump()
			};
		}

		auto db = ((passtrough*)event.wormhole)->db;

		//	try to get a record
		if (event.method == "GET") {

			auto recordData = db->get(entryID);

			//	returen error if no data was returned
			if (!recordData.size()) {
				return {
					200,
					{},
					JSON({
						{"success", false},
						{"error", "requested entry is not found"}
					}).dump()
				};
			}

			//	return the data
			try {

				//	try as a plain text first
				return {
					200,
					{},
					JSON({
						{"success", true},
						{"data", recordData}
					}).dump()
				};

			} catch(...) {

				//	if fails, encode base64 and return that
				return {
					200,
					{},
					JSON({
						{"success", true},
						{"base64", true},
						{"data", b64Encode(&recordData)}
					}).dump()
				};
			}
		}

		//	OK, let's store some data
		if (event.method == "POST") {

			//	returen error if request body is empty
			if (!event.body.size()) {
				return {
					200,
					{},
					JSON({
						{"success", false},
						{"error", "empty request body; Use DELETE method to remove data"}
					}).dump()
				};
			}

			//	write to the database, overwrite if exists
			db->set(entryID, event.body, true);

			//	report success
			return {
				200,
				{},
				JSON({
					{"success", true},
					{"info", "saved"}
				}).dump()
			};
		}

		//	handle deletion
		if (event.method == "DELETE") {

			//	check if we can delete entry
			//	this will fail only if entry is not present in database
			if (!db->remove(entryID)) {
				return {
					200,
					{},
					JSON({
						{"success", false},
						{"error", "no such entry or already deleted"}
					}).dump()
				};
			}

			//	report successfull deletion
			return {
				200,
				{},
				JSON({
					{"success", true},
					{"info", "deleted"}
				}).dump()
			};
		}
	}

	//	enough of the database
	//	let's try fetching something from remote
	if (event.path.startsWith("/api/proxy")) {

		auto proxyTo = event.searchQuery.find("target");
		if (!proxyTo.size()) {
			return {
				200,
				{},
				JSON({
					{"success", false},
					{"errors", "No url specified. Try like this: /api/proxy?target=http://google.com"}
				}).dump()
			};
		}

		//	GET http://google.com with no additional headers of request body
		auto googeResp = lambda::fetch(proxyTo);

		if (googeResp.errors.size()) {
			return {
				200,
				{},
				JSON({
					{"proxy to", proxyTo},
					{"success", false},
					{"errors", googeResp.errors}
				}).dump()
			};
		}

		auto listOfHeaders = JSON::array();

		auto allRespHeaders = googeResp.headers.list();

		for (auto header : allRespHeaders) {
			listOfHeaders.push_back({
				{"name", header.key},
				{"value", header.value}
			});
		};

		return {
			200,
			{},
			JSON({
				{"proxy to", proxyTo},
				{"success", true},
				{"response", googeResp.statusText},
				{"headers", listOfHeaders},
				{"body", std::to_string(googeResp.body.size()) + " bytes"}
			}).dump()
		};
	}

	//	For the last example, let's serve some static html, css and whatever else

	//	Fhis feature isn't really a selling point. I mean, lambda can be a
	//	 file server, but it was designed to replace AWS Lambda
	//	 on a local machine. Kinda like a cloud without a cloud.
	//	So at the moment, there are not much tools to help with serving files

	//	Format path, is up to you.
	//	Probably will add some basic path transformation rules in the future
	if (event.path.endsWith("/")) event.path += "index.html";
	

	std::string filecontents;
	std::string storageHeader;

	//	try to get files from vfs, then try form a directory
	auto vfs = ((passtrough*)event.wormhole)->vfs;

	if (vfs) {
		filecontents = vfs->read(event.path.sstring);
		storageHeader = "lambda VFS";
	}
	
	if (!filecontents.size()) {

		event.path = std::regex_replace(("demo/dist/" + event.path.sstring), std::regex("/+"), "/");
		storageHeader = "filesystem";
		
		if (!lambda::fs::readSync(event.path.sstring, &filecontents)) {
			return { 404, {}, "File not found"};
		}
	}

	//	determine the file extension
	auto fileext = event.path.sstring.find_last_of('.');
	//	determine content type based on file extension
	auto contentType = lambda::mimetype((fileext + 1) < event.path.sstring.size() ? event.path.sstring.substr(fileext + 1) : "bin");

	//	serve that kitty picture
	return {
		200, {
			{ "Content-Type", contentType },
			{ "X-Storage", storageHeader }
		}, filecontents
	};

}