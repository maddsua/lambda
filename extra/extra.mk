
include extra/kvstorage/kvstorage.mk
include extra/staticserver/staticserver.mk
include extra/vfs/vfs.mk

LIB_EXTRA				=	extra/extra.a
LIB_EXTRA_DEPS			=	$(LIB_EXTRA_KVSTORAGE_DEPS) $(LIB_EXTRA_STATICSERVER_DEPS)

# extra object
lambda.extra: $(LIB_EXTRA)

$(LIB_EXTRA): $(LIB_EXTRA_DEPS)
	ar rvs $(LIB_EXTRA) $(LIB_EXTRA_DEPS)
