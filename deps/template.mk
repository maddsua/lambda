
${warningmsg}

CC		= gcc
FLAGS	= -std=c11

.PHONY: all all-before all-after action-custom

deps: libbrotli-static.a libz-static.a

clean: action-custom
	del /S *.o *.exe *.a *.dll *.res

${sources}
