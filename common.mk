# common makefile used by all the subdirectories
# each sub makefile needs to specify its own BIN variable
CC = g++ -Wall -std=c++11 -pthread

SOURCES = ${wildcard *.cpp}

OBJ = ${SOURCES:.cpp=.o}

# ${shell echo ${OBJ}}

OBJ_DIR = ${BIN_DIR}/obj

${shell mkdir -p ${BIN_DIR}}

${shell mkdir -p ${OBJ_DIR}}

BIN := ${addprefix ${BIN_DIR}/,${BIN}}

OBJ := ${addprefix ${OBJ_DIR}/, ${OBJ}}

LINK_OBJ = ${wildcard ${OBJ_DIR}/*.o}
LINK_OBJ += ${OBJ}

all: ${LINK_OBJ} ${BIN}

${BIN}:${LINK_OBJ}
	${CC} -o $@ $^

${OBJ_DIR}/%.o:%.cpp
	${CC} -I ${INCLUDE_DIR} -o $@ -c $^