#ifndef __LIB_MADDSUA_LAMBDA_HTTP__
#define __LIB_MADDSUA_LAMBDA_HTTP__

#include <vector>
#include <string>
#include <map>
#include <cstdint>
#include <optional>
#include <functional>
#include <ctime>

#include "../net/net.hpp"

namespace Lambda {

	namespace HTTP {

		typedef std::vector<std::string> MultiValue;
		typedef std::vector<std::pair<std::string, std::string>> Entries;

		class Values {
			protected:
				std::map<std::string, MultiValue> m_entries;

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
				bool empty() const noexcept;
		};

		typedef std::vector<uint8_t> Buffer;

		const size_t LengthUnknown = -1;
	};

	enum struct Method : int {
		GET,
		POST,
		PUT,
		PATCH,
		DEL,
		HEAD,
		OPTIONS,
		TRACE,
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

	class BodyReader {
		public:
			BodyReader() = default;
			BodyReader(const BodyReader& other) = delete;
			virtual ~BodyReader() = default;

			virtual bool is_readable() const noexcept = 0;

			virtual HTTP::Buffer read(size_t chunk_size) = 0;
			virtual HTTP::Buffer read_all() = 0;
			virtual std::string text() = 0;

		//	todo: add form data
		//	todo: add json
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

	//	todo: add serve context to request
	struct Request {
		Net::RemoteAddress remote_addr;
		Method method;
		URL& url;
		Headers& headers;
		CookieValues cookies;
		BodyReader& body;
	};

	//	todo: add json and html writers (simply complete response objects)

	class ResponseWriter {
		public:
			ResponseWriter() = default;
			ResponseWriter(const ResponseWriter& other) = delete;
			virtual ~ResponseWriter() = default;

			virtual bool writable() const noexcept = 0;
			virtual Headers& header() noexcept = 0;
			virtual size_t write_header() = 0;
			virtual size_t write_header(Status status) = 0;
			virtual size_t write(const HTTP::Buffer& data) = 0;
			virtual size_t write(const std::string& text) = 0;
			virtual void set_cookie(const Cookie& cookie) = 0;
	};

	typedef std::function<void(Request& req, ResponseWriter& wrt)> HandlerFn;

	class Handler {
		public:
			virtual void handler_fn(Request& req, ResponseWriter& wrt) = 0;
			virtual ~Handler() = default;
	};

	struct SSEevent {
		std::optional<std::string> event;
		std::string data;
		std::optional<std::string> id;
		std::optional<uint32_t> retry;
	};

	class SSEWriter {
		private:
			ResponseWriter& m_writer;
			bool m_ok = true;
		public:
			SSEWriter(ResponseWriter& writer);
			~SSEWriter();

			size_t write(const SSEevent& msg);
			bool is_writable() const noexcept;
			size_t close();
	};
};

#endif
