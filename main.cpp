#include <iostream>
#include <string>
#include <regex>

#include <nlohmann/json.hpp>
using JSON = nlohmann::json;

#include "include/maddsua/lambda.hpp"

//	Pls note:
//
//	while lambda privides the backbone for your server app,
//	you still are responsible for routing and handling requests
//	
//	I'm thinking about adding some kind of simple rule based router,
//	so it will be trivial to use lambda as regular http server,	
//	but it's not the purpose lambda is built for


//	A user-defined structure, is used to pass any kind of data
//	to an isolated process
struct passtrough {
	maddsua::radishDB* db;
};

//	declare a request callback function
lambda::lambdaResponse requestHandeler(lambda::lambdaEvent event);


int main(int argc, char** argv) {

	//	create lambda server
	lambda::lambda lambdaserver;
	
	//	tweak server settings
	lambda::lambdaConfig lambdacfg;
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

	
	//	now let's create a database instance
	maddsua::radishDB database;

	//	set a test value to the database, so it can be easily accessed
	//	request url will look like this: "http://localhost:27015/api/db?entry=test"
	database.push("test", "test value", false);

	//	let's put a pointer to the database inside and object 
	//	you don't really need to use a struct/object for this, but I do it anyway
	//	and if you'd like to pass an object... you get it, just use a struct
	passtrough interstellarData;
		interstellarData.db = &database;

	//	and make it accessable to the lambda
	lambdaserver.openWormhole(&interstellarData);

	//	now just chill in the log loop
	while (true) {

		for (auto log : lambdaserver.showLogs()) std::cout << log << std::endl;

		//	just chill while server is running
		Sleep(1000);
	}

	return 0;
}


lambda::lambdaResponse requestHandeler(lambda::lambdaEvent event) {


	//	let' demonstrate a database access
	//	we can access database by it's pointer, passed trough the wormhole
	//	check if requested URL starts with something we consider the database API path
	if (lambda::startsWith(event.path, "/api/db")) {

		//	OK, so user asks to get database access

		//	let's check if we can access it now
		if (!event.wormhole) {
			return {
				200,
				{
					{ "content-type", lambda::mimetype("json") }
				},
				JSON({
					{"Request", "Failed"},
					{"Error", "Sorry, but the database is not accessable"}
				}).dump()
			};
		}

		//	let's see what he wants, get a record id
		auto entryID = lambda::findQuery("entry", &event.searchQuery);

		//	return if not specified
		if (!entryID.size()) {
			return {
				200,
				{
					{ "content-type", lambda::mimetype("json") }
				},
				JSON({
					{"Request", "Failed"},
					{"Error", "Entry id is not specified"}
				}).dump()
			};
		}

		auto db = ((passtrough*)event.wormhole)->db;

		//	try to get a record
		if (event.method == "GET") {

			auto recordData = db->pull(entryID);

			//	returen error if no data was returned
			if (!recordData.size()) {
				return {
					200,
					{
						{ "content-type", lambda::mimetype("json") }
					},
					JSON({
						{"Request", "Partially succeeded"},
						{"Error", "Requested entry is not found"}
					}).dump()
				};
			}

			//	return the data
			return {
				200,
				{
					{ "content-type", lambda::mimetype("txt") }
				},
				recordData
			};
		}

		//	OK, let's store some data
		if (event.method == "POST") {

			//	returen error if request body is empty
			if (!event.body.size()) {
				return {
					200,
					{
						{ "content-type", lambda::mimetype("json") }
					},
					JSON({
						{"Request", "Partially succeeded"},
						{"Error", "Empty request body; Use DELETE method to remove data"}
					}).dump()
				};
			}

			//	write to the database, overwrite if exists
			db->push(entryID, event.body, true);

			//	report success
			return {
				200,
				{
					{ "content-type", lambda::mimetype("json") }
				},
				JSON({
					{"Request", "Ok"},
					{"Info", "Saved"}
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
					{
						{ "content-type", lambda::mimetype("json") }
					},
					JSON({
						{"Request", "Partially succeeded"},
						{"Error", "No such entry or already deleted"}
					}).dump()
				};
			}

			//	report successfull deletion
			return {
				200,
				{
					{ "content-type", lambda::mimetype("json") }
				},
				JSON({
					{"Request", "Ok"},
					{"Info", "Deleted"}
				}).dump()
			};
		}
	}


	//	OK, we're done with the database
	//	let's serve some static files - html, css and whatever else

	//	Fhis feature isn't really a selling point. I mean, lambda can be a
	//	 file server, but it was designed to replace AWS Lambda
	//	 on a local machine. Kinda like a cloud without a cloud.
	//	So at the moment, there are not much tools to help with serving files

	//	Format path, is up to you.
	//	Probably will add some basic path transformation rules in the future
	if (event.path[event.path.size() - 1] == '/') event.path += "index.html";
	event.path = std::regex_replace(("demo/" + event.path), std::regex("/+"), "/");

	//	read file contents to this string
	//	return if fails
	std::string filecontents;
	if (!lambda::fs::readSync(event.path, &filecontents)) {
		return { 404, {}, "File not found"};
	}

	//	determine the file extension
	auto fileext = event.path.find_last_of('.');
	//	determine content type based on file extension
	auto contentType = lambda::mimetype((fileext + 1) < event.path.size() ? event.path.substr(fileext + 1) : "bin");

	//	serve that kitty picture
	return {
		200,
		{
			{ "Content-Type", contentType },
		},
		filecontents
	};

}