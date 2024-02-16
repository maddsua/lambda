
#ifndef __LIB_MADDSUA_LAMBDA_BUILD_OPTIONS__
#define __LIB_MADDSUA_LAMBDA_BUILD_OPTIONS__

	/**
	 * You can comment this line if you don't any compression libs to be used.
	 * This disables both zlib and brotli.
	 * 
	 * Don't forget to add "compression=disabled" make argument to disable linking with them.
	*/
	#define LAMBDA_BUILDOPTS_ENABLE_COMPRESSION

#endif
