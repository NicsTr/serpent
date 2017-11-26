OBJ_SERPENT=serpent.o serpent_algo.o

DEPS+=r_util
DEPFLAGS=-L../../util -lr_util -L.. -lr_crypto

STATIC_OBJ+=${OBJ_SERPENT}
TARGET_SERPENT=serpent.${EXT_SO}

ALL_TARGETS+=${TARGET_SERPENT}

${TARGET_SERPENT}: ${OBJ_SERPENT}
	${CC} $(call libname,serpent) ${LDFLAGS} ${CFLAGS} \
		-o ${TARGET_SERPENT} ${OBJ_SERPENT} $(DEPFLAGS)
