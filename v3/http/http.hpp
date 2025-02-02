#ifndef __LAMBDA_HTTP__
#define __LAMBDA_HTTP__

#include <vector>
#include <string>
#include <map>
#include <cstdint>
#include <optional>
#include <functional>
#include <ctime>

#include "../net/net.hpp"

namespace Lambda {

	enum struct Method : int {
		GET,
		POST,
		PUT,
		DEL,
		HEAD,
		OPTIONS,
		TRACE,
		PATCH,
		CONNECT
	};

	enum struct Status : int {
		Continue						= 100,
		SwitchingProtocols				= 101,
		Processing						= 102,
		EarlyHints						= 103,

		OK								= 200,
		Created							= 201,
		Accepted						= 202,
		NonAuthoritativeInfo			= 203,
		NoContent						= 204,
		ResetContent					= 205,
		PartialContent					= 206,
		MultiStatus						= 207,
		AlreadyReported					= 208,
		IMUsed							= 226,

		MultipleChoices					= 300,
		MovedPermanently				= 301,
		Found							= 302,
		SeeOther						= 303,
		NotModified						= 304,
		UseProxy						= 305,
		TemporaryRedirect				= 307,
		PermanentRedirect				= 308,

		BadRequest						= 400,
		Unauthorized					= 401,
		PaymentRequired					= 402,
		Forbidden						= 403,
		NotFound						= 404,
		MethodNotAllowed				= 405,
		NotAcceptable					= 406,
		ProxyAuthRequired				= 407,
		RequestTimeout					= 408,
		Conflict						= 409,
		Gone							= 410,
		LengthRequired					= 411,
		PreconditionFailed				= 412,
		RequestEntityTooLarge			= 413,
		RequestURITooLong				= 414,
		UnsupportedMediaType			= 415,
		RequestedRangeNotSatisfiable	= 416,
		ExpectationFailed				= 417,
		Teapot							= 418,
		MisdirectedRequest				= 421,
		UnprocessableEntity				= 422,
		Locked							= 423,
		FailedDependency				= 424,
		TooEarly						= 425,
		UpgradeRequired					= 426,
		PreconditionRequired			= 428,
		TooManyRequests					= 429,
		RequestHeaderFieldsTooLarge		= 431,
		UnavailableForLegalReasons		= 451,

		InternalServerError				= 500,
		NotImplemented					= 501,
		BadGateway						= 502,
		ServiceUnavailable				= 503,
		GatewayTimeout					= 504,
		HTTPVersionNotSupported			= 505,
		VariantAlsoNegotiates			= 506,
		InsufficientStorage				= 507,
		LoopDetected					= 508,
		NotExtended						= 510,
		NetworkAuthenticationRequired	= 511
	};

	namespace HTTP {

		typedef std::vector<std::string> MultiValue;
		typedef std::vector<std::pair<std::string, std::string>> Entries;

		class Values {
			protected:
				std::map<std::string, MultiValue> m_entries;
				std::string m_format_key(const std::string& key) const noexcept;

			public:
				Values() = default;
				Values(const Values& other);
				Values(Values&& other);

				Values& operator=(const Values& other) noexcept;
				Values& operator=(Values&& other) noexcept;

				bool has(const std::string& key) const noexcept;
				std::string get(const std::string& key) const noexcept;
				MultiValue get_all(const std::string& key) const noexcept;
				void set(const std::string& key, const std::string& value) noexcept;
				void append(const std::string& key, const std::string& value) noexcept;
				void del(const std::string& key) noexcept;
				Entries entries() const noexcept;
				size_t size() const noexcept;
		};

		typedef std::vector<uint8_t> Buffer;
		typedef std::function<size_t(Status status, const Values& header)> HeaderWriterCallback;
		typedef std::function<size_t(const Buffer& data)> WriterCallback;

		const size_t LengthUnknown = -1;
		typedef std::function<Buffer(size_t size)> ReaderCallback;
	};

