include config.mk

.PHONY: all 
all:
	@for dir in ${BUILD_DIR};\
	do \
		make -C $${dir};\
	done

.PHONY: clean
clean:
	rm -rf ${BUILD_ROOT}/bin/dep 
	rm -rf ${BUILD_ROOT}/bin/obj
	rm -rf ${BUILD_ROOT}/server
