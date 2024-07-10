#!/bin/bash

GREEN='\033[0;32m'
BLUE='\033[0;34m'
ORANGE='\033[0;33m'
NC='\033[0m' # No Color


cd raiz

echo -e "${GREEN} CREACION DE ARCHIVOS Y CARPETAS ${NC}"

echo -e "${BLUE} se crean 2 archivos y 2 carpeta${NC}"

touch a

touch b

mkdir carpeta

mkdir carpeta2

echo -e "${BLUE} dentro de ${ORANGE}raiz${BLUE} hay: ${NC}"

ls

echo " ----------------- "

echo -e "${GREEN} BORRADO DE ARCHIVOS Y CARPETAS ${NC}"

echo -e "${BLUE} se borran un archivo y una carpeta${NC}"

rm b

rmdir carpeta2

echo -e "${BLUE} dentro de ${ORANGE}raiz${BLUE} hay: ${NC}"

ls

echo " ----------------- "

echo -e "${GREEN} WRITE Y READ ${NC}"

echo -e "${BLUE} se crea un archivo 'c' y se guarda su stat en otro archivo 'stat_c'${NC}"

touch c

stat c > stat_c

echo -e "${BLUE} se hace cat de ${ORANGE}stat_c${BLUE}: ${NC}"

cat stat_c

echo -e "${BLUE} se apendea texto al archivo ${ORANGE}stat_c${BLUE} y ahora contiene:${NC}"

echo "holaaaaaaaaaaaaaaa soy" >> stat_c

echo "texto apendeado" >> stat_c

cat stat_c

echo -e "${BLUE} se pisa el archivo ${ORANGE}stat_c${BLUE} y ahora contiene:${NC}"

echo "texto pisado" > stat_c

cat stat_c

echo " ----------------- "

echo -e "${GREEN} VARIOS NIVELES DE RECUSIÓN  ${NC}"

echo -e "${BLUE} se crea una subcarpeta y se crea un archivo dentro de ella${NC}"

mkdir subcarpeta

cd subcarpeta

touch d

echo -e "${BLUE} estoy parado en: ${NC}"

pwd

echo -e "${BLUE} dentro de ${ORANGE}subcarpeta${BLUE} hay: ${NC}"

ls

echo -e "${BLUE} se crea una sub-subcarpeta y se crea un archivo dentro de ella${NC}"

mkdir sub-subcarpeta

cd sub-subcarpeta

touch e

echo -e "${BLUE} estoy parado en: ${NC}"

pwd

echo -e "${BLUE} dentro de ${ORANGE}sub-subcarpeta${BLUE} hay: ${NC}"

ls

echo " ----------------- "

cd ../../..

make clean

umount raiz
