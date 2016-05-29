#pragma once
namespace fb {	
	typedef void CURL;

	enum fcurl_type_e {
		CFTYPE_NONE = 0,
		CFTYPE_FILE = 1,
		CFTYPE_CURL = 2
	};

	struct fcurl_data
	{
		enum fcurl_type_e type;     /* type of handle */
		union {
			CURL *curl;
			FILE *file;
		} handle;                   /* handle */

		char *buffer;               /* buffer to store cached data*/
		size_t buffer_len;          /* currently allocated buffers length */
		size_t buffer_pos;          /* end of data in buffer*/
		int still_running;          /* Is background url fetch still in progress */
		char contentType[255];
	};

	typedef struct fcurl_data URL_FILE;

	void initializeCURL();
	void uninitializeCURL();
	URL_FILE *url_fopen(const char *url, const char *operation);
	void url_fclose(URL_FILE *file);
	size_t header_callback(char *buffer, size_t size,
		size_t nitems, void *userdata);
	size_t write_callback(char *buffer, size_t size, size_t nitems, void *userp);
	int fill_buffer(URL_FILE *file, size_t want);
	int use_buffer(URL_FILE *file, size_t want);
	int url_feof(URL_FILE *file);
	size_t url_fread(void *ptr, size_t size, size_t nmemb, URL_FILE *file);
	char *url_fgets(char *ptr, size_t size, URL_FILE *file);
	void url_rewind(URL_FILE *file);
}
