
include extra/kvstorage/kvstorage.mk

LIB_EXTRA				=	extra/extra.a
LIB_EXTRA_DEPS			=	$(LIB_EXTRA_KVSTORAGE_DEPS)

# extra object
lambda.extra: $(LIB_EXTRA)

$(LIB_EXTRA): $(LIB_EXTRA_DEPS)
	ar rvs $(LIB_EXTRA) $(LIB_EXTRA_DEPS)