	class URLSearchParams : public HTTP::Values {
		public:
			URLSearchParams() = default;
			URLSearchParams(const std::string& params);
			std::string to_string() const noexcept;
	};

	struct BasicAuth {
		std::string user;
		std::string password;
	};

	struct URL {
		URL() = default;
		URL(const std::string& url);

		std::string scheme;
		std::optional<BasicAuth> user;
		std::string host;
		std::string path;
		URLSearchParams search;
		std::string fragment;

		std::string to_string() const noexcept;
		std::string href() const;
	};

	std::string encode_uri_component(const std::string& input);
	std::string decode_uri_component(const std::string& input);

	class Date {
		private:
			time_t m_unix;
			tm m_tms;
		
		public:
			Date() noexcept;
			Date(time_t epoch) noexcept;

			std::string date() const noexcept;
			std::string to_utc_string() const noexcept;
			std::string to_log_string() const noexcept;

			int second() const noexcept {
				return this->m_tms.tm_sec;
			}

			int month() const noexcept {
				return this->m_tms.tm_mon;
			}

			int minute() const noexcept {
				return this->m_tms.tm_min;
			}

			int hour() const noexcept {
				return this->m_tms.tm_hour;
			}

			int year() const noexcept {
				return this->m_tms.tm_year;
			}

			int day() const noexcept {
				return this->m_tms.tm_mday;
			}

			time_t epoch() const noexcept {
				return this->m_unix;
			}
	};

	class RequestBody {
		private:
			HTTP::ReaderCallback m_reader;

		public:
			RequestBody() = default;
			RequestBody(HTTP::ReaderCallback reader) : m_reader(reader) {};

			HTTP::Buffer read(size_t chunk_size) {

				if (!this->m_reader) {
					return {};
				}

				return this->m_reader(chunk_size);
			}

			HTTP::Buffer read_all() {
				return this->read(HTTP::LengthUnknown);
			}

			std::string text() {
				auto buffer = this->read_all();
				return std::string(buffer.begin(), buffer.end());
			}

			//	todo: add form data
			//	todo: add json
			//	todo: check content types
	};

	typedef HTTP::Values Headers;
	typedef HTTP::Values CookieValues;

	enum struct SameSite {
		Default,
		Lax,
		Strict,
		None
	};

	struct Cookie {
		std::string name;
		std::string value;

		std::string domain;
		std::string path;
		std::optional<time_t> expires;
		std::optional<int64_t> max_age;

		bool secure = false;
		bool http_only = false;
		SameSite same_site = SameSite::Default;

		std::string to_string() const;
	};

	struct Request {
		Net::RemoteAddress remote_addr;
		Method method;
		URL url;
		Headers headers;
		RequestBody body;
		CookieValues cookies;
	};

	class ResponseWriter {
		private:
			Headers& m_headers;
			HTTP::HeaderWriterCallback m_header_writer;
			HTTP::WriterCallback m_writer;

		public:
			ResponseWriter(Headers& headers, HTTP::HeaderWriterCallback header_writer, HTTP::WriterCallback writer)
				: m_headers(headers), m_header_writer(header_writer), m_writer(writer) {};
			ResponseWriter(const ResponseWriter& other) = delete;

			Headers& header() noexcept {
				return this->m_headers;
			}

			size_t write_header() {
				return this->write_header(Status::OK);
			}

			size_t write_header(Status status) {
				return this->m_header_writer(status, this->m_headers);
			}

			size_t write(const HTTP::Buffer& data) {
				return this->m_writer(data);
			}

			size_t write(const std::string& text) {
				return this->m_writer(HTTP::Buffer(text.begin(), text.end()));
			}

			void set_cookie(const Cookie& cookie) {
				this->m_headers.append("Set-Cookie", cookie.to_string());
			}
	};

	typedef std::function<void(Request& req, ResponseWriter& wrt)> HandlerFn;
};

#endif
